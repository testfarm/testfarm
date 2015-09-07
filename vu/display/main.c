/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* Record & Display Graphical User Interface                                */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 05-JAN-2004                                                    */
/****************************************************************************/

/*
    This file is part of TestFarm,
    the Test Automation Tool for Embedded Software.
    Please visit http://www.testfarm.org.

    TestFarm is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    TestFarm is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with TestFarm.  If not, see <http://www.gnu.org/licenses/>.
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
