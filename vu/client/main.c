/**********************************************************************/
/* TestFarm Virtual User                                              */
/* TVU Engine program body                                            */
/**********************************************************************/
/* Author: Sylvain Giroudon                                           */
/* Creation: 14-JUN-2006                                              */
/**********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <glib.h>

#include "useful.h"
#include "child.h"
#include "sig.h"
#include "shell.h"
#include "log.h"
#include "link.h"
#include "link_txt.h"
#include "link_cmd.h"

#include "options.h"
#include "error.h"
#include "keysyms.h"
#include "capture.h"
#include "frame_buf.h"
#include "frame_geometry.h"
#include "frame_display.h"
#include "frame_command.h"
#include "color.h"
#include "match_text.h"
#include "match_command.h"
#include "km.h"
#include "pad.h"
#include "frame.h"
#include "grab.h"
#include "fuzz.h"


#define PROMPT  "VU> "


static shell_t *cmd_shell = NULL;

static GMainLoop *loop = NULL;
static capture_t *capture = NULL;
static frame_t *root = NULL;
static frame_display_t *display = NULL;
static pad_t *pad = NULL;

/* The refresh_dump mode flag.
   When enabled, every updated frame area are reported */
static int dump_update = 0;


/*******************************************************/
/* Frame update                                        */
/*******************************************************/

static void update_handler(frame_geometry_t *g, void *data)
{
  if ( g ) {
    if ( dump_update ) {
      printf("%sUPDATE %s\n", log_hdr_(CAPTURE_TAG), frame_geometry_str(g));
    }

    /* Update display and perform all processing */
    frame_update(root, g);
  }
  else {
    /* NULL geometry means capture processing overload */
    printf("%sCapture overload\n", log_hdr_(CAPTURE_TAG));
  }

  /* Process grab recording */
  grab_update(g);
}


/*******************************************************/
/* pad                                                 */
/*******************************************************/

static int pad_command_add(char *id, int argc, char **argv)
{
  char *hdr = log_hdr_(PAD_TAG);
  frame_t *frame = root;
  char *window_str = NULL;
  frame_geometry_t window;
  unsigned long color_value = 0;
  fuzz_t fuzz = FUZZ_NULL;
  unsigned int gap = 0;
  unsigned int min_width = 1;
  unsigned int min_height = 1;
  int argx;

  /* Parse command arguments */
  for (argx = 0; argx < argc; argx++) {
    char *str = argv[argx];
    char *eq = strchr(str, '=');

    if ( eq != NULL ) {
      *(eq++) = '\0';

      if ( strcmp(str, "frame") == 0 ) {
	frame = frame_get_child_by_id(root, eq);
	if ( frame == NULL ) {
	  error(NULL, "Unknown frame '%s'", eq);
	  return -1;
	}
      }
      else if ( strcmp(str, "window") == 0 ) {
	window_str = eq;
      }
      else if ( strcmp(str, "gap") == 0 ) {
	gap = strtoul(eq, NULL, 0);
      }
      else if ( strcmp(str, "color") == 0 ) {
	if ( color_parse(eq, &color_value) ) {
	  error(NULL, "Syntax error in color specification");
	  return -1;
	}
      }
      else if ( strcmp(str, "fuzz") == 0 ) {
	if ( fuzz_parse(&fuzz, eq) ) {
	  error(NULL, "Syntax error in fuzz specification");
	  return -1;
	}
      }
      else if ( strcmp(str, "min") == 0 ) {
	char *sheight;

	if ( (sheight = strchr(eq, 'x')) != NULL ) {
	  *(sheight++) = '\0';
	  min_width = atoi(eq);
	  min_height = atoi(sheight);
	}
	else {
	  min_width = min_height = atoi(eq);
	}
      }
      else {
	error(NULL, "pad: Unknown option '%s'", str);
	return -1;
      }
    }
  }

  /* Set window geometry */
  if ( window_str != NULL ) {
    if ( frame_rgb_parse_geometry(&(frame->hdr.fb->rgb), window_str, &window) ) {
      error(NULL, "Syntax error in window geometry");
      return -1;
    }
  }
  else {
    window = frame->g;
    window.x = 0;
    window.y = 0;
  }

  pad_add(pad, id, frame, &window, color_value, &fuzz, gap, min_width, min_height);
  pad_show(pad, id, hdr);

  return 0;
}


