/****************************************************************************/
/* Basil Dev - TestFarm                                                     */
/* Test Suite generator : Files modification checker                        */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 13-MAY-2004                                                    */
/****************************************************************************/

/*
 * $Revision: 765 $
 * $Date: 2007-10-05 12:53:10 +0200 (ven., 05 oct. 2007) $
 */

#include <stdio.h>
#include <gtk/gtk.h>

#include "codegen.h"

#include "options.h"
#include "interface.h"
#include "support.h"
#include "xpm_gui.h"
#include "check_gui.h"


enum {
  CHECK_GUI_COL_FILENAME,
  CHECK_GUI_COL_PIX,
  CHECK_GUI_COL_ACTION,
  CHECK_GUI_COL_KEY,
  CHECK_GUI_NCOLS
};


static void check_gui_done(GtkWidget *window)
{
  check_gui_done_t *done = gtk_object_get_data(GTK_OBJECT(window), "done_func");
  void *data = gtk_object_get_data(GTK_OBJECT(window), "done_data");

  if ( done != NULL )
    done(data);
}


static void check_gui_spot_clicked(GtkWidget *window)
{
  check_gui_spot_t *spot = gtk_object_get_data(GTK_OBJECT(window), "spot_func");
  void *data = gtk_object_get_data(GTK_OBJECT(window), "spot_data");
  unsigned long key = GPOINTER_TO_INT(gtk_object_get_data(GTK_OBJECT(window), "key"));

  if ( spot != NULL )
    spot(data, key);
}


static void check_gui_start_clicked(GtkWidget *window)
{
  check_gui_done(window);
  gtk_widget_destroy(window);
}


static gboolean check_gui_select(GtkTreeSelection *selection, GtkTreeModel *model,
				 GtkTreePath *path, gboolean path_currently_selected,
				 GtkWidget *window)
{
  int new_state = path_currently_selected ? 0:1;
  GtkWidget *button = lookup_widget(window, "spot");
  unsigned long key = 0;

  /* Get related object */
  if ( new_state ) {
    GtkTreeIter iter;

    gtk_tree_model_get_iter(model, &iter, path);
    gtk_tree_model_get(model, &iter,
		       CHECK_GUI_COL_KEY, &key,
		       -1);
  }

  gtk_object_set_data(GTK_OBJECT(window), "key", GINT_TO_POINTER(key));

  gtk_widget_set_sensitive(button, new_state);

  return TRUE;
}


static void check_gui_files_feed(tree_object_t *obj, GtkListStore *model)
{
  tree_item_t *item = obj->parent_item;
  char *filename;
  GdkPixbuf *pixbuf;
  char *action;

  switch ( obj->type ) {
  case TYPE_SEQ:
    filename = item->loc.filename;
    if ( item == item->parent_tree->head )
      pixbuf = pixbuf_tree;
    else
      pixbuf = pixbuf_seq;
    action = "Reload the Test Tree";
    break;

  case TYPE_CASE:
    filename = obj->d.Case->script->wizname;
    pixbuf = pixbuf_case;
    action = "Recompile the Test Suite";
    break;

  default:
    filename = NULL;
    pixbuf = NULL;
    action = NULL;
    break;
  }

  if ( filename != NULL ) {
    GtkTreeIter iter;

    gtk_list_store_append(model, &iter);
    gtk_list_store_set(model, &iter,
		       CHECK_GUI_COL_FILENAME, filename,
		       CHECK_GUI_COL_PIX, pixbuf,
		       CHECK_GUI_COL_ACTION, action,
		       CHECK_GUI_COL_KEY, obj->key,
		       -1);
  }
}


