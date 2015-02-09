#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <glib.h>

#include "sig.h"
#include "frame_geometry.h"
#include "capture_frame.h"
#include "capture_proc.h"


/* Capture process identification */
extern char proc_name[];

#define eprintf(args...) { fprintf(stderr, "%s: ", proc_name); fprintf(stderr, args); }
#define dprintf(args...) { fprintf(stderr, "[DEBUG AGENT %s] ", proc_name); fprintf(stderr, args); }


/* Capture device context and hooks */
extern capture_interface_t _capture_interface;
static capture_t *capture = NULL;

/* Capture process i/o handles */
static char *devname = NULL;
static capture_opt_t opt = 0;
static int callback_fd = -1;
static int update_fd = -1;

static GMainLoop *loop = NULL;
static GIOChannel *stdin_channel = NULL;
static guint stdin_tag = -1;


static void callback_open(capture_t *capture)
{
  if ( callback_fd < 0 ) {
    printf("[CALLBACK] open %d %d %s\n", capture->shmid, capture->cap, capture->name);
  }
  else {
    char buf[sizeof(capture_proc_cb_open_t) + strlen(capture->name)];
    capture_proc_cb_open_t *cb = (void *) buf;
    int ret;

    cb->hdr.type = CAPTURE_PROC_CB_OPEN;
    cb->hdr.len = sizeof(buf);
    cb->shmid = capture->shmid;
    cb->cap = capture->cap;
    strcpy(cb->name, capture->name);

    ret = write(callback_fd, buf, sizeof(buf));
    if ( ret < 0 ) {
      eprintf("write(callback-open): %s\n", strerror(errno));
    }
  }
}

static void callback_window(frame_geometry_t *g)
{
  if ( callback_fd < 0 ) {
    printf("[CALLBACK] window %s\n", frame_geometry_str(g));
  }
  else {
    char buf[sizeof(capture_proc_cb_window_t)];
    capture_proc_cb_window_t *cb = (void *) buf;
    int ret;

    cb->hdr.type = CAPTURE_PROC_CB_WINDOW;
    cb->hdr.len = sizeof(buf);
    cb->g = *g;

    ret = write(callback_fd, buf, sizeof(buf));
    if ( ret < 0 ) {
      eprintf("write(callback-window): %s\n", strerror(errno));
    }
  }
}

static void callback_period(long delay)
{
  if ( callback_fd < 0 ) {
    printf("[CALLBACK] period %ld\n", delay);
  }
  else {
    char buf[sizeof(capture_proc_cb_period_t)];
    capture_proc_cb_period_t *cb = (void *) buf;
    int ret;

    cb->hdr.type = CAPTURE_PROC_CB_PERIOD;
    cb->hdr.len = sizeof(buf);
    cb->delay = delay;

    ret = write(callback_fd, buf, sizeof(buf));
    if ( ret < 0 ) {
      eprintf("write(callback-period): %s\n", strerror(errno));
    }
  }
}

static void callback_attr(capture_attr_t *attrs, int nmemb)
{
  int i;

  if ( callback_fd < 0 ) {
    printf("[CALLBACK] attr ");
    for (i = 0; i < nmemb; i++) {
      if ( i > 0 )
	printf(",");
      printf("%s=%s", attrs[i].key, attrs[i].value);
    }
    printf("\n");
  }
  else {
    int len = sizeof(capture_proc_cb_attr_t);
    char *buf = malloc(len);
    capture_proc_cb_attr_t *cb = (capture_proc_cb_attr_t *) buf;
    int offs = 0;
    int ret;

    for (i = 0; i < nmemb; i++) {
      char *key = attrs[i].key;
      char *value = attrs[i].value;
      int kvlen = strlen(key) + 1;

      if ( value != NULL )
	kvlen += strlen(value) + 1;
      len += kvlen;
      buf = realloc(buf, len);
      cb = (capture_proc_cb_attr_t *) buf;
      snprintf(cb->key_value + offs, kvlen, "%s=%s", key, (value != NULL) ? value:"");
      offs += kvlen;
    }

    cb->hdr.type = CAPTURE_PROC_CB_ATTR;
    cb->hdr.len = len;
    cb->key_value[offs] = '\0';

    if ( opt & CAPTURE_OPT_DEBUG ) {
      dprintf("CALLBACK ATTR len=%d nmemb=%d\n", len, nmemb);
    }

    ret = write(callback_fd, buf, len);
    if ( ret < 0 ) {
      eprintf("write(callback-period): %s\n", strerror(errno));
    }
    else if ( ret != len ) {
      eprintf("write(callback-period): message truncated\n");
    }

    free(buf);
  }
}


