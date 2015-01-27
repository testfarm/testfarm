/****************************************************************************/
/* TestFarm                                                                 */
/* Log Viewer                                                               */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 07-DEC-1999                                                    */
/****************************************************************************/

/*
 * $Revision: 271 $
 * $Date: 2006-10-30 18:07:32 +0100 (lun., 30 oct. 2006) $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <gtk/gtk.h>

#include "useful.h"
#include "install.h"
#include "viewer.h"

#define DEFAULT_TIMEOUT 5

static viewer_t *viewer = NULL;
static int timeout_tag = -1;

static int command_id = -1;


static void command_handler(viewer_t *viewer, int fd, GdkInputCondition condition)
{
  char *buf = NULL;
  int size = 0;
  int len;

  if ( viewer == NULL )
    return;

  len = fgets2(stdin, &buf, &size);

  if ( buf == NULL )
    return;

  if ( len > 0 ) {
    char *args = buf + 1;

    switch ( buf[0] ) {
    case 'L': /* Load [<log_file>] */
      viewer_load(viewer, args);
      break;
    case 'F': /* Follow [<mode>] */
      viewer_follow(viewer, atoi(args));
      break;
    case 'C': /* Clear log */
      viewer_clear(viewer);
      break;
    case 'R': /* Raise [<case>] */
      gtk_window_present(GTK_WINDOW(viewer->window));
      if ( *args != '\0' )
        list_moveto_case(viewer->list, args);
      break;
    default: /* Ignore other commands */
      break;
    }
  }

  free(buf);
}


static void shutdown(void *arg)
{
  /* Close remote command handling */
  if ( command_id >= 0 )
    gdk_input_remove(command_id);
  command_id = -1;

  if ( timeout_tag >= 0 )
    g_source_remove(timeout_tag);
  timeout_tag = -1;

  if ( viewer != NULL )
    viewer_done(viewer);
  viewer = NULL;

  exit(EXIT_SUCCESS);
  /*gtk_main_quit();*/
}


static void usage(void)
{
  fprintf(stderr, "Usage: " INSTALL_LOGVIEW " [-f[<seconds>]] [-c] [-t <title>] [log-file]\n");
  exit(EXIT_FAILURE);
}


static gboolean follow(void)
{
  if ( viewer != NULL )
    viewer_follow(viewer, 0);
  return TRUE;
}


int main(int argc, char *argv[])
{
  int opt_follow = 0;
  int opt_command = 0;
  char *opt_title = NULL;
  char *logfile = NULL;
  int i;

  /* Get command arguments */
  for (i = 1; i < argc; i++) {
    char *args = argv[i];

    if ( args[0] == '-' ) {
      if ( strncmp(args, "-f", 2) == 0 ) {
        if ( args[2] != '\0' )
          opt_follow = atoi(args+2);
        if ( opt_follow <= 0 )
          opt_follow = DEFAULT_TIMEOUT;
      }
      else if ( strncmp(args, "-c", 2) == 0 ) {
        opt_command = 1;
      }
      else if ( strncmp(args, "-t", 2) == 0 ) {
        char *s = args + 2;

        if ( *s == '\0' ) {
          s = argv[++i];
          if ( s == NULL )
            usage();
        }

        opt_title = strskip_spaces(s);
      }
      else {
        usage();
      }
    }
    else {
      if ( logfile == NULL )
        logfile = args;
      else
        usage();
    }
  }

  /* GTK gears initialization */
  gtk_init(&argc, &argv);

  /* The viewer */
  viewer = viewer_init(opt_title);
  viewer_destroyed(viewer, shutdown, NULL);
  viewer_load(viewer, logfile);

  /* Setup remote command pipe if any */
  if ( opt_command ) {
    command_id = gdk_input_add(fileno(stdin), GDK_INPUT_READ,
                               (GdkInputFunction) command_handler, (gpointer) viewer);
  }

  /* Setup handling of automatic follow */
  if ( opt_follow ) {
    timeout_tag = g_timeout_add(opt_follow * 1000, (GSourceFunc) follow, NULL);
  }

  /* Processing loop */
  gtk_main();

  return 0;
}