void check_gui_files(GList *list,
		     check_gui_spot_t *spot, void *spot_data,
		     check_gui_done_t *done, void *done_data)
{
  GtkWidget *window;
  GtkWidget *button;
  GtkTreeView *view;
  GtkListStore *model;
  GtkTreeViewColumn *column;
  GtkCellRenderer *renderer;
  GtkTreeSelection *selection;

  /* Create file alteration report window */
  window = create_check_window();
  gtk_object_set_data(GTK_OBJECT(window), "spot_func", spot);
  gtk_object_set_data(GTK_OBJECT(window), "spot_data", spot_data);
  gtk_object_set_data(GTK_OBJECT(window), "done_func", done);
  gtk_object_set_data(GTK_OBJECT(window), "done_data", done_data);
  gtk_object_set_data(GTK_OBJECT(window), "key", GINT_TO_POINTER(0));

  button = lookup_widget(window, "spot");
  gtk_signal_connect_object(GTK_OBJECT(button), "clicked",
                            GTK_SIGNAL_FUNC(check_gui_spot_clicked), window);
  gtk_widget_set_sensitive(button, FALSE);

  if ( done != NULL ) {
    button = lookup_widget(window, "go");
    gtk_widget_show_all(button);
    gtk_signal_connect_object(GTK_OBJECT(button), "clicked",
                              GTK_SIGNAL_FUNC(check_gui_start_clicked), window);

    button = lookup_widget(window, "cancel");
  }
  else {
    button = lookup_widget(window, "close");
  }
  gtk_widget_show_all(button);
  gtk_signal_connect_object(GTK_OBJECT(button), "clicked",
                            GTK_SIGNAL_FUNC(gtk_widget_destroy), window);

  /* Setup check list viewer */
  view = GTK_TREE_VIEW(lookup_widget(window, "treeview"));

  model = gtk_list_store_new(CHECK_GUI_NCOLS,
			     /* File Name */ G_TYPE_STRING,
			     /* Node pix  */ GDK_TYPE_PIXBUF,
			     /* Action    */ G_TYPE_STRING,
			     /* Key */       G_TYPE_ULONG);
  gtk_tree_view_set_model(view, GTK_TREE_MODEL(model));

  /* Setup list selection handler */
  selection = gtk_tree_view_get_selection(view);
  gtk_tree_selection_set_mode(selection, GTK_SELECTION_SINGLE);
  gtk_tree_selection_set_select_function(selection,
					 (GtkTreeSelectionFunc) check_gui_select, window,
					 NULL);

  /* Column #0: Verdict and Criticity */
  column = gtk_tree_view_column_new();
  gtk_tree_view_column_set_title(column, "File Name");

  renderer = gtk_cell_renderer_pixbuf_new();
  gtk_tree_view_column_pack_start(column, renderer, FALSE);
  gtk_tree_view_column_add_attribute(column, renderer, "pixbuf", CHECK_GUI_COL_PIX);

  renderer = gtk_cell_renderer_text_new();
  gtk_tree_view_column_pack_start(column, renderer, TRUE);
  gtk_tree_view_column_add_attribute(column, renderer, "text", CHECK_GUI_COL_FILENAME);

  gtk_tree_view_column_set_clickable(column, FALSE);
  gtk_tree_view_column_set_resizable(column, TRUE);
  gtk_tree_view_column_set_sizing(column, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
  gtk_tree_view_column_set_expand(column, TRUE);
  gtk_tree_view_append_column(view, column);

  /* Column #1: Suggested Action */
  renderer = gtk_cell_renderer_text_new();
  gtk_tree_view_insert_column_with_attributes(view, -1, "Suggested Action", renderer,
					      "text", CHECK_GUI_COL_ACTION,
					      NULL);
  column = gtk_tree_view_get_column(view, 1);
  gtk_tree_view_column_set_clickable(column, FALSE);
  gtk_tree_view_column_set_resizable(column, TRUE);
  gtk_tree_view_column_set_sizing(column, GTK_TREE_VIEW_COLUMN_AUTOSIZE);

  /* Feed the check list */  
  g_list_foreach(list, (GFunc) check_gui_files_feed, model);

  gtk_widget_show(window);
  gtk_tree_selection_unselect_all(selection);
}
