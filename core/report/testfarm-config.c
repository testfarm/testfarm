/****************************************************************************/
/* TestFarm                                                                 */
/* Graphical User Interface : Report Options editor                         */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 07-JUN-2004                                                    */
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
#include <gtk/gtk.h>

#include "install.h"
#include "useful.h"
#include "support.h"
#include "report_config_gui.h"

#define NAME "testfarm-config"


static int command_id = -1;

static void command_handler(GtkWidget *window, int fd, GdkInputCondition condition)
{
  char *buf = NULL;
  int size = 0;
  int len;

  if ( window == NULL )
    return;

  len = fgets2(stdin, &buf, &size);

  if ( buf == NULL )
    return;

  if ( len > 0 ) {
    //char *args = buf + 1;

    switch ( buf[0] ) {
    case 'R': /* Raise */
      gtk_window_present(GTK_WINDOW(window));
      break;
    default: /* Ignore other commands */
      break;
    }
  }

  free(buf);
}


static void usage(void)
{
  fprintf(stderr, "TestFarm - Test Report Configuration Editor\n");
  fprintf(stderr, "Usage: " NAME " [<XWindow Options>]\n");

  exit(2);
}


int main (int argc, char *argv[])
{
  GtkWidget *window;

  /* Init GTK stuffs */
  gtk_set_locale();
  gtk_init(&argc, &argv);

  /* Get command arguments */
  if ( argc > 1 )
    usage();

  /* Add pixmap search path */
  {
    char *home = get_home();
    char path[strlen(home)+10];
    snprintf(path, sizeof(path), "%s/icons", home);
    add_pixmap_directory(path);
  }
  add_pixmap_directory (".");

  /* Start Report Config gui */
  window = report_config_gui(NULL);
  gtk_signal_connect_object(GTK_OBJECT(window), "destroy",
                            GTK_SIGNAL_FUNC(gtk_main_quit), (gpointer) NULL);

  /* Setup control command handling */
  command_id = gdk_input_add(fileno(stdin), GDK_INPUT_READ,
			     (GdkInputFunction) command_handler, window);

  gtk_main();

  return 0;
}
