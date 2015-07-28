/****************************************************************************/
/* TestFarm                                                                 */
/* Graphical User Interface : Test Tree execution                           */
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
    along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <gtk/gtk.h>

#include "interface.h"
#include "support.h"
#include "color.h"
#include "xpm_gui.h"
#include "tree_run.h"
#include "tree_gui.h"


enum {
  TREE_GUI_COL_TYPE_PIXBUF=0,
  TREE_GUI_COL_NAME,
  TREE_GUI_COL_BRKPT_PIXBUF,
  TREE_GUI_COL_BRKPT,
  TREE_GUI_COL_FLAGS_PIXBUF,
  TREE_GUI_COL_SKIP,
  TREE_GUI_COL_VERDICT_PIXBUF,
  TREE_GUI_COL_CRITICITY,
  TREE_GUI_COL_CRITICITY_COLOR,
  TREE_GUI_COL_BACKGROUND,
  TREE_GUI_COL_STYLE,
  TREE_GUI_COL_KEY,
  TREE_GUI_COL_OBJECT,
  TREE_GUI_NCOLS
};


/*===========================================*/
/* Object storage                            */
/*===========================================*/

typedef struct {
  unsigned long key;
  gboolean found;
  GtkTreeIter iter;
} tree_gui_get_iter_data_t;

static gboolean tree_gui_get_iter_item(GtkTreeModel *model,
				       GtkTreePath *path, GtkTreeIter *iter,
				       tree_gui_get_iter_data_t *data)
{
  tree_object_t *object;

  gtk_tree_model_get(model, iter,
		     TREE_GUI_COL_OBJECT, &object,
		     -1);

  if ( (object != NULL) && (object->key == data->key) ) {
    data->found = TRUE;
    data->iter = *iter;
  }

  return data->found;
}

static gboolean tree_gui_get_iter(tree_gui_t *gui, unsigned long key, GtkTreeIter *iter)
{
  tree_gui_get_iter_data_t data;

  data.key = key;
  data.found = FALSE;
  gtk_tree_model_foreach(GTK_TREE_MODEL(gui->tree_model),
			 (GtkTreeModelForeachFunc) tree_gui_get_iter_item, &data);

  if ( data.found )
    *iter = data.iter;

  return data.found;
}


static GtkTreePath *tree_gui_get_path(tree_gui_t *gui, unsigned long key)
{
  GtkTreePath *path = NULL;
  GtkTreeIter iter;

  if ( tree_gui_get_iter(gui, key, &iter) )
    path = gtk_tree_model_get_path(GTK_TREE_MODEL(gui->tree_model), &iter);

  return path;
}


static tree_object_t *tree_gui_get_active(tree_gui_t *gui, GtkTreeIter *iter)
{
  tree_object_t *object = NULL;

  if ( gui->selected.object ) {
    object = gui->selected.object;
    if ( iter != NULL )
      *iter = gui->selected.iter;
  }
  else if ( gui->current.object ) {
    object = gui->current.object;
    if ( iter != NULL )
      *iter = gui->current.iter;
  }

  return object;
}


/*===========================================*/
/* Menus and Buttons sensitivity             */
/*===========================================*/

static int tree_gui_debug(tree_gui_t *gui);


static void tree_gui_sensitivity_skip(tree_gui_t *gui, int enable)
{
  gtk_widget_set_sensitive(gui->force_skip[0], enable);
  gtk_widget_set_sensitive(gui->force_skip[1], enable);
}


static void tree_gui_sensitivity_brk_skip(tree_gui_t *gui, int enable)
{
  gtk_widget_set_sensitive(gui->clear_brkpt_skip[0], enable);
  gtk_widget_set_sensitive(gui->clear_brkpt_skip[1], enable);
}


static void tree_gui_sensitivity_exec(tree_gui_t *gui)
{
  int state = gui->exec_state;
  int waiting = (state == stWAIT) || (state == stINPUT);
  int debug = 0;
  int reset = 0;
  int go = 0;
  int step = 0;
  int abort = 0;

  /* Disable the [Reset] button if PERL is working */
  if ( (state >= 0) && (state != stSPAWN) && (state != stRUN) )
    reset = 1;

  /* Enable debug option if not running */
  if ( (state < 0) || (state == stRESET) || (state == stFINISHED) ) {
    debug = 1;
  }

  /* Disable the [Go] button if execution is in progress */
  if ( (state == stRESET) || (state == stWAIT) ) {
    go = 1;

    /* Disable the [Step] button if PERL debugger enabled */
    step = ! tree_gui_debug(gui);
  }

  /* Enable the [Abort] button if execution is in progress */
  if ( (state == stSPAWN) || (state == stRUN) || (state == stINPUT) ) {
    abort= 1;
  }

  /* Perform sensitivity update */
  gtk_widget_set_sensitive(gui->debug, debug);
  gtk_widget_set_sensitive(gui->reset[0], reset);
  gtk_widget_set_sensitive(gui->reset[1], reset);
  gtk_widget_set_sensitive(gui->go[0], go);
  gtk_widget_set_sensitive(gui->go[1], go);
  gtk_widget_set_sensitive(gui->step[0], step);
  gtk_widget_set_sensitive(gui->step[1], step);
  gtk_widget_set_sensitive(gui->abort[0], abort);
  gtk_widget_set_sensitive(gui->abort[1], abort);

  /* Update input area sensitivity */
  input_gui_sensitivity(gui->ig, waiting);

  /* Update Manual User Interface sensitivity */
  system_gui_manual_enable(gui->sg, waiting);
}


