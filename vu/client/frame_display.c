/****************************************************************************/
/* TestFarm VNC Interface                                                   */
/* Display interface                                                        */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 05-JAN-2004                                                    */
/****************************************************************************/

/*
 * $Revision: 1183 $
 * $Date: 2010-07-28 22:09:53 +0200 (mer., 28 juil. 2010) $
 */

#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <errno.h>
#include <limits.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "useful.h"
#include "shell.h"
#include "log.h"

#include "error.h"
#include "pattern.h"
#include "capture.h"
#include "frame_ctl.h"
#include "frame_display.h"


/*******************************************************/
/* Commands received from the Display Tool             */
/*******************************************************/

static void frame_display_cmd_reap_frame(frame_t *frame, frame_display_t *display)
{
  frame_display_frame_add(display, frame);
}

static void frame_display_cmd_read_history(shell_history_item_t *hsti, void *data)
{
	frame_display_t *display = data;
	if (display->dsock > 0)
		frame_ctl_history(display->dsock, hsti->id, hsti->str);
}

static void frame_display_cmd_reap(frame_display_t *display, frame_ctl_cmd *cmd)
{
  /* Send refresh period */
  frame_display_period(display, capture_get_period(display->root->capture));

  /* Send all existing frames */
  frame_foreach_child(display->root, (GFunc) frame_display_cmd_reap_frame, display, 0);

  /* Send all existing patterns */
  if ( display->pattern_get )
    display->pattern_get(display->pattern_data, cmd);

  /* Send command history list */
  if (display->dsock > 0) {
	  frame_ctl_history(display->dsock, 0, NULL);
	  if (display->shell != NULL)
		  shell_history_foreach(&(display->shell->history), frame_display_cmd_read_history, display);
  }
}


static int frame_display_cmd_source_done(shell_t *shell, frame_display_t *display)
{
  shell_set_source_done(shell, NULL, NULL);
  frame_display_source_done(display);
  return 0;
}


static void frame_display_cmd_process(frame_display_t *display, frame_ctl_cmd *cmd)
{
  int notfound = 0;

  /* Nothing to do if no shell interpreter is present */
  if ( display->shell == NULL )
    return;

  /* Set error reporting tag */
  error_default_tag(NULL);

  switch ( cmd->hdr.ctl ) {
  case FRAME_CTL_NONE:
    /* Display tool disconnected : cancel source termination handler */
    shell_set_source_done(display->shell, NULL, NULL);
    break;

  case FRAME_CTL_PATTERN:
    if ( display->pattern_set )
      display->pattern_set(display->pattern_data, cmd);
    break;

  case FRAME_CTL_REAP:
    frame_display_cmd_reap(display, cmd);
    break;

  case FRAME_CTL_KEY:
    capture_action_key(display->root->capture, cmd->key.down, cmd->key.key);
    break;

  case FRAME_CTL_POINTER:
    capture_action_pointer(display->root->capture, cmd->pointer.buttons, cmd->pointer.x, cmd->pointer.y);
    break;

  case FRAME_CTL_SCROLL:
    capture_action_scroll(display->root->capture, cmd->scroll.direction);
    break;

  case FRAME_CTL_COMMAND:
    if ( display->shell != NULL ) {
      shell_exec_line(display->shell, cmd->command.str, &notfound);
      if ( notfound )
	fprintf(stderr, NAME ": Received unknown command '%s' from display tool\n", cmd->command.str);
    }
    else {
      fprintf(stderr, NAME ": Couldn't handle display tool command '%s'\n", cmd->command.str);
    }
    break;

  case FRAME_CTL_PERIOD:
    capture_set_period(display->root->capture, cmd->period.period);
    break;

  case FRAME_CTL_SOURCE:
    /* Set source termination event handler */
    if ( (cmd->source.args[0] == '\0') || (strcmp(cmd->source.args, "-stop") == 0) )
      shell_set_source_done(display->shell, NULL, NULL);
    else
      shell_set_source_done(display->shell, (shell_event_handler_t *) frame_display_cmd_source_done, display);

    {
      char *argv[2] = { cmd->source.args, NULL };
      if ( shell_source(display->shell, 1, argv) == -1 )
	frame_display_cmd_source_done(display->shell, display);
    }
    break;

  default:
    fprintf(stderr, NAME ": Received unexpected control message from display tool: ctl=%02X len=%u\n", cmd->hdr.ctl, cmd->hdr.len);
    break;
  }
}


/*******************************************************/
/* Display Tool i/o management                         */
/*******************************************************/

