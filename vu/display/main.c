/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* Record & Display Graphical User Interface                                */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 05-JAN-2004                                                    */
/****************************************************************************/

/*
 * $Revision: 1094 $
 * $Date: 2009-12-27 12:10:08 +0100 (dim., 27 déc. 2009) $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <gtk/gtk.h>

#include "sig.h"

#include "interface.h"
#include "support.h"
#include "window_icon.h"
#include "style.h"
#include "xpm.h"
#include "utils.h"
#include "display.h"


#define NAME "testfarm-vu-display"

GtkWidget *main_window = NULL;


static void main_terminate(void)
{
  display_done();
  xpm_done();
  style_done();
}


static void main_shutdown(void)
{
  sig_done();
  main_terminate();
  exit(0);
}


int options_parse(int *pargc, char ***pargv)
{
  int argc = *pargc;
  char **argv = *pargv;
  int i;

  for (i = 1; i < argc; i++) {
    char *s = argv[i];

    if ( strncmp(s, "-debug=", 7) == 0 ) {
      opt_debug = atoi(s+7);
    }
    else {
      return -1;
    }
  }

  *pargc = argc - i;
  *pargv = argv + i;

  return 0;
}


int main(int argc, char *argv[])
{
  /* Init GTK stuffs */
  gtk_set_locale();
  gtk_init(&argc, &argv);

  /* Get options */
  if ( options_parse(&argc, &argv) ) {
    fprintf(stderr, "Usage: " NAME " [-debug=<level>]\n");
    exit(2);
  }

  /* Add pixmap search path */
  add_pixmap_directory ("/opt/testfarm/icons");

  /* Init RGB display */
  gdk_rgb_init();

  /* Catch termination signals */
  sig_init(main_terminate);
  signal(SIGPIPE, SIG_IGN);

  /* Create main window */
  main_window = create_main_window();
  gtk_widget_realize(main_window);
  gtk_signal_connect_object(GTK_OBJECT(main_window), "destroy",
                            GTK_SIGNAL_FUNC(main_shutdown), (gpointer) NULL);
  set_window_icon(GTK_WINDOW(main_window));

#if 0
  /* Setup the Quit button */
  button = create_button(main_window, "Quit", "exit.xpm");
  gtk_box_pack_end(GTK_BOX(lookup_widget(main_window, "file_box")), button, FALSE, FALSE, 0);
  gtk_signal_connect_object(GTK_OBJECT(button), "clicked",
                            GTK_SIGNAL_FUNC(main_shutdown), (gpointer) NULL);
#endif

  /* Init fonts, colors and styles */
  style_init();
  xpm_init(main_window);

  display_init(GTK_WINDOW(main_window));

  gtk_widget_show(main_window);
  gtk_main();

  return 0;
}