static void tree_gui_sensitivity_clear(tree_gui_t *gui)
{
  gtk_widget_set_sensitive(gui->reset[0], 0);
  gtk_widget_set_sensitive(gui->reset[1], 0);
  gtk_widget_set_sensitive(gui->go[0], 0);
  gtk_widget_set_sensitive(gui->go[1], 0);
  gtk_widget_set_sensitive(gui->step[0], 0);
  gtk_widget_set_sensitive(gui->step[1], 0);
  gtk_widget_set_sensitive(gui->abort[0], 0);
  gtk_widget_set_sensitive(gui->abort[1], 0);
  gtk_widget_set_sensitive(gui->breakpoint[0], 0);
  gtk_widget_set_sensitive(gui->breakpoint[1], 0);
  gtk_widget_set_sensitive(gui->spot_tree, 0);
  gtk_widget_set_sensitive(gui->spot_log, 0);
  tree_gui_sensitivity_skip(gui, 0);
  tree_gui_sensitivity_brk_skip(gui, 0);

  system_gui_enable(gui->sg, 0);
}


/*===========================================*/
/* Output dump                               */
/*===========================================*/

static void tree_gui_show(tree_gui_t *gui)
{
  GtkTreeIter iter;
  tree_object_t *object = tree_gui_get_active(gui, &iter);
  int is_valid = (object != NULL);
  int is_case = is_valid && (object->type == TYPE_CASE);

  /* Show output data */
  if ( ! is_valid ) {
    is_valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(gui->tree_model), &iter);
  }

  if ( is_valid ) {
    GtkTreeIter parent;
    char *name = NULL;
    int is_root;

    gtk_tree_model_get(GTK_TREE_MODEL(gui->tree_model), &iter,
		       TREE_GUI_COL_NAME, &name,
		       -1);

    is_root = ! gtk_tree_model_iter_parent(GTK_TREE_MODEL(gui->tree_model), &parent, &iter);

    if ( name != NULL ) {
      output_gui_show(gui->og, name, is_root);
      free(name);
    }
  }

  /* Show validation info */
  validate_gui_select(gui->vg, object);

  /* Change breakpoint button sensitivity */
  gtk_widget_set_sensitive(gui->breakpoint[0], is_case);
  gtk_widget_set_sensitive(gui->breakpoint[1], is_case);

  /* Change spot-in-tree button sensitivity */
  gtk_widget_set_sensitive(gui->spot_tree, 1);

  /* Enable spot-in-log button for a case node */
  gtk_widget_set_sensitive(gui->spot_log, is_case);
}


static void tree_gui_show_current(tree_gui_t *gui)
{
  if ( gui->selected.object == NULL )
    tree_gui_show(gui);
}


void tree_gui_output(tree_gui_t *gui, unsigned long key, int channel, char *str)
{
  tree_object_t *object;

  if ( gui == NULL )
    return;

  object = tree_gui_get_active(gui, NULL);
  if ( (object != NULL) && (object->key == key) ) {
    output_gui_feed(gui->og, channel, str);

    /* Scroll down output display if execution is waiting for input */
    if ( gui->exec_state == stINPUT ) {
      output_gui_scrolldown(gui->og);
    }
  }
}


/*===========================================*/
/* Node selection and indexing               */
/*===========================================*/

static void tree_gui_paint_iter(tree_gui_t *gui, GtkTreeIter *iter)
{
  GdkColor *color;
  PangoStyle style = PANGO_STYLE_NORMAL;
  tree_object_t *object;
  int skip;
  int is_current;

  gtk_tree_model_get(GTK_TREE_MODEL(gui->tree_model), iter,
		     TREE_GUI_COL_OBJECT, &object,
		     TREE_GUI_COL_SKIP, &skip,
		     -1);

  is_current = (object == gui->current.object);
  color = is_current ? &color_yellow : &color_white;

  if ( skip ) {
    if ( ! is_current )
      color = &color_gray;
    style = PANGO_STYLE_ITALIC;
  }

  gtk_tree_store_set(gui->tree_model, iter,
		     TREE_GUI_COL_BACKGROUND, color,
		     TREE_GUI_COL_STYLE, style,
		     -1);
}


void tree_gui_set_current(tree_gui_t *gui, unsigned long key)
{
  if ( gui == NULL )
    return;

  /* Unpaint previous row */
  if ( gui->current.object ) {
    gui->current.object = NULL;
    tree_gui_paint_iter(gui, &(gui->current.iter));
  }

  if ( (key != 0) && tree_gui_get_iter(gui, key, &(gui->current.iter)) ) {
    GtkTreePath *path;

    gtk_tree_model_get(GTK_TREE_MODEL(gui->tree_model), &(gui->current.iter),
		       TREE_GUI_COL_OBJECT, &(gui->current.object),
		       -1);

    /* Expand tree to current path */
    path = gtk_tree_model_get_path(GTK_TREE_MODEL(gui->tree_model), &(gui->current.iter));
    gtk_tree_view_expand_to_path(gui->tree_view, path);
    gtk_tree_path_free(path);

    /* Paint row */
    tree_gui_paint_iter(gui, &(gui->current.iter));
  }
  //fprintf(stderr, "-- CURRENT = %lu\n", key);

  /* Update output dump display */
  tree_gui_show_current(gui);
}