static int pad_command(shell_t *shell, shell_argv_t *cmd_argv, char *tag)
{
  int argc = cmd_argv->argc;
  char **argv = cmd_argv->argv;
  char *op;
  char *id;

  /* Set error reporting tag */
  error_default_tag(tag);

  if ( argc < 2 ) {
    shell_std_help(shell, argv[0]);
    return -1;
  }
  op = argv[1];
  id = argv[2];

  /* Operation 'add' */
  if ( strcmp(op, "add") == 0 ) {
    /* Process 'pad add' command */
    if ( pad_command_add(id, argc-3, argv+3) )
      return -1;
  }

  /* Operation 'remove' */
  else if ( strcmp(op, "remove") == 0 ) {
    /* Check number of arguments */
    if ( check_argc(shell, cmd_argv, tag, 2, 3) )
      return -1;

    if ( pad_remove(pad, id) ) {
      error(NULL, "remove: Unknown pad %s", id);
      return -1;
    }
  }

  /* Operation 'show' */
  else if ( strcmp(op, "show") == 0 ) {
    /* Check number of arguments */
    if ( check_argc(shell, cmd_argv, tag, 2, 4) )
      return -1;

    pad_show(pad, id, log_hdr_(PAD_TAG));
  }

  /* Unknown operation */
  else {
    error(NULL, "Unknown operation '%s'", op);
    shell_std_help(shell, argv[0]);
    return -1;
  }

  return 0;
}


/*******************************************************/
/* getpix                                              */
/*******************************************************/

static int getpix_command(shell_t *shell, shell_argv_t *cmd_argv, char *tag)
{
  char **argv = cmd_argv->argv;
  char *hdr = log_hdr_(tag);
  frame_geometry_t g = FRAME_GEOMETRY_NULL;
  frame_rgb_t *rgb;
  unsigned char *pix;
  char *str;

  /* Set error reporting tag */
  error_default_tag(tag);

  /* Check number of arguments */
  if ( check_argc(shell, cmd_argv, tag, 2, 2) )
    return -1;

  str = argv[1];

  if ( ((str[0] != '+') && (str[0] != '-')) || frame_geometry_parse_position(str, &g) ) {
    error(NULL, "Syntax error in pixel position");
    shell_std_help(shell, argv[0]);
    return -1;
  }

  rgb = &(root->hdr.fb->rgb);
  frame_rgb_clip_geometry(rgb, &g);
  pix = rgb->buf + (g.y * rgb->rowstride) + (g.x * rgb->bpp);
  printf("%s+%d+%d #%02X%02X%02X\n", hdr, g.x, g.y, pix[0], pix[1], pix[2]);

  return 0;
}


/*******************************************************/
/* status                                              */
/*******************************************************/

static int status_command(shell_t *shell, shell_argv_t *cmd_argv, char *tag)
{
  char *hdr;

  /* Check argument list */
  if ( check_argc(shell, cmd_argv, tag, 0, 1) )
    return -1;

  hdr = log_hdr_(tag);

  printf("%sDisplay Environment:\n", hdr);
  printf("%s  Root frame SHM id: %d\n", hdr, root->hdr.shmid);
  printf("%s  Display Tool: %sconnected\n", hdr, frame_display_connected(display) ? "":"dis");

  capture_show_status(capture, hdr);

  return 0;
}


/*******************************************************/
/* version                                             */
/*******************************************************/

static int version_command(shell_t *shell, shell_argv_t *cmd_argv, char *tag)
{
  /* Check arguments */
  if ( check_argc(shell, cmd_argv, tag, 0, 1) )
    return -1;

  /* Show version */
  printf("%sTestFarm Virtual User " VERSION " (" __DATE__ ")\n", log_hdr_(tag));

  return 0;
}


