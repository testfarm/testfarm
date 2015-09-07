/****************************************************************************/
/* Basil Dev - TestFarm                                                     */
/* Test Suite User Interface : file management                              */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 10-MAY-2000                                                    */
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
#include <string.h>
#include <malloc.h>
#include <sys/stat.h>
#include <gtk/gtk.h>

#include "codegen.h"

#include "interface.h"
#include "support.h"
#include "tree_run.h"
#include "xpm_gui.h"
#include "status.h"
#include "tree_gui.h"
#include "check_gui.h"
#include "file_gui.h"


/*============================================================*/
/* File Load dialog window                                    */
/*============================================================*/

static void file_gui_clear(file_gui_t *file)
{
  if ( file->report.window != NULL ) {
    gtk_widget_destroy(file->report.window);
    file->report.window = NULL;
  }
}


static void file_gui_report_clicked(file_gui_t *file);


void file_gui_load(file_gui_t *file, char *filename)
{
  tree_t *tree;

  /* Load Test Tree */
  file_gui_clear(file);

  if ( (tree = tree_run_load(file->tree_run, filename)) == NULL ) {
    status_mesg("Cannot load Test Tree file \"%s\"", filename);
    return;
  }

  gtk_widget_set_sensitive(file->reload, TRUE);
  gtk_widget_set_sensitive(file->check, TRUE);

  if ( (tree->errcount != 0) || (tree->warncount != 0) ) {
    file_gui_report_clicked(file);
    gtk_widget_set_sensitive(file->report.button, TRUE);
  }
  else {
    gtk_widget_set_sensitive(file->report.button, FALSE);
  }
}


void file_gui_reload(file_gui_t *file)
{
  char *filename;

  if ( file->tree_run == NULL )
    return;
  if ( file->tree_run->tree == NULL )
    return;
  if ( file->tree_run->tree->loc.filename == NULL )
    return;

  filename = strdup(file->tree_run->tree->loc.filename);

  file_gui_load(file, filename);

  free(filename);
}


static void file_gui_spot(file_gui_t *file, unsigned long key)
{
  tree_gui_spot(file->tree_run->gui, key);
}


/*============================================================*/
/* File Report dialog window                                  */
/*============================================================*/

enum {
  FILE_GUI_COL_LINE,
  FILE_GUI_COL_PIX,
  FILE_GUI_COL_LEVEL,
  FILE_GUI_COL_MESSAGE,
  FILE_GUI_COL_OBJECT,
  FILE_GUI_NCOLS
};


static void file_gui_report_destroyed(file_gui_t *file)
{
  if ( file->report.list != NULL ) {
    gtk_list_store_clear(file->report.list);
    file->report.list = NULL;
  }
  file->report.window = NULL;
}


static gboolean file_gui_report_select(GtkTreeSelection *selection, GtkTreeModel *model,
				       GtkTreePath *path, gboolean path_currently_selected,
				       file_gui_t *file)
{
  int new_state = path_currently_selected ? 0:1;
  tree_object_t *object = NULL;

  /* Get related object */
  if ( new_state ) {
    GtkTreeIter iter;

    gtk_tree_model_get_iter(model, &iter, path);
    gtk_tree_model_get(model, &iter,
		       FILE_GUI_COL_OBJECT, &object,
		       -1);
  }

  if ( tree_run_loaded(file->tree_run) && (object != NULL) )
    gtk_widget_set_sensitive(file->report.spot, TRUE);
  else
    gtk_widget_set_sensitive(file->report.spot, FALSE);

  file->report.object = object;

  return TRUE;
}


static void file_gui_report_spot(file_gui_t *file)
{
  file_gui_spot(file, file->report.object->key);
}