void tree_gui_set_report(tree_gui_t *gui, char *log_name, char *report_directory)
{
  if ( gui == NULL )
    return;

  report_gui_set_name(gui->rg, log_name, report_directory);
}


/*===========================================*/
/* PERL Debug option                         */
/*===========================================*/

static int tree_gui_debug(tree_gui_t *gui)
{
  return gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(gui->debug));
}


static void tree_gui_debug_toggled(tree_gui_t *gui)
{
  tree_run_set_debug(gui->actions_arg, gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(gui->debug)));
  tree_gui_sensitivity_exec(gui);
}


/*===========================================*/
/* Verdicts drawings                         */
/*===========================================*/

void tree_gui_verdict(tree_gui_t *gui, unsigned long key, tree_result_t *result)
{
  GtkTreeIter iter;
  char *id = "";
  char *color = "black";

  if ( gui == NULL )
    return;

  if ( (result->criticity > CRITICITY_NONE) && (result->verdict == VERDICT_FAILED) ) {
    id = criticity_id(result->criticity);
    color = criticity_color(result->criticity);
  }

  if ( tree_gui_get_iter(gui, key, &iter) ) {
    gtk_tree_store_set(gui->tree_model, &iter,
		       TREE_GUI_COL_VERDICT_PIXBUF, report_gui_verdict_pixbuf(result->verdict),
		       TREE_GUI_COL_CRITICITY, id,
		       TREE_GUI_COL_CRITICITY_COLOR, color,
		       -1);
  }

  if ( result->verdict != VERDICT_UNEXECUTED ) {
    report_gui_executed(gui->rg, key, result);
  }
}


/*===========================================*/
/* Breakpoints management                    */
/*===========================================*/

static void tree_gui_breakpoint_iter(tree_gui_t *gui, GtkTreeIter *iter, int state)
{
  GdkPixbuf *pixbuf = state ? pixbuf_breakpoint : pixbuf_blank;
  gtk_tree_store_set(gui->tree_model, iter,
		     TREE_GUI_COL_BRKPT_PIXBUF, pixbuf,
		     TREE_GUI_COL_BRKPT, state,
		     -1);
}


static gboolean tree_gui_breakpoint_clear_iter(GtkTreeModel *model,
					       GtkTreePath *path, GtkTreeIter *iter,
					       tree_gui_t *gui)
{
  unsigned long key;

  /* Show BrkPt state */
  tree_gui_breakpoint_iter(gui, iter, 0);

  /* Update BrkPt state */
  gtk_tree_model_get(GTK_TREE_MODEL(gui->tree_model), iter,
		     TREE_GUI_COL_KEY, &key,
		     -1);
  tree_run_set_brkpt(gui->actions_arg, key, 0);

  return FALSE;
}

static void tree_gui_breakpoint_clear_clicked(tree_gui_t *gui)
{
  gtk_tree_model_foreach(GTK_TREE_MODEL(gui->tree_model),
			 (GtkTreeModelForeachFunc) tree_gui_breakpoint_clear_iter, gui);
}


static void tree_gui_breakpoint_clicked(tree_gui_t *gui)
{
  GtkTreeIter iter;
  tree_object_t *object;

  object = tree_gui_get_active(gui, &iter);

  if ( object ) {
    int state;

    /* Get current BrkPt state */
    gtk_tree_model_get(GTK_TREE_MODEL(gui->tree_model), &iter,
		       TREE_GUI_COL_BRKPT, &state,
		       -1);

    /* Toggle BrkPt state */
    state = state ? 0:1;

    /* Show BrkPt state */
    tree_gui_breakpoint_iter(gui, &iter, state);

    /* Update BrkPt state */
    tree_run_set_brkpt(gui->actions_arg, object->key, state);
  }
}


/*===========================================*/
/* ForceSkip management                      */
/*===========================================*/

static void tree_gui_skip_iter(tree_gui_t *gui, GtkTreeIter *iter, int state)
{
  GtkTreeIter child;

  /* Set skip flag */
  gtk_tree_store_set(gui->tree_model, iter,
		     TREE_GUI_COL_SKIP, state,
		     -1);

  /* Set node background accordingly */
  tree_gui_paint_iter(gui, iter);

  /* Propagate ForceSkip state to children */
  if ( gtk_tree_model_iter_children(GTK_TREE_MODEL(gui->tree_model), &child, iter) ) {
    do {
      unsigned long key;

      gtk_tree_model_get(GTK_TREE_MODEL(gui->tree_model), &child,
			 TREE_GUI_COL_KEY, &key,
			 -1);

      tree_run_set_skip(gui->actions_arg, key, state);

      tree_gui_skip_iter(gui, &child, state);
    } while ( gtk_tree_model_iter_next(GTK_TREE_MODEL(gui->tree_model), &child) );
  }
}