/*******************************************************/
/* mode switching                                      */
/*******************************************************/

static int mode_xxx(char *hdr, char *flag, char *value, int *ptr)
{
  if ( value != NULL ) {
    /* Parse value */
    if ( strcmp(value, "on") == 0 )
      *ptr = 1;
    else if ( strcmp(value, "off") == 0 )
      *ptr = 0;
    else {
      error(NULL, "mode %s: value should be 'on' or 'off'", flag);
      return -1;
    }
  }

  /* Show current value */
  printf("%s%s %s\n", hdr, flag, *ptr ? "on":"off");

  return 0;
}


static int mode_prompt(char *hdr, char *value, shell_t *shell)
{
  static int latch = 0;
  int ret = mode_xxx(hdr, "prompt", value, &latch);

  /* Switch mode */
  if ( ret == 0 )
    shell_set_input_echo(shell, latch ? shell_std_input_echo : NULL, NULL);

  return ret;
}


static int mode_dump_update(char *hdr, char *value)
{
  return mode_xxx(hdr, "dump:update", value, &dump_update);
}


static int mode_command(shell_t *shell, shell_argv_t *cmd_argv, char *tag)
{
  int argc = cmd_argv->argc;
  char **argv = cmd_argv->argv;
  char *hdr = log_hdr_(tag);
  char *flag, *value;

  error_default_tag(tag);

  /* Check arguments */
  if ( check_argc(shell, cmd_argv, tag, 1, 3) )
    return -1;

  /* Show all flags if command has no argument */
  if ( argc < 2 ) {
    mode_prompt(hdr, NULL, shell);
    mode_dump_update(hdr, NULL);
    return 0;
  }

  flag = argv[1];
  value = argv[2];

  if ( strcmp(flag, "prompt") == 0 ) {
    if ( mode_prompt(hdr, value, shell) )
      return -1;
  }
  else if ( strcmp(flag, "dump:update") == 0 ) {
    if ( mode_dump_update(hdr, value) )
      return -1;
  }
  else {
    /* Unknown mode flag */
    error(NULL, "mode: Unknown flag '%s'", flag);
    if ( shell->interactive ) {
      fprintf(stderr, "Available mode flags:\n");
      fprintf(stderr, "  prompt      on|off\n");
      fprintf(stderr, "  dump:text   on|off\n");
      fprintf(stderr, "  dump:update on|off\n");
    }
    return -1;
  }

  return 0;
}


/*******************************************************/
/* Help                                                */
/*******************************************************/

static shell_std_help_t help_tab[] = {
  { "mode",    "mode [<flag> [on|off]]\n"
               "  Switch/show a mode flag.\n" },
  { "version", "version\n"
               "  Show command interpreter version\n" },
  { "status",  "status\n"
               "  Report device status\n"},
  { "sleep",   "sleep <time> [h|min|s|ms]\n"
               "  Wait <time> milliseconds or [hours|minutes|seconds|milliseconds]\n" },
  { "pad",     "pad add <id> [window=<geometry>] color=#<rgb-value> [fuzz=#<rgb-fuzz>] [gap=<npixels>] [min=<pad-size>]\n"
               "  Create a pad list containing color <rgb-value>, with optional color fuzz range <rgb-fuzz>\n"
               "  If frame=<id> is specified, padding is performed on this frame (root frame by default).\n"
               "  If window=<geometry> is specified, color padding occurs within this window (XWindow geometry format).\n"
               "  Pads that are closer to <npixels> are merged together.\n"
               "  Pads that are smaller to <min-size> are ignored.\n"
               "pad remove [<id>]\n"
               "  Remove pad list <id>, or all pad lists if <id> is omitted\n"
               "pad show [<id>]\n"
               "  Show pad list <id>, or all pad lists if <id> is omitted\n" },
  { "getpix",  "getpix +<x>+<y>\n"
               "  Show pixel RGB values at position <x>,<y>\n" },
  { NULL,     NULL }
};