static void frame_display_dsock_shutdown(frame_display_t *display)
{
  /* Detach shell history event callback */
  if (display->shell)
	  shell_history_set_callback(&(display->shell->history), NULL, NULL);

  if ( display->dsock_tag > 0 ) {
    g_source_remove(display->dsock_tag);
    display->dsock_tag = 0;
  }

  if ( display->dsock_channel != NULL ) {
    g_io_channel_unref(display->dsock_channel);
    display->dsock_channel = NULL;
  }

  if ( display->dsock > 0 ) {
    shutdown(display->dsock, 2);
    close(display->dsock);
    display->dsock = -1;
  }
}


static int frame_display_read_buf(frame_display_t *display, unsigned char *buf, int size)
{
  int rsize = read(display->dsock, buf, size);

  /* Check for I/O error */
  if ( rsize < 0 ) {
    if ( (errno == ECONNRESET) || (errno == ENETRESET) || (errno == ECONNABORTED) )
      rsize = 0;
    else
      fprintf(stderr, NAME ": read(%s): %s\n", display->sockname, strerror(errno));
  }

  /* Check for disconnect */
  if ( rsize == 0 ) {
    //fprintf(stderr, "-- DISPLAY DISCONNECTED\n");
    frame_display_dsock_shutdown(display);
    return 0;
  }

  return rsize;
}


static frame_ctl_cmd *frame_display_read(frame_display_t *display)
{
  frame_ctl_cmd *cmd;
  int hsize, dsize;

  /* Pre-alloc control message buffer */
  if ( display->dbuf == NULL ) {
    display->dsize = sizeof(frame_ctl_cmd);
    display->dbuf = malloc(display->dsize);
  }

  /* Clear command buffer */
  cmd = (frame_ctl_cmd *) display->dbuf;
  cmd->hdr.ctl = FRAME_CTL_NONE;
  cmd->hdr.len = 0;

  if ( display->dsock < 0 )
    return cmd;

  /* Read control message header */
  hsize = frame_display_read_buf(display, display->dbuf, sizeof(frame_ctl_hdr));
  if ( hsize <= 0 )
    return NULL;

  if ( hsize != sizeof(frame_ctl_hdr) ) {
    fprintf(stderr, NAME ": Received truncated control message header: %d bytes\n", hsize);
    return NULL;
  }

  if ( cmd->hdr.len == 0 ) {
    fprintf(stderr, NAME ": Received null control message length: ctl=%02X\n",cmd->hdr.ctl);
    return NULL;
  }

  /* Alloc control message buffer */
  if ( cmd->hdr.len > FRAME_CTL_MAXSIZE ) {
    fprintf(stderr, NAME ": Received illegal control message length: ctl=%02X len=%u\n", cmd->hdr.ctl, cmd->hdr.len);
    cmd->hdr.len = FRAME_CTL_MAXSIZE;
  }

  if ( display->dsize < cmd->hdr.len ) {
    display->dsize = cmd->hdr.len;
    display->dbuf = realloc(display->dbuf, display->dsize);
  }

  /* Read control message body */
  dsize = cmd->hdr.len - hsize;
  if ( dsize > 0 ) {
    dsize = frame_display_read_buf(display, display->dbuf + hsize, dsize);
    if ( dsize <= 0 )
      return NULL;
  }

  return (frame_ctl_cmd *) display->dbuf;
}


static gboolean frame_display_dsock_event(GIOChannel *source, GIOCondition condition, frame_display_t *display)
{
  frame_ctl_cmd *cmd;

  if ( condition & G_IO_HUP ) {
    frame_display_dsock_shutdown(display);
    return FALSE;
  }

  cmd = frame_display_read(display);
  if ( cmd != NULL ) {
    frame_display_cmd_process(display, cmd);
  }

  return TRUE;
}


static void frame_display_history_event(shell_history_item_t *hsti, shell_history_event_t event, void *data)
{
	frame_display_t *display = data;
	int ctl_id;
	char *ctl_cmd;

	switch (event) {
	case SHELL_HISTORY_ADD:
		ctl_id = hsti->id;
		ctl_cmd = hsti->str;
		break;
	case SHELL_HISTORY_DEL:
		ctl_id = -(hsti->id);
		ctl_cmd = NULL;
		break;
	case SHELL_HISTORY_MOVE_UP:
		ctl_id = hsti->id;
		ctl_cmd = NULL;
		break;
	default: return;
	}

	if (display->dsock > 0)
		frame_ctl_history(display->dsock, ctl_id, ctl_cmd);
}