static char *parse_command_arg(char **str)
{
  char *s1 = *str;
  char *s2;

  /* Skip leading blanks */
  while ( (*s1 != '\0') && (*s1 <= ' ') )
    s1++;

  /* Cut after end of word */
  s2 = s1;
  while ( *s2 > ' ' )
    s2++;
  if ( *s2 != '\0' )
    *(s2++) = '\0';
  while ( (*s2 != '\0') && (*s2 <= ' ') )
    s2++;

  *str = s2;
  return s1;
}


static void parse_command_action(char *cmd, char *str)
{
#define MAX_ARGC 4
  char *argv[MAX_ARGC];
  int argc = 0;

  for (argc = 0; (*str != '\0') && (argc < MAX_ARGC); argc++) {
    argv[argc] = parse_command_arg(&str);
  }

  while ( argc < MAX_ARGC ) {
    argv[argc++] = NULL;
  }

  if ( argc < 1 )
    return;

  if ( strcmp(cmd, "action_key") == 0 ) {
    if ( argc > 1 ) {
      if ( _capture_interface.action_key ) {
	int down = strtol(argv[0], NULL, 0);
	unsigned long key = strtoul(argv[1], NULL, 0);
	_capture_interface.action_key(capture, down, key);
      }
    }
    else {
      eprintf("Too few arguments for command '%s'", cmd);
    }
  }
  else if ( strcmp(cmd, "action_pointer") == 0 ) {
    if ( argc > 2 ) {
      if ( _capture_interface.action_pointer ) {
	unsigned char buttons = strtoul(argv[0], NULL, 0);
	unsigned int x = strtoul(argv[1], NULL, 0);
	unsigned int y = strtoul(argv[2], NULL, 0);
	_capture_interface.action_pointer(capture, buttons, x, y);
      }
    }
    else {
      eprintf("Too few arguments for command '%s'", cmd);
    }
  }
  else if ( strcmp(cmd, "action_scroll") == 0 ) {
    if ( argc > 0 ) {
      if ( _capture_interface.action_scroll ) {
	unsigned char direction = strtoul(argv[0], NULL, 0);
	_capture_interface.action_scroll(capture, direction);
      }
    }
    else {
      eprintf("Too few arguments for command '%s'", cmd);
    }
  }
}


static void parse_command(char *str)
{
  char *p;
  char *cmd;

  p = str + strlen(str);
  while ( (p > str) && (*(--p) < ' ') )
    *p = '\0';

  cmd = parse_command_arg(&str);
  if ( *cmd == '\0' )
    return;

  if ( strcmp(cmd, "window") == 0 ) {
    frame_geometry_t g = FRAME_GEOMETRY_NULL;

    if ( *str != '\0' ) {
      if ( _capture_interface.set_window ) {
	frame_geometry_parse(str, &g);
	_capture_interface.set_window(capture, &g);
      }
    }
    else {
      if ( _capture_interface.get_window ) {
	_capture_interface.get_window(capture, &g);
	callback_window(&g);
      }
    }
  }
  else if ( strcmp(cmd, "period") == 0 ) {   // period
    long delay = 0;

    if ( *str != '\0' ) {
      if ( _capture_interface.set_period )
	delay = _capture_interface.set_period(capture, strtol(str, NULL, 0));
    }
    else {
      if ( _capture_interface.set_period )
	delay = _capture_interface.get_period(capture);
    }

    callback_period(delay);
  }
  else if ( strcmp(cmd, "refresh") == 0 ) {
    if ( _capture_interface.refresh )
      _capture_interface.refresh(capture);
  }
  else if ( strncmp(cmd, "action_", 7) == 0 ) {
    parse_command_action(cmd, str);
  }
  else if ( strcmp(cmd, "attr") == 0 ) {
    char *key = parse_command_arg(&str);
    capture_attr_t *attrs = NULL;
    int nmemb = 0;

    if ( *key != '\0' ) {
      char *value = parse_command_arg(&str);

      if ( *value != '\0' ) {
	if ( _capture_interface.attr_set )
	  _capture_interface.attr_set(capture, key, value);
      }
      else {
	if ( _capture_interface.attr_get )
	  attrs = _capture_interface.attr_get(capture, key, &nmemb);
      }
    }
    else {
      if ( _capture_interface.attr_get )
	attrs = _capture_interface.attr_get(capture, NULL, &nmemb);
    }

    if ( attrs != NULL ) {
      callback_attr(attrs, nmemb);
    }
  }
  else if ( strcmp(cmd, "show_status") == 0 ) {
    if ( _capture_interface.show_status ) {
      _capture_interface.show_status(capture, str);
    }
  }
  else if ( strcmp(cmd, "exit") == 0 ) {
    exit(0);
  }
  else {
    eprintf("Unknown command '%s'\n", cmd);
  }
}