/*******************************************************/
/* Script interpreter                                  */
/*******************************************************/

static shell_command_t cmd_tab[] = {
  { "pad",     pad_command,     PAD_TAG   },
  { "getpix",  getpix_command,  "GETPIX " },
  { "status",  status_command,  "STATUS " },
  { "version", version_command, "VERSION" },
  { "sleep",   shell_std_sleep, "SLEEP  " },
  { "mode",    mode_command,    "MODE   " },
  { NULL,      shell_std_unknown, NULL }
};


static int prologue(shell_t *shell, void *arg)
{
  /* Show prompt if interactive mode */
  if ( shell->interactive ) {
    printf(PROMPT);
    fflush(stdout);
  }

  return 0;
}


static void terminate(void)
{
  char *hdr = log_hdr_(TAG_DONE);

  /* Disable display connection */
  frame_display_disable(display);

  /* Prevent termination from being restarted by a signal */
  sig_done();
  signal(SIGTERM, SIG_IGN);

  /* Kill all links and their subprocesses */
  link_done(hdr);

  /* Shut down capture device */
  if ( capture != NULL )
    capture_close(capture);
  capture = NULL;

  /* Free frame buffer display */
  if ( display != NULL )
    frame_display_destroy(display);
  display = NULL;

  /* Free all frame buffers */
  if ( root != NULL )
    frame_free(root, NULL);
  root = NULL;

  /* Free pad management */
  if ( pad != NULL )
    pad_destroy(pad);
  pad = NULL;

  /* Uninstall keyboard/mouse commands */
  km_done(cmd_shell);

  /* Uninstall grab commands */
  grab_done(cmd_shell);

  /* Uninstall pattern matching commands */
  match_command_done(cmd_shell);

  /* Uninstall frame commands */
  frame_command_done(cmd_shell);

  /* Free shell manager */
  if ( cmd_shell != NULL ) {
    shell_free(cmd_shell);
    cmd_shell = NULL;
  }

  /* Kill key symbols */
  keysyms_destroy();

  if ( loop != NULL )
    g_main_loop_quit(loop);
  loop = NULL;
}


static void usage(void)
{
  fprintf(stderr, "TestFarm Virtual User Engine - (c) Basil Dev 2006-2008\n");
  fprintf(stderr, "Usage: " NAME " [-debug] [-shared] [-name <display-name>] <method>://<device> [<script> ...]\n");
  fprintf(stderr, "  -debug: Show debug messages during device connection\n");
  fprintf(stderr, "  -shared: Connect to VNC server in shared mode\n");
  fprintf(stderr, "  -name <display-name>: Change display name for the Display Tool\n");
  fprintf(stderr, "  -rotate-cw: Rotate physical frame 90 deg. clockwise\n");
  fprintf(stderr, "  -rotate-ccw: Rotate physical frame 90 deg. counter-clockwise\n");
  fprintf(stderr, "  -filter <program>: Use <program> as a filter agent\n");
  fprintf(stderr, "  <method>://<device> : source device specification\n");
  fprintf(stderr, "    Image file : [file://]<filename>\n");
  fprintf(stderr, "      <filename>: Image file (png/ppm/ppm.gz/jpg)\n");
  fprintf(stderr, "    VNC server (RFB protocol) : rfb://[<password>@]<host>[:<port>]\n");
  fprintf(stderr, "      <password>: VNC server password (if required)\n");
  fprintf(stderr, "      <host>: VNC server host name\n");
  fprintf(stderr, "      <port>: VNC server display/port number (default=0)\n");
  fprintf(stderr, "    V4L (Video for Linux) capture device : v4l://[<device>][:<input>]\n");
  fprintf(stderr, "      <device>: Device node name (default=/dev/video)\n");
  fprintf(stderr, "      <input>: Video input number (default=0)\n");
  fprintf(stderr, "  <script>: Script file [followed by some arguments]\n");
  exit(2);
}