static int frame_display_csock_accept(frame_display_t *display)
{
  struct sockaddr_un sun;
  int sock;
  socklen_t size;
  frame_geometry_t g;

  /* Accept client connection */
  size = sizeof(sun);
  sock = accept(display->csock, (struct sockaddr *) &sun, &size);
  if ( sock < 0 ) {
    fprintf(stderr, NAME ": accept(%s): %s\n", display->sockname, strerror(errno));
    return -1;
  }

  if ( display->dsock > 0 ) {
    fprintf(stderr, NAME ": Rejecting redundant display connection\n");
    close(sock);
    return 0;
  }

  display->dsock = sock;
  display->dsock_channel = g_io_channel_unix_new(display->dsock);
  display->dsock_tag = g_io_add_watch(display->dsock_channel, G_IO_IN | G_IO_HUP,
				      (GIOFunc) frame_display_dsock_event, display);

  /* Setup shell history event callback */
  if (display->shell != NULL)
	  shell_history_set_callback(&(display->shell->history), frame_display_history_event, display);

  /* Send init message */
  frame_ctl_init(display->dsock, display->root->hdr.shmid, display->root->capture->cap);

  capture_get_window(display->root->capture, &g);
  frame_ctl_window(display->dsock, &g);

  /* Refresh the entire display */
  g.x = 0;
  g.y = 0;
  g.width = display->root->hdr.fb->rgb.width;
  g.height = display->root->hdr.fb->rgb.height;
  frame_display_refresh(display, display->root, &g);

  return 0;
}


static gboolean frame_display_csock_event(GIOChannel *source, GIOCondition condition, frame_display_t *display)
{
  if ( condition & G_IO_HUP ) {
    fprintf(stderr, NAME ": *WARNING* Display connection socket '%s' shut down\n", display->sockname);
    fprintf(stderr, NAME ": *WARNING* Display Tool won't be able to connect any more\n");
    return FALSE;
  }

  frame_display_csock_accept(display);
  return TRUE;
}


int frame_display_connected(frame_display_t *display)
{
  return (display->dsock > 0) ? 1:0;
}


void frame_display_disable(frame_display_t *display)
{
  if ( display == NULL )
    return;

  /* Remove connection socket */
  remove(display->sockname);

  /* Shutdown data socket */
  frame_display_dsock_shutdown(display);
}


void frame_display_destroy(frame_display_t *display)
{
  if ( display == NULL )
    return;

  if (display->shell != NULL) {
	  shell_history_set_callback(&(display->shell->history), NULL, NULL);
	  display->shell = NULL;
  }

  display->pattern_set = NULL;
  display->pattern_get = NULL;
  display->pattern_data = NULL;

  /* Disable display connection */
  frame_display_disable(display);

  /* Shutdown connection socket */
  if ( display->csock_tag > 0 ) {
    g_source_remove(display->csock_tag);
    display->csock_tag = 0;
  }

  if ( display->csock_channel != NULL ) {
    g_io_channel_unref(display->csock_channel);
    display->csock_channel = NULL;
  }

  if ( display->csock > 0 ) {
    shutdown(display->csock, 2);
    close(display->csock);
    display->csock = -1;
  }

  /* Free connection buffers */
  if ( display->dbuf != NULL ) {
    free(display->dbuf);
    display->dbuf = NULL;
    display->dsize = 0;
  }

  if ( display->sockname != NULL ) {
    free(display->sockname);
    display->sockname = NULL;
  }

  free(display);
}