static void file_gui_report_feed(tree_err_t *err, GtkListStore *model)
{
  char *sline = NULL;
  char *level = "";
  GdkPixbuf *pixbuf = pixbuf_blank;
  GtkTreeIter iter;

  if ( err->loc.filename != NULL ) {
    int len = strlen(err->loc.filename);
    sline = (char *) malloc(len+10);
    strcpy(sline, err->loc.filename);

    if ( err->loc.lineno > 0 )
      sprintf(sline+len, ":%d", err->loc.lineno);
  }

  switch ( err->level ) {
  case TREE_ERR_INFO:
    pixbuf = pixbuf_info;
    level = " INFO ";
    break;
  case TREE_ERR_WARNING:
    pixbuf = pixbuf_warning;
    level = " WARNING ";
    break;
  case TREE_ERR_ERROR:
    pixbuf = pixbuf_error;
    level = " ERROR ";
    break;
  case TREE_ERR_PANIC:
    pixbuf = pixbuf_error;
    level = " PANIC ";
    break;
  default:
    break;
  }

  gtk_list_store_append(model, &iter);
  gtk_list_store_set(model, &iter,
		     FILE_GUI_COL_PIX, pixbuf,
		     FILE_GUI_COL_LEVEL, level,
		     FILE_GUI_COL_MESSAGE, err->msg,
		     FILE_GUI_COL_OBJECT, err->object,
		     -1);

  if ( sline != NULL ) {
    gtk_list_store_set(model, &iter,
		       FILE_GUI_COL_LINE, sline,
		       -1);
    free(sline);
  }
}