int main(int argc, char *argv[])
{
  int interactive = isatty(STDIN_FILENO);
  capture_opt_t capture_opt = 0;
  char *method, *device, *name;
  int ret = EXIT_FAILURE;

  /* Retrieve command options */
  if ( options_parse(&argc, &argv) )
    usage();
  if ( opt_shared )
    capture_opt |= CAPTURE_OPT_SHARED;
  if ( opt_debug )
    capture_opt |= CAPTURE_OPT_DEBUG;
  if ( opt_rotate != 0 ) {
    capture_opt |= CAPTURE_OPT_ROTATE;
    if ( opt_rotate < 0 )
      capture_opt |= CAPTURE_OPT_ROTATE_CCW;
  }

  /* Check number of remaining arguments */
  if ( argc < 1 )
    usage();

  /* Parse source device specification */
  method = strdup(*argv);
  device = strstr(method, "://");
  if ( device != NULL ) {
    *device = '\0';
    device += 3;
  }
  else {
    device = method;
    method = strdup("file");
  }

  argc--;
  argv++;

  /* If a file argument is supplied, disable interactive mode */
  if ( argc > 0 )
    interactive = 0;

  /* Hook termination procedure at exit stack and signal handling */
  atexit(terminate);
  sig_init(terminate);
  signal(SIGPIPE, SIG_IGN);

  /* Init child process management */
  child_init();
  child_use_glib();

  /* Enable multithreading */
  if ( !g_thread_supported() )
    g_thread_init(NULL);

  /* Init input link management */
  if ( link_txt_init() == -1 )
    exit(EXIT_FAILURE);
  if ( link_init() == -1 )
    exit(EXIT_FAILURE);
  link_use_glib();

  /* Disable i/o buffering */
  setbuf(stdin, NULL);
  setbuf(stdout, NULL);

  /* Setup keyboard key symbols */
  keysyms_init();

  /* Setup capture device */
  if ( capture_init(method) )
    usage();

  capture = capture_open(device, capture_opt, update_handler, NULL);
  if ( capture == NULL ) {
    fprintf(stderr, "Failed to open capture device %s://%s\n", method, device);
    usage();
  }

  /* Alloc root frame buffer */
  root = frame_alloc_root(capture);
  if ( root == NULL )
    exit(1);

  /* Init frame display */
  name = opt_name ? opt_name : capture->name;
  display = frame_display_alloc(name, root);
  if ( display == NULL )
    exit(1);

  /* Request initial frame buffer update */
  capture_refresh(capture);

  /* Setup pad management */
  pad = pad_alloc(display);
  if ( pad == NULL )
    exit(1);

  /* Create command shell interpreter */
  cmd_shell = shell_alloc(NAME, argc, argv, NULL);

  /* Create main loop */
  loop = g_main_loop_new(NULL, FALSE);
  shell_use_glib(cmd_shell, loop);

  /* Set interactive mode if standard input is a terminal */
  shell_set_interactive(cmd_shell, interactive);

  /* Setup prologue routine, used for displaying user's prompt */
  shell_set_prologue(cmd_shell, prologue, NULL);

  /* Input echo : default configuration */
  shell_set_input_echo(cmd_shell, NULL, NULL);

  /* Setup special commands */
  shell_set_cmd(cmd_shell, cmd_tab);
  frame_command_init(cmd_shell, display);
  match_command_init(cmd_shell, display);
  km_init(cmd_shell, root);
  grab_init(cmd_shell, display);
  link_cmd_init(cmd_shell);

  /* Setup standard commands, standard header routine, help table */
  shell_std_setup(cmd_shell, help_tab);
  shell_std_set_header(log_hdr_);

  /* Declare shell interpreter to Display Tool manager */
  frame_display_set_shell(display, cmd_shell);

  /* Enter processing loop */
  if ( shell_exec(cmd_shell) == 0 ) {
    g_main_loop_run(loop);
    ret = cmd_shell->exit_code;
  }

  /* Destroy main loop */
  g_main_loop_unref(loop);
  loop = NULL;

  terminate();

  return ret;
}