frame_display_t *frame_display_alloc(char *name, frame_t *root)
{
  char *sockdir;
  frame_display_t *display;
  struct sockaddr_un sun;

  /* Create temporary directory */
  sockdir = frame_ctl_sockdir();
  if ( sockdir == NULL )
    return NULL;
  free(sockdir);

  /* Alloc display management structure */
  display = (frame_display_t *) calloc(1, sizeof(frame_display_t));

  display->root = root;

  display->shell = NULL;

  display->pattern_set = NULL;
  display->pattern_get = NULL;
  display->pattern_data = NULL;

  display->sockname = NULL;

  display->csock = -1;
  display->csock_channel = NULL;
  display->csock_tag = 0;

  display->dsock = -1;
  display->dsock_channel = NULL;
  display->dsock_tag = 0;

  display->dbuf = NULL;
  display->dsize = 0;

  /* Create display tool communication socket */
  if ( (display->csock = socket(PF_UNIX, SOCK_STREAM, 0)) == -1 ) {
    perror(NAME ": socket");
    frame_display_destroy(display);
    return NULL;
  }

  /* Bind display tool communication socket */
  display->sockname = frame_ctl_sockname(name);

  if ( access(display->sockname, F_OK) == 0 ) {
    remove(display->sockname);
    sleep(1);
  }

  sun.sun_family = AF_UNIX;
  strcpy(sun.sun_path, display->sockname);
  if ( bind(display->csock, (struct sockaddr *) &sun, sizeof(struct sockaddr_un)) == -1 ) {
    fprintf(stderr, NAME ": bind(%s): %s\n", display->sockname, strerror(errno));
    frame_display_destroy(display);
    return NULL;
  }

  /* Listen for incoming display tool connection */
  if ( listen(display->csock, 0) ) {
    fprintf(stderr, NAME ": listen(%s): %s\n", display->sockname, strerror(errno));
    frame_display_destroy(display);
    return NULL;
  }

  display->csock_channel = g_io_channel_unix_new(display->csock);
  display->csock_tag = g_io_add_watch(display->csock_channel, G_IO_IN | G_IO_HUP,
				      (GIOFunc) frame_display_csock_event, display);

  return display;
}


void frame_display_set_shell(frame_display_t *display, shell_t *shell)
{
  display->shell = shell;
}


void frame_display_set_pattern_operations(frame_display_t *display,
					  frame_display_pattern_operation *set,
					  frame_display_pattern_operation *get,
					  void *data)
{
  display->pattern_set = set;
  display->pattern_get = get;
  display->pattern_data = data;
}


/*******************************************************/
/* Events and responses sent to the Display Tool       */
/*******************************************************/

int frame_display_geometry(frame_display_t *display, frame_geometry_t *g)
{
  /* Requested frame size should be at least 1x1 */
  if ( g->width < 1 )
    g->width = 1;
  if ( g->height < 1 )
    g->height = 1;

  /* Clip active frame geometry */
  frame_rgb_clip_geometry(&(display->root->hdr.fb->rgb), g);

  /* Show active frame geometry */
  if ( display->dsock >= 0 )
    frame_ctl_window(display->dsock, g);

  return 0;
}


void frame_display_refresh(frame_display_t *display, frame_t *frame, frame_geometry_t *g)
{
  if ( (display != NULL) && (display->dsock >= 0) )
    frame_ctl_refresh(display->dsock, frame->hdr.shmid, g);
}


void frame_display_period(frame_display_t *display, unsigned long delay)
{
  if ( display->dsock >= 0 )
    frame_ctl_period(display->dsock, delay);
}


void frame_display_source_done(frame_display_t *display)
{
  if ( display->dsock >= 0 )
    frame_ctl_source(display->dsock, "");
}


void frame_display_pattern_add(frame_display_t *display, pattern_t *pattern)
{
  if ( display->dsock >= 0 )
    frame_ctl_pattern(display->dsock, pattern);
}


void frame_display_pattern_remove(frame_display_t *display, char *id)
{
  if ( display->dsock >= 0 )
    frame_ctl_pattern_remove(display->dsock, id);
}


void frame_display_pattern_show(frame_display_t *display, pattern_t *pattern)
{
  int state;

  if ( display->dsock < 0 )
    return;

  state = (pattern->matched.width > 0);
  frame_ctl_match(display->dsock, pattern->id, state);

  if ( state ) {
	  frame_geometry_t g = pattern->matched;
	  g.x += pattern->frame->g0.x;
	  g.y += pattern->frame->g0.y;
	  frame_ctl_pad(display->dsock, &g, 1);
  }
}


void frame_display_pattern_hide(frame_display_t *display, pattern_t *pattern)
{
  if ( display->dsock >= 0 )
    frame_ctl_match(display->dsock, pattern->id, 0);
}


void frame_display_pad_show(frame_display_t *display, frame_geometry_t *gtab, int nmemb)
{
  if ( display->dsock >= 0 )
    frame_ctl_pad(display->dsock, gtab, nmemb);
}


void frame_display_frame_add(frame_display_t *display, frame_t *frame)
{
  if ( display->dsock >= 0 )
    frame_ctl_frame(display->dsock, frame->parent->hdr.shmid, (frame_hdr_t *) frame);
}


void frame_display_frame_remove(frame_display_t *display, frame_t *frame)
{
  if ( display->dsock >= 0 )
    frame_ctl_frame(display->dsock, -1, (frame_hdr_t *) frame);
}
