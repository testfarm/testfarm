/****************************************************************************/
/* TestFarm                                                                 */
/* Graphical User Interface                                                 */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 12-APR-2000                                                    */
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

#include "sig.h"
#include "useful.h"
#include "install.h"
#include "codegen_criticity.h"
#include "validate.h"

#include "options.h"
#include "interface.h"
#include "support.h"
#include "color.h"
#include "style.h"
#include "child.h"
#include "file_gui.h"
#include "status_gui.h"
#include "xpm_gui.h"
#include "tree_run.h"


#define BANNER "testfarm-run"


GtkWidget *main_window = NULL;
file_gui_t *main_file = NULL;
tree_run_t *main_tree = NULL;


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


void main_terminate(void)
{
  /* Close control command handling */
  if ( command_id >= 0 )
    gdk_input_remove(command_id);
  command_id = -1;

  if ( main_file != NULL ) {
    file_gui_done(main_file);
    main_file = NULL;
  }

  if ( main_tree != NULL ) {
    tree_run_destroy(main_tree);
    main_tree = NULL;
  }

  status_gui_done();
  xpm_gui_done();
  style_done();
  color_done();
  child_done();

  /* Destroy validation states definitions */
  validate_destroy();

  /* Destroy criticity level definitions */
  criticity_destroy();
}


void main_shutdown(void)
{
  sig_done();
  main_terminate();
  exit(0);
}


static GtkWidget *about_window = NULL;

static void about_destroyed(void)
{
  about_window = NULL;
}

static void about(void)
{
  GtkWidget *w;

  /* Bring the window to front if it already exists */
  if ( about_window != NULL ) {
    gtk_window_present(GTK_WINDOW(about_window));
    return;
  }

  /* Create the window */
  about_window = create_about_window();
  gtk_signal_connect_object(GTK_OBJECT(about_window), "destroy",
			    GTK_SIGNAL_FUNC(about_destroyed), NULL);
  gtk_signal_connect_object(GTK_OBJECT(lookup_widget(about_window, "ok")), "clicked",
			    GTK_SIGNAL_FUNC(gtk_widget_destroy), about_window);

  /* Display the logo */
  w = lookup_widget(about_window, "logo");
  gtk_image_set_from_pixbuf(GTK_IMAGE(w), pixbuf_logo);

  /* Display the version */
  w = lookup_widget(about_window, "version");
  gtk_label_set_text(GTK_LABEL(w), "Version " VERSION);

  gtk_widget_show(about_window);
}


void main_usage(void)
{
  fprintf(stderr, "Usage: " BANNER " [<XWindow Options>] ");
  opt_usage(1);
  fprintf(stderr, " [<Tree File> ...]\n");
  opt_usage(0);
  exit(2);
}


int main (int argc, char *argv[])
{
  int argx;
  GtkWidget *pixmap;

  /* Init GTK stuffs */
  gtk_set_locale();
  gtk_init(&argc, &argv);

  /* Get options */
  argx = opt_get(argc, argv);
  if ( argx < 0 )
    main_usage();

  /* Add pixmap search path */
  {
    char *home = get_home();
    char path[strlen(home)+10];
    snprintf(path, sizeof(path), "%s/icons", home);
    add_pixmap_directory(path);
  }
  add_pixmap_directory (".");

  /* Disable stdio buffering */
  setbuf(stdin, NULL);
  setlinebuf(stdout);

  /* Catch termination signals */
  sig_init(main_terminate);

  /* Load criticity level definitions */
  criticity_init();

  /* Load validation states definitions */
  validate_init();

  /* Init child process management */
  child_init();

  /* Create main window */
  if ( ! opt_nogui ) {
    main_window = create_main_window();
    gtk_widget_realize(main_window);
    gtk_signal_connect_object(GTK_OBJECT(main_window), "destroy",
			      GTK_SIGNAL_FUNC(main_shutdown), NULL);

    /* Setup the Quit button */
    gtk_signal_connect_object(GTK_OBJECT(lookup_widget(main_window, "session_quit")), "activate",
			      GTK_SIGNAL_FUNC(main_shutdown), NULL);

    /* Setup the About button */
    gtk_signal_connect_object(GTK_OBJECT(lookup_widget(main_window, "help_about")), "activate",
			      GTK_SIGNAL_FUNC(about), NULL);
    /* Setup GUI helpers */
    color_init();
    style_init();
    xpm_gui_init();
    status_gui_init(main_window);

    /* Display the logo */
    pixmap = lookup_widget(main_window, "main_pixmap");
    gtk_image_set_from_pixbuf(GTK_IMAGE(pixmap), pixbuf_logo);
  }

  /* Setup Test Suite tree viewer */
  main_tree = tree_run_init(main_window);

  /* Setup Test Suite file loading */
  if ( ! opt_nogui ) {
    main_file = file_gui_init(main_window, main_tree);
  }

  if ( argv[argx] != NULL ) {
    char *filename = argv[argx];

    if ( main_file != NULL ) {
      file_gui_load(main_file, filename);
    }
    else {
      if ( tree_run_load(main_tree, filename) == NULL ) {
	status_mesg("Cannot load Test Tree file \"%s\"", filename);
	exit(1);
      }
    }
  }

  /* Setup control command handling */
  if ( opt_command && (! opt_nogui) ) {
    command_id = gdk_input_add(fileno(stdin), GDK_INPUT_READ,
                               (GdkInputFunction) command_handler, main_window);
  }

  /* Here we go... */
  gtk_main();

  return 0;
}