static void tree_gui_skip_clicked(tree_gui_t *gui)
{
  GtkTreeIter iter;
  tree_object_t *object;

  object = tree_gui_get_active(gui, &iter);

  if ( object ) {
    /* Toggle ForceSkip flag */
    int state = tree_run_set_skip(gui->actions_arg, object->key, -1);

    /* Set and propagate skip flag */
    tree_gui_skip_iter(gui, &iter, state);
  }
}


/*===========================================*/
/* BrkPt + ForceSkip clear                   */
/*===========================================*/

static gboolean tree_gui_clear_iter(GtkTreeModel *model,
				    GtkTreePath *path, GtkTreeIter *iter,
				    tree_gui_t *gui)
{
  unsigned long key;

  /* Get node key */
  gtk_tree_model_get(GTK_TREE_MODEL(gui->tree_model), iter,
		     TREE_GUI_COL_KEY, &key,
		     -1);

  /* Clear BrkPt flag */
  tree_gui_breakpoint_iter(gui, iter, 0);
  tree_run_set_brkpt(gui->actions_arg, key, 0);

  /* Clear Skip flag */
  tree_gui_skip_iter(gui, iter, 0);
  tree_run_set_skip(gui->actions_arg, key, 0);

  return FALSE;
}

static void tree_gui_clear_clicked(tree_gui_t *gui)
{
  gtk_tree_model_foreach(GTK_TREE_MODEL(gui->tree_model),
			 (GtkTreeModelForeachFunc) tree_gui_clear_iter, gui);
}


/*===========================================*/
/* Tree view selection                       */
/*===========================================*/

static gboolean tree_gui_select(GtkTreeSelection *selection, GtkTreeModel *model,
				GtkTreePath *path, gboolean path_currently_selected,
				tree_gui_t *gui)
{
  int new_state = path_currently_selected ? 0:1;

  /* Change force-skip button sensitivity */
  tree_gui_sensitivity_skip(gui, new_state);

  /* Get selected node name */
  gui->selected.object = NULL;
  if ( new_state ) {
    if ( gtk_tree_model_get_iter(model, &(gui->selected.iter), path) )
      gtk_tree_model_get(model, &(gui->selected.iter),
			 TREE_GUI_COL_OBJECT, &(gui->selected.object),
			 -1);
    else
      return FALSE;
  }

  /* Show output dump */
  tree_gui_show(gui);

  return TRUE;
}


static void tree_gui_unselect(tree_gui_t *gui)
{
  /* Clear node selection */
  gtk_tree_selection_unselect_all(gui->tree_selection);
  gui->selected.object = NULL;
}


/*===========================================*/
/* Spot node in Tree View                    */
/*===========================================*/

static void tree_gui_spot_path(tree_gui_t *gui, GtkTreePath *path)
{
  /* Expand the node to spot to */
  gtk_tree_view_expand_to_path(gui->tree_view, path);

  /* Make the node viewable within the tree window */
  gtk_tree_view_scroll_to_cell(gui->tree_view, path, NULL, TRUE, 0.5, 0);

  /* Select focused row */
  gtk_tree_selection_select_path(gui->tree_selection, path);
}


void tree_gui_spot(tree_gui_t *gui, unsigned long key)
{
  GtkTreePath *path = NULL;

  if ( gui == NULL )
    return;

  /* Spot requested node */
  if ( key == 0 ) {
    path = gtk_tree_path_new_first();
  }
  else {
    path = tree_gui_get_path(gui, key);
  }

  if ( path != NULL ) {
    tree_gui_spot_path(gui, path);
    gtk_tree_path_free(path);
  }

  /* If spotted node is the current exec pointer, unselect it */
  if ( (gui->current.object != NULL) && (gui->current.object->key == key) ) {
    tree_gui_unselect(gui);
  }
}


static void tree_gui_spot_tree_clicked(tree_gui_t *gui)
{
  GtkTreeIter iter;

  if ( tree_gui_get_active(gui, &iter) ) {
    GtkTreePath *path = gtk_tree_model_get_path(GTK_TREE_MODEL(gui->tree_model), &iter);
    tree_gui_spot_path(gui, path);
    gtk_tree_path_free(path);
  }
}


/*===========================================*/
/* Spot node in Log View                     */
/*===========================================*/

static void tree_gui_spot_log_iter(tree_gui_t *gui, GtkTreeIter *iter)
{
  char *name;

  gtk_tree_model_get(GTK_TREE_MODEL(gui->tree_model), iter,
		     TREE_GUI_COL_NAME, &name,
		     -1);

  report_gui_log_show(gui->rg, name);

  free(name);
}


static void tree_gui_spot_log_clicked(tree_gui_t *gui)
{
  GtkTreeIter iter;

  if ( tree_gui_get_active(gui, &iter) ) {
    tree_gui_spot_log_iter(gui, &iter);
  }
}


/*===========================================*/
/* Tree view row activation                  */
/*===========================================*/

static void tree_gui_row_activated(GtkTreeView *treeview,
				   GtkTreePath *path,
				   GtkTreeViewColumn *column,
				   tree_gui_t *gui)
{
  GtkTreeIter iter;

  if ( gtk_tree_model_get_iter(GTK_TREE_MODEL(gui->tree_model), &iter, path) ) {
    tree_gui_spot_log_iter(gui, &iter);
  }
}


/*===========================================*/
/* Execution start                           */
/*===========================================*/