static void file_gui_report_clicked(file_gui_t *file)
{
  if ( file->report.window == NULL ) {
    GtkTreeView *view;
    GtkTreeSelection *selection;
    GtkTreeViewColumn *column;
    GtkCellRenderer *renderer;

    file->report.window = create_load_report_window();
    file->report.spot = lookup_widget(file->report.window, "spot");

    /* Connect button signals */
    gtk_signal_connect_object(GTK_OBJECT(file->report.window), "destroy",
			      (GtkSignalFunc) file_gui_report_destroyed, file);
    gtk_signal_connect_object (GTK_OBJECT(lookup_widget(file->report.window, "close")),
                               "clicked", GTK_SIGNAL_FUNC(gtk_widget_destroy),
                               GTK_OBJECT(file->report.window));
    gtk_signal_connect_object(GTK_OBJECT(file->report.spot), "clicked",
			      GTK_SIGNAL_FUNC(file_gui_report_spot), file);

    /* Construct report list */
    view = GTK_TREE_VIEW(lookup_widget(file->report.window, "treeview"));
    file->report.list = gtk_list_store_new(FILE_GUI_NCOLS,
					   /* Line location */ G_TYPE_STRING,
					   /* Error icon    */ GDK_TYPE_PIXBUF,
					   /* Error level   */ G_TYPE_STRING,
					   /* Error message */ G_TYPE_STRING,
					   /* Object        */ G_TYPE_POINTER);
    gtk_tree_view_set_model(view, GTK_TREE_MODEL(file->report.list));
    gtk_tree_view_set_rules_hint(view, TRUE);

    /* Setup list selection handler */
    selection = gtk_tree_view_get_selection(view);
    gtk_tree_selection_set_mode(selection, GTK_SELECTION_SINGLE);
    gtk_tree_selection_set_select_function(selection,
					   (GtkTreeSelectionFunc) file_gui_report_select, file,
					   NULL);

    /* Column #0: Line Number */
    renderer = gtk_cell_renderer_text_new();
    gtk_tree_view_insert_column_with_attributes(view, -1, "Line", renderer,
						"text", FILE_GUI_COL_LINE,
						NULL);
    column = gtk_tree_view_get_column(view, 0);
    gtk_tree_view_column_set_clickable(column, FALSE);
    gtk_tree_view_column_set_resizable(column, FALSE);
    gtk_tree_view_column_set_sizing(column, GTK_TREE_VIEW_COLUMN_AUTOSIZE);

    /* Column #1: Error Level */
    column = gtk_tree_view_column_new();
    gtk_tree_view_column_set_title(column, "Level");

    renderer = gtk_cell_renderer_pixbuf_new();
    gtk_tree_view_column_pack_start(column, renderer, FALSE);
    gtk_tree_view_column_add_attribute(column, renderer, "pixbuf", FILE_GUI_COL_PIX);

    renderer = gtk_cell_renderer_text_new();
    gtk_object_set(GTK_OBJECT(renderer), "scale", 0.6, NULL);
    gtk_tree_view_column_pack_start(column, renderer, TRUE);
    gtk_tree_view_column_add_attribute(column, renderer, "text", FILE_GUI_COL_LEVEL);

    gtk_tree_view_column_set_clickable(column, FALSE);
    gtk_tree_view_column_set_resizable(column, FALSE);
    gtk_tree_view_column_set_sizing(column, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
    gtk_tree_view_append_column(view, column);

    /* Column #2: Error Message */
    renderer = gtk_cell_renderer_text_new();
    gtk_tree_view_insert_column_with_attributes(view, -1, "Message", renderer,
						"text", FILE_GUI_COL_MESSAGE,
						NULL);
    column = gtk_tree_view_get_column(view, 2);
    gtk_tree_view_column_set_clickable(column, FALSE);
    gtk_tree_view_column_set_resizable(column, FALSE);
    gtk_tree_view_column_set_sizing(column, GTK_TREE_VIEW_COLUMN_AUTOSIZE);

    /* Feed the list */
    g_list_foreach(file->tree_run->tree->errlist,
		   (GFunc) file_gui_report_feed, file->report.list);

    /*gtk_widget_set_usize(GTK_WIDGET(file->report.window), 600, 200);*/
    gtk_widget_show(file->report.window);
    gtk_tree_selection_unselect_all(selection);
  }
  else {
    gtk_window_present(GTK_WINDOW(file->report.window));
  }
}


/*============================================================*/
/* File Check dialog window                                   */
/*============================================================*/

static void file_gui_check_clicked(GtkWidget *widget, file_gui_t *fg)
{
  GList *list = tree_check(fg->tree_run->tree);
  check_gui_files(list,
		  (check_gui_spot_t *) file_gui_spot, fg,
		  NULL, NULL);
  g_list_free(list);
}


/*============================================================*/
/* File GUI setup                                             */
/*============================================================*/

file_gui_t *file_gui_init(GtkWidget *window, tree_run_t *tree_run)
{
  file_gui_t *fg;
  GtkWidget *button;

  /* Alloc file descriptor */
  fg = (file_gui_t *) malloc(sizeof(file_gui_t));
  fg->window = window;
  fg->tree_run = tree_run;
  fg->report.button = NULL;
  fg->report.window = NULL;
  fg->report.list = NULL;
  fg->report.object = NULL;

  /* Setup Open button callback */
  button = lookup_widget(window, "session_reload");
  gtk_signal_connect_object(GTK_OBJECT(button), "activate",
			    GTK_SIGNAL_FUNC(file_gui_reload), fg);
  fg->reload = button;
  gtk_widget_set_sensitive(fg->reload, FALSE);

  /* Setup Errors button callback */
  button = lookup_widget(window, "session_errors");
  gtk_signal_connect_object(GTK_OBJECT(button), "activate",
			    GTK_SIGNAL_FUNC(file_gui_report_clicked), fg);
  fg->report.button = button;
  gtk_widget_set_sensitive(fg->report.button, FALSE);

  /* Setup Check button callback */
  button = lookup_widget(window, "session_check");
  gtk_signal_connect(GTK_OBJECT(button), "activate",
                     GTK_SIGNAL_FUNC(file_gui_check_clicked), fg);
  fg->check = button;
  gtk_widget_set_sensitive(fg->check, FALSE);

  return fg;
}


void file_gui_done(file_gui_t *fg)
{
  file_gui_clear(fg);
  fg->tree_run = NULL;

  free(fg);
}