static void update(frame_geometry_t *g, void *data)
{
  if ( update_fd < 0 ) {
    printf("[UPDATE] %s\n", frame_geometry_str(g));
  }
  else {
    write(update_fd, g, sizeof(*g));
  }
}


static gboolean stdin_read(GIOChannel *source, GIOCondition condition, void *data)
{
  GIOStatus status;
  GError *error = NULL;
  gchar *buf = NULL;
  gsize size = 0;

  if ( condition & G_IO_HUP ) {
    exit(EXIT_FAILURE);
    return FALSE;
  }

  status = g_io_channel_read_line(stdin_channel, &buf, &size, NULL, &error);
  if ( status == G_IO_STATUS_NORMAL ) {
    if ( buf != NULL ) {
      parse_command(buf);
      g_free(buf);
    }
  }
  else if ( status != G_IO_STATUS_AGAIN ) {
    if ( status == G_IO_STATUS_ERROR ) {
      eprintf("read(stdin): %s\n", error->message);
      return FALSE;
    }
  }

  return TRUE;
}


static int parse_options(int argc, char **argv)
{
  int i;

  /* Parse command arguments */
  for (i = 1; i < argc; i++) {
    char *s = argv[i];

    if ( *s == '-' ) {
      s++;
      
      if ( strcmp(s, "callback") == 0 ) {
	char *sv = argv[++i];

	if ( sv == NULL )
	  return -1;
	if ( callback_fd >= 0 )
	  return -1;
	callback_fd = atoi(sv);
      }
      else if ( strcmp(s, "update") == 0 ) {
	char *sv = argv[++i];

	if ( sv == NULL )
	  return -1;
	if ( update_fd >= 0 )
	  return -1;
	update_fd = atoi(sv);
      }
      else if ( strcmp(s, "shared") == 0 ) {
	opt |= CAPTURE_OPT_SHARED;
      }
      else if ( strcmp(s, "debug") == 0 ) {
	opt |= CAPTURE_OPT_DEBUG;
      }
      else if ( strcmp(s, "rotate-cw") == 0 ) {
	opt |= CAPTURE_OPT_ROTATE;
      }
      else if ( strcmp(s, "rotate-ccw") == 0 ) {
	opt |= CAPTURE_OPT_ROTATE;
	opt |= CAPTURE_OPT_ROTATE_CCW;
      }
    }
    else {
      if ( devname != NULL )
	return -1;
      devname = s;
    }
  }

  return 0;
}


static void usage(void)
{
    fprintf(stderr, "Usage: %s <devname> [-callback <fd>] [-update <fd>] [-shared] [-debug] [-rotate-cw] [-rotate-ccw]\n", proc_name);
    exit(2);
}


static void terminate(void)
{
  if ( capture != NULL )
    _capture_interface.close(capture);
  capture = NULL;

  if ( update_fd > 0 ) {
    close(update_fd);
    update_fd = -1;
  }

  if ( callback_fd > 0 ) {
    close(callback_fd);
    callback_fd = -1;
  }

  if ( loop != NULL )
    g_main_loop_quit(loop);
  loop = NULL;
}


int main(int argc, char *argv[])
{
  /* Get command arguments */
  if ( parse_options(argc, argv) )
    usage();
  if ( devname == NULL )
    usage();

  if ( opt & CAPTURE_OPT_DEBUG ) {
    dprintf("START: device=\"%s\" callback(w)=%d update(w)=%d\n", devname, callback_fd, update_fd);
  }

  /* Enable multithreading */
  if ( !g_thread_supported() )
    g_thread_init(NULL);

  /* Open capture device */
  capture = _capture_interface.open(devname, opt, update, NULL);
  if ( capture == NULL ) {
    eprintf("Failed to open capture device\n");
    exit(1);
  }

  callback_open(capture);

  /* Hook termination procedure at exit stack and signal handling */
  atexit(terminate);
  sig_init(terminate);
  signal(SIGPIPE, SIG_IGN);

  /* Hook stdin read handler */
  fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);
  stdin_channel = g_io_channel_unix_new(STDIN_FILENO);
  stdin_tag = g_io_add_watch(stdin_channel, G_IO_IN | G_IO_HUP,
			     (GIOFunc) stdin_read, NULL);

  /* Create main loop */
  loop = g_main_loop_new(NULL, FALSE);

  /* Enter processing loop */
  g_main_loop_run(loop);

  /* Destroy main loop */
  g_main_loop_unref(loop);
  loop = NULL;

  terminate();

  return 0;
}