static void tree_gui_jump(tree_gui_t *gui)
{
  tree_object_t *object;

  /* If in WAIT state, jump to selected node */
  if ( gui->exec_state == stWAIT ) {
    if ( gui->selected.object != NULL ) {
      tree_run_jump(gui->actions_arg, gui->selected.object->key);
    }
  }

  /* Clear current node if focus is on it */
  object = tree_gui_get_active(gui, NULL);
  if ( (object != NULL) && (object->key == gui->current.object->key) ) {
    output_gui_clear(gui->og);
  }
}


static void tree_gui_start_clicked(tree_gui_t *gui)
{
  tree_gui_jump(gui);
  tree_run_start(gui->actions_arg, 0);
}


static void tree_gui_step_clicked(tree_gui_t *gui)
{
  tree_gui_jump(gui);
  tree_run_start(gui->actions_arg, 1);
}


/*===========================================*/
/* Input actions handling                    */
/*===========================================*/

static void tree_gui_input(tree_gui_t *gui, int verdict, char *text)
{
  tree_gui_jump(gui);
  tree_run_input(gui->actions_arg, verdict, text);
}


/*===========================================*/
/* Tree view construction                    */
/*===========================================*/

static void tree_gui_build_branch(tree_gui_t *gui, GtkTreeIter *parent_iter, tree_object_t *object)
{
  GtkTreeIter iter;
  GdkPixbuf *pixbuf;
  int i;

  /* Setup node pix */
  switch ( object->type ) {
  case TYPE_CASE:
    pixbuf = pixbuf_case;
    break;

  case TYPE_SEQ:
    if ( parent_iter == NULL )
      pixbuf = pixbuf_tree;
    else
      pixbuf = pixbuf_seq;
    break;

  default :
    pixbuf = pixbuf_blank;
    break;
  }

  /* Create new row */
  gtk_tree_store_append(gui->tree_model, &iter, parent_iter);

  gtk_tree_store_set(gui->tree_model, &iter,
		     TREE_GUI_COL_TYPE_PIXBUF, pixbuf,
		     TREE_GUI_COL_NAME, object->parent_item->name,
		     TREE_GUI_COL_SKIP, object->flags & FLAG_FORCE_SKIP,
		     TREE_GUI_COL_KEY, object->key,
		     TREE_GUI_COL_OBJECT, object,
		     -1);
  //fprintf(stderr, "-- BUILD %lu => %s\n", object->key, object->parent_item->name);

  /* Show Abort-If-Failed flag status */
  if ( object->flags & FLAG_ABORT_IF_FAILED ) {
    gtk_tree_store_set(gui->tree_model, &iter,
		       TREE_GUI_COL_FLAGS_PIXBUF, pixbuf_abort,
		       -1);
  }
  else if ( object->flags & FLAG_BREAK_IF_FAILED ) {
    gtk_tree_store_set(gui->tree_model, &iter,
		       TREE_GUI_COL_FLAGS_PIXBUF, pixbuf_break,
		       -1);
  }

  switch ( object->type ) {
  case TYPE_CASE:
    /* Init object data */
    tree_gui_breakpoint_iter(gui, &iter, object->d.Case->breakpoint);
    break;

  case TYPE_SEQ:
    for (i = 0; i < object->d.Seq->nnodes; i++) {
      tree_item_t *item = object->d.Seq->nodes[i];
      if ( item != NULL )
	tree_gui_build_branch(gui, &iter, item->object);
    }
    break;

  default :
    break;
  }

  /* Refresh background painting */
  tree_gui_paint_iter(gui, &iter);
}


void tree_gui_load(tree_gui_t *gui, tree_t *tree, output_t *out)
{
  tree_item_t *head = tree->head;
  char title[strlen(head->name)+24];
  GtkTreePath *path;

  if ( gui == NULL )
    return;

  /* Clear node selection/index */
  gui->current.object = NULL;
  gui->selected.object = NULL;
  gui->exec_state = stRESET;

  /* Build tree structure */
  tree_gui_build_branch(gui, NULL, head->object);

  /* Expand tree root */
  path = gtk_tree_path_new_first();
  gtk_tree_view_expand_to_path(gui->tree_view, path);
  gtk_tree_path_free(path);

  /* Clear selection */
  tree_gui_unselect(gui);

  /* Update window title */
  snprintf(title, sizeof(title), "TestFarm Runner - %s", head->name);
  gtk_window_set_title(GTK_WINDOW(gui->window), title);

  /* Update button sensitivity */
  tree_gui_sensitivity_brk_skip(gui, 1);

  /* Init output and reporting stuffs */
  output_gui_set(gui->og, out);
  report_gui_load(gui->rg, tree, out);

  /* Activate Service start/stop buttons */
  if ( tree->system != NULL ) {
    system_gui_set(gui->sg, tree->system);
    system_gui_enable(gui->sg, 1);
  }
}


void tree_gui_unload(tree_gui_t *gui)
{
  if ( gui == NULL )
    return;

  /* Clear output display */
  output_gui_clear(gui->og);
  output_gui_set(gui->og, NULL);

  /* Clear node selection/index */
  gui->current.object = NULL;
  gui->selected.object = NULL;
  gui->exec_state = -1;

  /* Clear tree view */
  gtk_tree_store_clear(gui->tree_model);

  /* Clear buttons sensitivity */
  tree_gui_sensitivity_clear(gui);

  /* Clear report stuffs */
  report_gui_unload(gui->rg);
}


static void tree_gui_init_treeview(tree_gui_t *gui)
{
  //GtkTooltips *tooltips;
  GtkTreeViewColumn *column;
  GtkCellRenderer *renderer;

  /* Get tooltips engine */
  //tooltips = gtk_object_get_data(GTK_OBJECT(gui->window), "tooltips");

  /* Create test tree */
  gui->tree_model = gtk_tree_store_new(TREE_GUI_NCOLS,
				       /* TREE_GUI_COL_TYPE_PIXBUF     */ GDK_TYPE_PIXBUF,
				       /* TREE_GUI_COL_NAME            */ G_TYPE_STRING,
				       /* TREE_GUI_COL_BRKPT_PIXBUF    */ GDK_TYPE_PIXBUF,
				       /* TREE_GUI_COL_BRKPT           */ G_TYPE_INT,
				       /* TREE_GUI_COL_FLAGS_PIXBUF    */ GDK_TYPE_PIXBUF,
				       /* TREE_GUI_COL_SKIP            */ G_TYPE_INT,
				       /* TREE_GUI_COL_VERDICT_PIXBUF  */ GDK_TYPE_PIXBUF,
				       /* TREE_GUI_COL_CRITICITY       */ G_TYPE_STRING,
				       /* TREE_GUI_COL_CRITICITY_COLOR */ G_TYPE_STRING,
				       /* TREE_GUI_COL_BACKGROUND      */ GDK_TYPE_COLOR,
				       /* TREE_GUI_COL_STYLE           */ PANGO_TYPE_STYLE,
				       /* TREE_GUI_COL_KEY             */ G_TYPE_ULONG,
				       /* TREE_GUI_COL_OBJECT          */ G_TYPE_POINTER);

  gui->tree_view = GTK_TREE_VIEW(lookup_widget(gui->window, "tree_view"));
  gtk_tree_view_set_model(gui->tree_view, GTK_TREE_MODEL(gui->tree_model));

  /* Setup tree selection handler */
  gui->tree_selection = gtk_tree_view_get_selection(gui->tree_view);
  gtk_tree_selection_set_mode(gui->tree_selection, GTK_SELECTION_SINGLE);
  gtk_tree_selection_set_select_function(gui->tree_selection,
					 (GtkTreeSelectionFunc) tree_gui_select, gui,
					 NULL);

  /* Set up row activation handler */
  gtk_signal_connect(GTK_OBJECT(gui->tree_view), "row-activated",
		     GTK_SIGNAL_FUNC(tree_gui_row_activated), gui);

  /* Column #0: breakpoint flag */
  renderer = gtk_cell_renderer_pixbuf_new();
  gtk_tree_view_insert_column_with_attributes(gui->tree_view, -1, "Brk", renderer,
					      "pixbuf", TREE_GUI_COL_BRKPT_PIXBUF,
					      "cell-background-gdk", TREE_GUI_COL_BACKGROUND,
					      NULL);

  /* Setup breakpoints clear button */
  column = gtk_tree_view_get_column(gui->tree_view, 0);
  gtk_tree_view_column_set_resizable(column, FALSE);
  gtk_tree_view_column_set_clickable(column, TRUE);
  // TODO: gtk_tooltips_set_tip (tooltips, w, "Clear all breakpoints", NULL);
  gtk_signal_connect_object(GTK_OBJECT(column), "clicked",
			    GTK_SIGNAL_FUNC(tree_gui_breakpoint_clear_clicked), gui);

  /* Column #1: Node pix and name */
  column = gtk_tree_view_column_new();
  gtk_tree_view_column_set_title(column, "Test Tree");
  gtk_tree_view_column_set_expand(column, TRUE);

  renderer = gtk_cell_renderer_pixbuf_new();
  gtk_tree_view_column_pack_start(column, renderer, FALSE);
  gtk_tree_view_column_add_attribute(column, renderer, "pixbuf", TREE_GUI_COL_TYPE_PIXBUF);
  gtk_tree_view_column_add_attribute(column, renderer, "cell-background-gdk", TREE_GUI_COL_BACKGROUND);

  renderer = gtk_cell_renderer_text_new();
  //gtk_object_set(GTK_OBJECT(renderer), "family", "Monospace", NULL);
  gtk_tree_view_column_pack_start(column, renderer, TRUE);
  gtk_tree_view_column_add_attribute(column, renderer, "text", TREE_GUI_COL_NAME);
  gtk_tree_view_column_add_attribute(column, renderer, "cell-background-gdk", TREE_GUI_COL_BACKGROUND);
  gtk_tree_view_column_add_attribute(column, renderer, "style", TREE_GUI_COL_STYLE);

  gtk_tree_view_column_set_resizable(column, TRUE);
  gtk_tree_view_append_column(gui->tree_view, column);
  gtk_tree_view_set_expander_column(gui->tree_view, column);

  /* Setup selection clear button */
  gtk_tree_view_column_set_clickable(column, TRUE);
  // TODO: gtk_tooltips_set_tip (tooltips, w, "Clear selection", NULL);
  gtk_signal_connect_object(GTK_OBJECT(column), "clicked",
                            GTK_SIGNAL_FUNC(tree_gui_unselect), gui);

  /* Column #2: runtime flag */
  renderer = gtk_cell_renderer_pixbuf_new();
  gtk_tree_view_insert_column_with_attributes(gui->tree_view, -1, "Flg", renderer,
					      "pixbuf", TREE_GUI_COL_FLAGS_PIXBUF,
					      "cell-background-gdk", TREE_GUI_COL_BACKGROUND,
					      NULL);

  /* Column #3: verdict and criticity */
  column = gtk_tree_view_column_new();
  gtk_tree_view_column_set_title(column, "Verdict");

  renderer = gtk_cell_renderer_pixbuf_new();
  gtk_tree_view_column_pack_start(column, renderer, FALSE);
  gtk_tree_view_column_add_attribute(column, renderer, "pixbuf", TREE_GUI_COL_VERDICT_PIXBUF);
  gtk_tree_view_column_add_attribute(column, renderer, "cell-background-gdk", TREE_GUI_COL_BACKGROUND);

  renderer = gtk_cell_renderer_text_new();
  gtk_object_set(GTK_OBJECT(renderer), "scale", 0.6, NULL);
  gtk_tree_view_column_pack_start(column, renderer, TRUE);
  gtk_tree_view_column_add_attribute(column, renderer, "text", TREE_GUI_COL_CRITICITY);
  gtk_tree_view_column_add_attribute(column, renderer, "foreground", TREE_GUI_COL_CRITICITY_COLOR);
  gtk_tree_view_column_add_attribute(column, renderer, "cell-background-gdk", TREE_GUI_COL_BACKGROUND);

  gtk_tree_view_column_set_resizable(column, FALSE);
  gtk_tree_view_column_set_sizing(column, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
  gtk_tree_view_append_column(gui->tree_view, column);
}


/*===========================================*/
/* Action handlers setup                     */
/*===========================================*/

static void tree_gui_init_actions(tree_gui_t *gui, void *arg)
{
  GtkWidget *window = gui->window;

  gui->actions_arg = arg;

  /* Setup execution control buttons */
  gui->reset[0] = lookup_widget(window, "tool_reset");
  gui->reset[1] = lookup_widget(window, "execution_reset");
  gtk_signal_connect_object(GTK_OBJECT(gui->reset[0]), "clicked",
                            GTK_SIGNAL_FUNC(tree_run_reset), arg);
  gtk_signal_connect_object(GTK_OBJECT(gui->reset[1]), "activate",
                            GTK_SIGNAL_FUNC(tree_run_reset), arg);

  gui->go[0] = lookup_widget(window, "tool_go");
  gui->go[1] = lookup_widget(window, "execution_go");
  gtk_signal_connect_object(GTK_OBJECT(gui->go[0]), "clicked",
                            GTK_SIGNAL_FUNC(tree_gui_start_clicked), gui);
  gtk_signal_connect_object(GTK_OBJECT(gui->go[1]), "activate",
                            GTK_SIGNAL_FUNC(tree_gui_start_clicked), gui);

  gui->step[0] = lookup_widget(window, "tool_step");
  gui->step[1] = lookup_widget(window, "execution_step");
  gtk_signal_connect_object(GTK_OBJECT(gui->step[0]), "clicked",
                            GTK_SIGNAL_FUNC(tree_gui_step_clicked), gui);
  gtk_signal_connect_object(GTK_OBJECT(gui->step[1]), "activate",
                            GTK_SIGNAL_FUNC(tree_gui_step_clicked), gui);

  gui->abort[0] = lookup_widget(window, "tool_abort");
  gui->abort[1] = lookup_widget(window, "execution_abort");
  gtk_signal_connect_object(GTK_OBJECT(gui->abort[0]), "clicked",
                            GTK_SIGNAL_FUNC(tree_run_abort), arg);
  gtk_signal_connect_object(GTK_OBJECT(gui->abort[1]), "activate",
                            GTK_SIGNAL_FUNC(tree_run_abort), arg);

  /* Setup tree flags buttons */
  gui->breakpoint[0] = lookup_widget(window, "tool_breakpoint");
  gui->breakpoint[1] = lookup_widget(window, "debug_breakpoint");
  gtk_signal_connect_object(GTK_OBJECT(gui->breakpoint[0]), "clicked",
                            GTK_SIGNAL_FUNC(tree_gui_breakpoint_clicked), gui);
  gtk_signal_connect_object(GTK_OBJECT(gui->breakpoint[1]), "activate",
                            GTK_SIGNAL_FUNC(tree_gui_breakpoint_clicked), gui);

  gui->force_skip[0] = lookup_widget(window, "tool_skip");
  gui->force_skip[1] = lookup_widget(window, "debug_skip");
  gtk_signal_connect_object(GTK_OBJECT(gui->force_skip[0]), "clicked",
                            GTK_SIGNAL_FUNC(tree_gui_skip_clicked), gui);
  gtk_signal_connect_object(GTK_OBJECT(gui->force_skip[1]), "activate",
                            GTK_SIGNAL_FUNC(tree_gui_skip_clicked), gui);

  gui->clear_brkpt_skip[0] = lookup_widget(window, "tool_clear");
  gui->clear_brkpt_skip[1] = lookup_widget(window, "debug_clear");
  gtk_signal_connect_object(GTK_OBJECT(gui->clear_brkpt_skip[0]), "clicked",
                            GTK_SIGNAL_FUNC(tree_gui_clear_clicked), gui);
  gtk_signal_connect_object(GTK_OBJECT(gui->clear_brkpt_skip[1]), "activate",
                            GTK_SIGNAL_FUNC(tree_gui_clear_clicked), gui);

  /* Setup test case action buttons */
  gui->spot_tree = lookup_widget(window, "case_spot_tree");
  gtk_signal_connect_object(GTK_OBJECT(gui->spot_tree), "clicked",
                            GTK_SIGNAL_FUNC(tree_gui_spot_tree_clicked), gui);

  gui->spot_log = lookup_widget(window, "case_spot_log");
  gtk_signal_connect_object(GTK_OBJECT(gui->spot_log), "clicked",
                            GTK_SIGNAL_FUNC(tree_gui_spot_log_clicked), gui);

  /* Setup PERL debug enable option menu */
  gui->debug = lookup_widget(window, "debug_perl");
  gtk_signal_connect_object(GTK_OBJECT(gui->debug), "toggled",
                            GTK_SIGNAL_FUNC(tree_gui_debug_toggled), gui);


  /* Setup running state icon */
  gui->running = lookup_widget(window, "run_perl_pixmap");
}


tree_gui_t *tree_gui_init(GtkWidget *window, properties_t *prop, void *arg)
{
  tree_gui_t *gui;

  if ( window == NULL )
    return NULL;

  gui = (tree_gui_t *) malloc(sizeof(tree_gui_t));
  gui->window = window;

  gui->exec_state = -1;

  /* Init tree view */
  tree_gui_init_treeview(gui);

  /* Init action handlers */
  tree_gui_init_actions(gui, arg);

  /* Init output area */
  gui->og = output_gui_init(window);

  /* Setup validation tool */
  gui->vg = validate_gui_init(window, prop);

  /* Setup reporting */
  gui->rg = report_gui_init(window, (report_gui_spot_tree_t *) tree_gui_spot, gui);

  /* Init input area */
  gui->ig = input_gui_init(window, (input_gui_done_t *) tree_gui_input, gui);

  /* Setup system control */
  gui->sg = system_gui_init(window);
  system_gui_manual_handler(gui->sg, (system_gui_handler_t *) tree_run_manual_action, arg);

  /* Clear buttons sensitivity */
  tree_gui_sensitivity_clear(gui);

  gtk_widget_show(window);

  return gui;
}


void tree_gui_destroy(tree_gui_t *gui)
{
  if ( gui == NULL )
    return;

  /* Destroy System Management stuffs */
  system_gui_destroy(gui->sg);
  gui->sg = NULL;

  /* Destroy input stuffs */
  input_gui_destroy(gui->ig);
  gui->ig = NULL;

  /* Destroy report stuffs */
  report_gui_destroy(gui->rg);
  gui->rg = NULL;

  /* Destroy script validation stuffs */
  validate_gui_destroy(gui->vg);
  gui->vg = NULL;

  /* Destroy output display stuffs */
  output_gui_destroy(gui->og);
  gui->og = NULL;

  free(gui);
}


/*===========================================*/
/* Execution state events                    */
/*===========================================*/

const char *state_title[] = {
  "Reset",
  "Spawning PERL process",
  "Halted",
  "Running",
  "Waiting for Input",
  "Finished",
};


void tree_gui_set_state(tree_gui_t *gui, int state)
{
  GtkLabel *label;
  char str[80];

  if ( gui == NULL )
    return;

  /* Latch execution state */
  gui->exec_state = state;

  /* Build state string */
  strcpy(str, state_title[state]);
  if ( (state == stRUN) && tree_gui_debug(gui) )
    strcat(str, " with PERL debugger");

  label = GTK_LABEL(lookup_widget(gui->window, "run_label"));
  gtk_label_set_text(label, str);

  /* Update running state icon */
  if ( (state >= 0) && (state != stRESET) && (state != stFINISHED) ) {
    gtk_image_set_from_pixbuf(GTK_IMAGE(gui->running), pixbuf_up);
  }
  else {
    gtk_image_set_from_pixbuf(GTK_IMAGE(gui->running), pixbuf_down);
  }

  /* Clear output and reporting stuffs if execution is resetted */
  if ( state == stRESET ) {
    report_gui_reset(gui->rg);
    output_gui_clear(gui->og);
  }

  /* Spot current node if execution waits on it */
  if ( (state == stWAIT) || (state == stINPUT) ) {
    if ( gui->current.object != NULL )
      tree_gui_spot(gui, gui->current.object->key);
  }

  /* If execution is waiting for input,
     Scroll down output display and enable input area */
  if ( state == stINPUT ) {
    output_gui_scrolldown(gui->og);
    input_gui_wakeup(gui->ig);
  }

  /* Update test case display when finished */
  if ( state == stFINISHED ) {
    tree_gui_show_current(gui);
  }

  /* Update execution buttons/menus sensitivity */
  tree_gui_sensitivity_exec(gui);
}


/*===========================================*/
/* Main window close function                */
/*===========================================*/

void tree_gui_quit(tree_gui_t *gui)
{
  if ( gui == NULL )
    return;

  gtk_widget_destroy(gui->window);
}
