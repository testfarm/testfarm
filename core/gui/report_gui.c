/****************************************************************************/
/* TestFarm                                                                 */
/* Graphical User Interface : Test Report Management                        */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 18-MAY-2000                                                    */
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
#include <malloc.h>
#include <errno.h>
#include <fcntl.h>
#include <gtk/gtk.h>

#include "install.h"
#include "codegen.h"

#include "output.h"
#include "interface.h"
#include "support.h"
#include "child.h"
#include "xpm_gui.h"
#include "status.h"
#include "error_gui.h"
#include "report_progress.h"
#include "report_gui.h"


/*================================================================*/
/* Some useful stuffs                                             */
/*================================================================*/

GdkPixbuf *report_gui_verdict_pixbuf(int verdict)
{
  GdkPixbuf *ret = pixbuf_blank;

  switch ( verdict ) {
  case VERDICT_PASSED:
    ret = pixbuf_passed;
    break;
  case VERDICT_FAILED:
    ret = pixbuf_failed;
    break;
  case VERDICT_INCONCLUSIVE:
    ret = pixbuf_inconclusive;
    break;
  case VERDICT_SKIP:
    ret = pixbuf_skip;
    break;
  default :
    break;
  }

  return ret;
}


static char *strnum(int num)
{
  static char buf[20];
  sprintf(buf, "%d", num);
  return buf;
}


/*================================================================*/
/* Subprocess window management                                   */
/*================================================================*/

static void report_gui_proc_terminated(int status, report_gui_proc_t *proc)
{
  if ( proc->stdin >= 0 )
    close(proc->stdin);
  proc->stdin = -1;

  proc->child = NULL;
}


static void report_gui_proc_command(report_gui_proc_t *proc, char cmd, char *arg)
{
  char lf = '\n';

  if ( proc->stdin < 0 )
    return;

  write(proc->stdin, &cmd, 1);
  if ( arg != NULL )
    write(proc->stdin, arg, strlen(arg));
  write(proc->stdin, &lf, 1);
}


static void report_gui_proc_spawn(report_gui_proc_t *proc, char *argv[])
{
  int stdin_pipe[2];

  /* Ensure everything is closed */
  report_gui_proc_terminated(0, proc);

  /* Create command pipe */
  if ( pipe(stdin_pipe) ) {
    fprintf(stderr, "Unable to create pipe for \"%s\": %s", argv[0], strerror(errno));
    return;
  }

  fcntl(stdin_pipe[0], F_SETFD, 0);          /* Disable close-on-exec mode on read endpoint */
  fcntl(stdin_pipe[1], F_SETFD, FD_CLOEXEC); /* Enable close-on-exec mode on write endpoint */

  proc->child = child_spawn(argv, stdin_pipe[0], -1 /* stdout */, -1 /* stderr */,
			    (child_handler_t *) report_gui_proc_terminated, proc);

  /* Close now useless pipe endpoint */
  close(stdin_pipe[0]);

  if ( proc->child == NULL ) {
    close(stdin_pipe[1]);
    status_mesg("Cannot execute process \"%s\"", argv[0]);
  }
  else {
    proc->stdin = stdin_pipe[1];
  }
}


static void report_gui_proc_clear(report_gui_proc_t *proc)
{
  proc->child = NULL;
  proc->stdin = -1;
}


static void report_gui_proc_terminate(report_gui_proc_t *proc)
{
  if ( proc->stdin >= 0 )
    close(proc->stdin);
  proc->stdin = -1;

  if ( proc->child != NULL )
    child_terminate(proc->child);
  proc->child = NULL;
}


/*================================================================*/
/* Log viewer                                                     */
/*================================================================*/

void report_gui_log_show(report_gui_t *rg, char *nodename)
{
  char *argv[] = { INSTALL_LOGVIEW, "-t", rg->tree->head->name, "-c", rg->log_name, NULL};
  report_gui_proc_t *proc = &(rg->log_viewer);

  if ( proc->child == NULL ) {
    report_gui_proc_spawn(proc, argv);
  }

  report_gui_proc_command(proc, 'R', nodename);
}


static void report_gui_log_clicked(GtkWidget *widget, report_gui_t *rg)
{
  report_gui_log_show(rg, NULL);
}


/*================================================================*/
/* Report configuration                                           */
/*================================================================*/

static void report_gui_conf_clicked(GtkWidget *widget, report_gui_t *rg)
{
  char *argv[] = { "testfarm-config", NULL};
  report_gui_proc_t *proc = &(rg->report_conf);

  if ( proc->child == NULL ) {
    report_gui_proc_spawn(proc, argv);
  }

  report_gui_proc_command(proc, 'R', NULL);
}


/*================================================================*/
/* Verdict list                                                   */
/*================================================================*/

enum {
  REPORT_LIST_COL_NAME=0,
  REPORT_LIST_COL_VERDICT,
  REPORT_LIST_COL_VERDICT_VALUE,
  REPORT_LIST_COL_CRITICITY_DEFAULT,
  REPORT_LIST_COL_CRITICITY_DEFAULT_COLOR,
  REPORT_LIST_COL_CRITICITY_DEFAULT_VALUE,
  REPORT_LIST_COL_CRITICITY,
  REPORT_LIST_COL_CRITICITY_COLOR,
  REPORT_LIST_COL_CRITICITY_VALUE,
  REPORT_LIST_COL_KEY,
  REPORT_LIST_NCOLS
};

enum {
  REPORT_LIST_SORT_NAME=0,
  REPORT_LIST_SORT_CRITICITY,
  REPORT_LIST_SORT_VERDICT,
  REPORT_LIST_NSORTS
};


static void report_gui_list_verdict_iter(GtkListStore *model, GtkTreeIter *iter,
					 tree_result_t *result)
{
  int verdict = VERDICT_UNEXECUTED;
  int criticity = CRITICITY_NONE;
  char *id = "";
  char *color = "black";

  if ( result != NULL ) {
    verdict = result->verdict;
    criticity = result->criticity;
  }

  if ( (criticity > CRITICITY_NONE) && (verdict == VERDICT_FAILED) ) {
    id = criticity_id(criticity);
    color = criticity_color(criticity);
  }

  gtk_list_store_set(model, iter,
		     REPORT_LIST_COL_VERDICT, report_gui_verdict_pixbuf(verdict),
		     REPORT_LIST_COL_VERDICT_VALUE, verdict,
		     REPORT_LIST_COL_CRITICITY, id,
		     REPORT_LIST_COL_CRITICITY_COLOR, color,
		     REPORT_LIST_COL_CRITICITY_VALUE, criticity,
		     -1);
}


typedef struct {
  unsigned long key;
  tree_result_t *result;
} report_gui_list_verdict_data_t;


static int report_gui_list_verdict_update(GtkListStore *model,
					  GtkTreePath *path, GtkTreeIter *iter,
					  report_gui_list_verdict_data_t *data)
{
  unsigned long key2;

  gtk_tree_model_get(GTK_TREE_MODEL(model), iter,
		     REPORT_LIST_COL_KEY, &key2, -1);
  if ( key2 == 0 )
    return TRUE;

  if ( (data->key == 0) || (data->key == key2) ) {
    report_gui_list_verdict_iter(model, iter, data->result);
  }

  return (data->key == key2);
}


static void report_gui_list_verdict(report_gui_t *rg, unsigned long key, tree_result_t *result)
{
  report_gui_list_verdict_data_t data;

  if ( rg->list.model == NULL )
    return;

  data.key = key;
  data.result = result;

  gtk_tree_model_foreach(GTK_TREE_MODEL(rg->list.model),
			 (GtkTreeModelForeachFunc) report_gui_list_verdict_update,
			 &data);
}


static void report_gui_list_tree_clicked(report_gui_t *rg)
{
  if ( rg->spot_tree_func != NULL )
    rg->spot_tree_func(rg->spot_tree_arg, rg->list.select_key);
}


static void report_gui_list_log_clicked(report_gui_t *rg)
{
  report_gui_log_show(rg, rg->list.select_name);
}


static void report_gui_list_destroyed(report_gui_t *rg)
{
  rg->list.window = NULL;
  rg->list.spot_tree = NULL;
  rg->list.spot_log = NULL;

  gtk_list_store_clear(rg->list.model);
  rg->list.view = NULL;
  rg->list.model = NULL;
}


static gint report_gui_list_sort(GtkListStore *model,
				 GtkTreeIter *iter1, GtkTreeIter *iter2,
				 gpointer data)
{
  static unsigned int verdict_order[VERDICT_MAX] = {
    0, /* UNEXECUTED */
    2, /* PASSED */
    3, /* FAILED */
    4, /* INCONCLUSIVE */
    1, /* SKIP */
  };
  int id = GPOINTER_TO_INT(data);
  char *name1, *name2;
  int crit1, crit2;
  int val1 = 0;
  int val2 = 0;
  int ret;

  switch ( id ) {
  case REPORT_LIST_SORT_CRITICITY:
    /* Get criticity values */
    gtk_tree_model_get(GTK_TREE_MODEL(model), iter1,
		       REPORT_LIST_COL_CRITICITY_DEFAULT_VALUE, &val1, -1);
    gtk_tree_model_get(GTK_TREE_MODEL(model), iter2,
		       REPORT_LIST_COL_CRITICITY_DEFAULT_VALUE, &val2, -1);
    break;

  case REPORT_LIST_SORT_VERDICT:
    /* Get verdict/criticity values */
    gtk_tree_model_get(GTK_TREE_MODEL(model), iter1,
		       REPORT_LIST_COL_VERDICT_VALUE, &val1,
		       REPORT_LIST_COL_CRITICITY_VALUE, &crit1,
		       -1);
    gtk_tree_model_get(GTK_TREE_MODEL(model), iter2,
		       REPORT_LIST_COL_VERDICT_VALUE, &val2,
		       REPORT_LIST_COL_CRITICITY_VALUE, &crit2,
		       -1);

    /* Compare verdicts */
    val1++;
    if ( val1 >= VERDICT_MAX )
      val1 = VERDICT_MAX-1;
    val1 = (verdict_order[val1] << 8) + (crit1 & 0xFF);

    val2++;
    if ( val2 >= VERDICT_MAX )
      val2 = VERDICT_MAX-1;
    val2 = (verdict_order[val2] << 8) + (crit2 & 0xFF);
    break;

  default:
    break;
  }

  if ( val1 > val2 )
    return +1;
  if ( val1 < val2 )
    return -1;

  /* Get node names */
  gtk_tree_model_get(GTK_TREE_MODEL(model), iter1,
		     REPORT_LIST_COL_NAME, &name1, -1);
  gtk_tree_model_get(GTK_TREE_MODEL(model), iter2,
		     REPORT_LIST_COL_NAME, &name2, -1);

  ret = strcmp(name1, name2);

  free(name1);
  free(name2);

  return ret;
}


static void report_gui_list_select_clear(report_gui_list_t *rgl)
{
  rgl->select_key = 0;

  if ( rgl->select_name != NULL ) {
    free(rgl->select_name);
    rgl->select_name = NULL;
  }
}


static gboolean report_gui_list_select(GtkTreeSelection *selection, GtkTreeModel *model,
				       GtkTreePath *path, gboolean path_currently_selected,
				       report_gui_list_t *rgl)
{
  int new_state = path_currently_selected ? 0:1;
  gboolean ret = TRUE;

  /* Clear previous selection */
  report_gui_list_select_clear(rgl);

  /* Set new selection */
  if ( new_state ) {
    GtkTreeIter iter;

    /* Get selected row */
    gtk_tree_model_get_iter(model, &iter, path);
    gtk_tree_model_get(model, &iter,
		       REPORT_LIST_COL_NAME, &(rgl->select_name),
		       REPORT_LIST_COL_KEY, &(rgl->select_key),
		       -1);

    if ( rgl->select_key == 0 ) {
      new_state = 0;
      ret = FALSE;
    }
  }

  gtk_widget_set_sensitive(rgl->spot_tree, new_state);
  gtk_widget_set_sensitive(rgl->spot_log, new_state);

  return ret;
}


static void report_gui_list_feed_object(tree_object_t *object, GtkListStore *model)
{
  GtkTreeIter iter;
  PangoColor color;
  char color_str[16] = "black";

  if ( object->type != TYPE_CASE )
    return;

  /* Create new list item */
  gtk_list_store_append(model, &iter);

  if ( pango_color_parse(&color, criticity_color(object->d.Case->criticity)) ) {
    color.red += 0x3333;
    color.green += 0x3333;
    color.blue += 0x3333;
    snprintf(color_str, sizeof(color_str), "#%04X%04X%04X", color.red, color.green, color.blue);
  }

  gtk_list_store_set(model, &iter,
		     REPORT_LIST_COL_NAME, object->parent_item->name,
		     REPORT_LIST_COL_KEY, object->key,
		     REPORT_LIST_COL_CRITICITY_DEFAULT, criticity_id(object->d.Case->criticity),
		     REPORT_LIST_COL_CRITICITY_DEFAULT_COLOR, color_str,
		     REPORT_LIST_COL_CRITICITY_DEFAULT_VALUE, object->d.Case->criticity,
		     -1);

  report_gui_list_verdict_iter(model, &iter, &(object->d.Case->result));
}


static void report_gui_list_feed(report_gui_t *rg)
{
  report_gui_list_t *rgl = &(rg->list);

  /* Build list content */
  tree_foreach(rg->tree, (tree_func_t *) report_gui_list_feed_object, rgl->model);
}


static void report_gui_list_clicked(report_gui_t *rg)
{
  report_gui_list_t *rgl = &(rg->list);
  GtkTreeViewColumn *column;
  GtkCellRenderer *renderer;
  GtkTreeSelection *selection;
  int i;

  if ( rgl->window == NULL ) {
    rgl->window = create_report_list_window();

    /* Create list display widget */
    rgl->view = GTK_TREE_VIEW(lookup_widget(rgl->window, "report_list_view"));

    rgl->model = gtk_list_store_new(REPORT_LIST_NCOLS,
				    /* Node Name   */      G_TYPE_STRING,
				    /* Verdict     */      GDK_TYPE_PIXBUF,
				    /* Verdict Value */    G_TYPE_INT,
				    /* Def. Crit.  */      G_TYPE_STRING,
				    /* Def. Crit. Color */ G_TYPE_STRING,
				    /* Def. Crit. Value */ G_TYPE_INT,
				    /* Criticity   */      G_TYPE_STRING,
				    /* Crit. Color */      G_TYPE_STRING,
				    /* Criticity Value */  G_TYPE_INT,
				    /* Key         */      G_TYPE_ULONG);
    rgl->model_sort = gtk_tree_model_sort_new_with_model(GTK_TREE_MODEL(rgl->model));
    gtk_tree_view_set_model(rgl->view, rgl->model_sort);

    /* Setup tree selection handler */
    report_gui_list_select_clear(rgl);
    selection = gtk_tree_view_get_selection(rgl->view);
    gtk_tree_selection_set_mode(selection, GTK_SELECTION_SINGLE);
    gtk_tree_selection_set_select_function(selection,
					   (GtkTreeSelectionFunc) report_gui_list_select, &(rg->list),
					   NULL);

    /* Column #0: Test Case name */
    renderer = gtk_cell_renderer_text_new();
    gtk_tree_view_insert_column_with_attributes(rgl->view, -1, "Test Case", renderer,
						"text", REPORT_LIST_COL_NAME,
						NULL);
    column = gtk_tree_view_get_column(rgl->view, 0);
    gtk_tree_view_column_set_clickable(column, TRUE);
    gtk_tree_view_column_set_sort_column_id(column, REPORT_LIST_SORT_NAME);
    gtk_tree_view_column_set_resizable(column, TRUE);
    gtk_tree_view_column_set_expand(column, TRUE);

    /* Column #1: Default criticity */
    renderer = gtk_cell_renderer_text_new();
    gtk_object_set(GTK_OBJECT(renderer), "scale", 0.7, NULL);
    gtk_tree_view_insert_column_with_attributes(rgl->view, -1, "Def. Criticity", renderer,
						"text", REPORT_LIST_COL_CRITICITY_DEFAULT,
						"foreground", REPORT_LIST_COL_CRITICITY_DEFAULT_COLOR,
						NULL);
    column = gtk_tree_view_get_column(rgl->view, 1);
    gtk_tree_view_column_set_clickable(column, TRUE);
    gtk_tree_view_column_set_sort_column_id(column, REPORT_LIST_SORT_CRITICITY);
    gtk_tree_view_column_set_resizable(column, FALSE);
    gtk_tree_view_column_set_sizing(column, GTK_TREE_VIEW_COLUMN_AUTOSIZE);

    /* Column #2: Verdict and Criticity */
    column = gtk_tree_view_column_new();
    gtk_tree_view_column_set_title(column, "Verdict");

    renderer = gtk_cell_renderer_pixbuf_new();
    gtk_tree_view_column_pack_start(column, renderer, FALSE);
    gtk_tree_view_column_add_attribute(column, renderer, "pixbuf", REPORT_LIST_COL_VERDICT);

    renderer = gtk_cell_renderer_text_new();
    gtk_object_set(GTK_OBJECT(renderer), "scale", 0.7, NULL);
    gtk_tree_view_column_pack_start(column, renderer, TRUE);
    gtk_tree_view_column_add_attribute(column, renderer, "text", REPORT_LIST_COL_CRITICITY);
    gtk_tree_view_column_add_attribute(column, renderer, "foreground", REPORT_LIST_COL_CRITICITY_COLOR);

    gtk_tree_view_column_set_clickable(column, TRUE);
    gtk_tree_view_column_set_sort_column_id(column, REPORT_LIST_SORT_VERDICT);
    gtk_tree_view_column_set_resizable(column, FALSE);
    gtk_tree_view_column_set_sizing(column, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
    gtk_tree_view_append_column(rgl->view, column);

    /* Setup list sort functions */
    for (i = 0; i < REPORT_LIST_NSORTS; i++)
      gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(rgl->model_sort), i,
				      (GtkTreeIterCompareFunc) report_gui_list_sort, GINT_TO_POINTER(i),
				      NULL);

    /* Setup spot buttons */
    rgl->spot_tree = lookup_widget(rgl->window, "report_list_tree");
    gtk_widget_set_sensitive(rgl->spot_tree, 0);
    gtk_signal_connect_object(GTK_OBJECT(rgl->spot_tree),
			      "clicked", (GtkSignalFunc) report_gui_list_tree_clicked, rg);

    rgl->spot_log = lookup_widget(rgl->window, "report_list_log");
    gtk_widget_set_sensitive(rgl->spot_log, 0);
    gtk_signal_connect_object(GTK_OBJECT(rgl->spot_log),
			      "clicked", (GtkSignalFunc) report_gui_list_log_clicked, rg);


    /* Setup termination events */
    gtk_signal_connect_object(GTK_OBJECT(rgl->window), "destroy",
			      (GtkSignalFunc) report_gui_list_destroyed, rg);
    gtk_signal_connect_object(GTK_OBJECT(lookup_widget(rgl->window, "report_list_close")),
                              "clicked", GTK_SIGNAL_FUNC(gtk_widget_destroy), GTK_OBJECT(rgl->window));

    /* Feed the list */
    report_gui_list_feed(rg);

    gtk_widget_show(rgl->window);
    gtk_tree_selection_unselect_all(selection);
  }
  else {
    gtk_window_present(GTK_WINDOW(rgl->window));
  }
}


/*================================================================*/
/* Report generation                                              */
/*================================================================*/

static void report_gui_build_terminated(int status, report_gui_t *rg)
{
  rg->builder = NULL;

  /* Write child process termination status */
  if ( rg->child_pipe[1] > 0 )
    write(rg->child_pipe[1], &status, sizeof(status));
}


static void report_gui_view(report_gui_t *rg);

static void report_gui_build_completed(report_gui_t *rg, int fd, GdkInputCondition condition)
{
  int status;

  /* Read child process termination status */
  read(fd, &status, sizeof(status));
  //fprintf(stderr, "Test Report completed with status %d\n", status);

  /* Write message to status bar */
  if ( status == 0 ) {
    status_mesg("Test report \"%s\" written", rg->report_name);
  }
  else {
    eprintf("Error building Test Report\n\"%s\"\nfrom Test Output\n\"%s\"", rg->report_name, rg->out->file->name);
  }

  /* Restore buttons sensitivity */
  gtk_widget_set_sensitive(rg->rtv_build, 1);
  gtk_widget_set_sensitive(rg->rtv_view, 1);
  gtk_widget_set_sensitive(rg->rtv_view2, 1);

  /* Call Test Report viewer if requested */
  if ( (status == 0) && (rg->build_then_browse) ) {
    rg->build_then_browse = 0;
    report_gui_view(rg);
  }
}


static void report_gui_build(report_gui_t *rg, int browse)
{
  char *argv[] = {NULL, NULL, NULL};

  /* Only once... */
  if ( rg->builder != NULL )
    return;

  /* Check output file presence */
  if ( access(rg->out->file->name, R_OK) != 0 ) {
    eprintf("Test Output file not found:\n%s", rg->out->file->name);
    return;
  }

  rg->build_then_browse = browse;

  /* Build HTML document */
  argv[0] = "testfarm-report";
  argv[1] = rg->out->file->name;

  rg->builder = child_spawn(argv, -1, -1, -1, (child_handler_t *) report_gui_build_terminated, rg);
  if ( rg->builder == NULL ) {
    status_mesg("Cannot execute report generator \"%s\"", argv[0]);
  }
  else {
    gtk_widget_set_sensitive(rg->rtv_build, 0);
    gtk_widget_set_sensitive(rg->rtv_view, 0);
    gtk_widget_set_sensitive(rg->rtv_view2, 0);
  }
}


static void report_gui_build_clicked(GtkWidget *widget, report_gui_t *rg)
{
  report_gui_build(rg, 0);
}


/*================================================================*/
/* Report viewer                                                  */
/*================================================================*/

static void report_gui_view(report_gui_t *rg)
{
  static int null_stdout = -1;
  static int null_stderr = -1;
  char *argv[] = {NULL, NULL, NULL};
  char *url;
  int size;

  /* Open /dev/null if not already done */
  if (null_stdout < 0) {
	  null_stdout = open("/dev/null", O_WRONLY);
	  if (null_stdout < 0) {
		  perror("Cannot open '/dev/null' for stdout redirection");
	  }
  }

  if (null_stdout >= 0) {
	  fprintf(stderr, "Browser stdout redirected to /dev/null (fd=%d)\n", null_stdout);
  }

  if (null_stderr < 0) {
	  null_stderr = open("/dev/null", O_WRONLY);
	  if (null_stderr < 0) {
		  perror("Cannot open '/dev/null' for stderr redirection");
	  }
  }

  if (null_stderr >= 0) {
	  fprintf(stderr, "Browser stderr redirected to /dev/null (fd=%d)\n", null_stderr);
  }

  /* Build URL */
  size = strlen(rg->report_name) + 6;
  url = (char *) malloc(size);
  snprintf(url, size, "file:%s", rg->report_name);

  /* Spawn HTML browser */
  argv[0] = get_browser();
  argv[1] = url;
  fprintf(stderr, "Start Test Report viewer: %s %s\n", argv[0], argv[1]);

  if (child_spawn(argv, -1 /* stdin */, null_stdout /* stdout */, null_stderr /* stderr */,
		  (child_handler_t *) NULL, NULL) == NULL) {
	  status_mesg("Cannot execute browser \"%s\"", argv[0]);
  }

  free(url);
}


static void report_gui_view_clicked(GtkWidget *widget, report_gui_t *rg)
{
  report_gui_build(rg, 1);
}


/*================================================================*/
/* Progress display                                               */
/*================================================================*/

static void report_gui_sensitive(report_gui_t *rg, int status)
{
  gtk_widget_set_sensitive(rg->button_list[0], status);
  gtk_widget_set_sensitive(rg->button_list[1], status);
  gtk_widget_set_sensitive(rg->log, status);
  gtk_widget_set_sensitive(rg->log2, status);
  gtk_widget_set_sensitive(rg->rtv_build, status);
  gtk_widget_set_sensitive(rg->rtv_view, status);
  gtk_widget_set_sensitive(rg->rtv_view2, status);
}

extern long int lround(double x);

static void report_gui_progress(GtkWidget *pbar, gdouble fraction)
{
  char *title = gtk_object_get_data(GTK_OBJECT(pbar), "title");
  char text[32];

  if ( fraction > 1 )
    fraction = 1;

  snprintf(text, sizeof(text), "%s: %ld %%", title, lround(fraction*100));
  gtk_progress_bar_set_text(GTK_PROGRESS_BAR(pbar), text);
  gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(pbar), fraction);
}


void report_gui_reset(report_gui_t *rg)
{
  /* Reset progress bars */
  gtk_label_set_text(GTK_LABEL(rg->significant), "0");
  report_gui_progress(rg->executed, 0.0);
  report_gui_progress(rg->passed, 0.0);
  report_gui_progress(rg->failed, 0.0);
  report_gui_progress(rg->inconclusive, 0.0);
  report_gui_progress(rg->skip, 0.0);

  /* Refresh verdict pixmaps */
  report_gui_list_verdict(rg, 0, NULL);
}


static void report_gui_total(report_gui_t *rg)
{
  if ( rg->out != NULL )
    gtk_label_set_text(GTK_LABEL(rg->total), strnum(rg->out->stat.ncase));
  else
    gtk_label_set_text(GTK_LABEL(rg->total), "0");
}


void report_gui_set_name(report_gui_t *rg, char *log_name, char *report_directory)
{
  int size = strlen(report_directory) + 16;

  /* Change log name */
  if ( rg->log_name != NULL )
    free(rg->log_name);
  rg->log_name = strdup(log_name);

  /* Setup log viewer */
  if ( rg->log_viewer.child != NULL )
    report_gui_proc_command(&(rg->log_viewer), 'L', rg->log_name);

  /* Change report name */
  if ( rg->report_name != NULL )
    free(rg->report_name);
  rg->report_name = malloc(size);
  snprintf(rg->report_name, size, "%s" G_DIR_SEPARATOR_S "index.html", report_directory);
}


void report_gui_load(report_gui_t *rg, tree_t *tree, output_t *out)
{
  if ( rg == NULL )
    return;

  /* Unload previous stuffs */
  report_gui_unload(rg);

  /* Set tree-related data */
  rg->tree = tree;
  rg->out = out;

  /* Report total number of test case */
  report_gui_total(rg);
  report_gui_reset(rg);

  report_gui_sensitive(rg, 1);
}


void report_gui_executed(report_gui_t *rg, unsigned long key, tree_result_t *result)
{
  output_stat_t *stat = &(rg->out->stat);
  float n = (float) (stat->ncase);
  int significant = stat->passed + stat->failed + stat->inconclusive;

  /* Update significant test counter */
  gtk_label_set_text(GTK_LABEL(rg->significant), strnum(significant));

  /* Update progress bars */
  report_gui_progress(rg->executed, (float) (stat->executed) / n);
  report_gui_progress(rg->skip, (float) (stat->skip) / n);
  if ( significant > 0 ) {
    report_gui_progress(rg->passed, (float) (stat->passed) / (float) significant);
    report_gui_progress(rg->failed, (float) (stat->failed) / (float) significant);
    report_gui_progress(rg->inconclusive, (float) (stat->inconclusive) / (float) significant);
  }

  /* Update test case list */
  report_gui_list_verdict(rg, key, result);
}


static void report_gui_clear(report_gui_t *rg)
{
  if ( rg == NULL )
    return;

  if ( rg->list.window != NULL ) {
    report_gui_list_select_clear(&(rg->list));
    gtk_widget_destroy(rg->list.window);
    rg->list.window = NULL;
    rg->list.view = NULL;
    rg->list.model = NULL;
  }

  /* Clear tree-related data */
  rg->tree = NULL;
  rg->out = NULL;

  /* Free log file name */
  if ( rg->log_name != NULL ) {
    free(rg->log_name);
    rg->log_name = NULL;
  }

  /* Clear report name */
  if ( rg->report_name != NULL ) {
    free(rg->report_name);
    rg->report_name = NULL;
  }
}


void report_gui_unload(report_gui_t *rg)
{
  if ( rg == NULL )
    return;

  /* Clear what is currently loaded */
  report_gui_clear(rg);

  /* Report total number of test case */
  report_gui_total(rg);
  report_gui_reset(rg);

  /* Disable some widgets when no tree is loaded */
  report_gui_sensitive(rg, 0);
}


/*================================================================*/
/*================================================================*/

report_gui_t *report_gui_init(GtkWidget *window,
			      report_gui_spot_tree_t *spot_tree_func, void *spot_tree_arg)
{
  report_gui_t *rg;

  rg = (report_gui_t *) malloc(sizeof(report_gui_t));
  rg->window = window;
  rg->total = lookup_widget(window, "report_total_value");
  rg->significant = lookup_widget(window, "report_significant_value");
  rg->executed = lookup_widget(window, "report_executed_slider");
  rg->passed = lookup_widget(window, "report_passed_slider");
  rg->failed = lookup_widget(window, "report_failed_slider");
  rg->inconclusive = lookup_widget(window, "report_inconclusive_slider");
  rg->skip = lookup_widget(window, "report_skip_slider");
  rg->rtv_build = lookup_widget(window, "results_build_report");

  gtk_object_set_data(GTK_OBJECT(rg->executed), "title", "Executed");
  gtk_object_set_data(GTK_OBJECT(rg->passed), "title", "PASSED");
  gtk_object_set_data(GTK_OBJECT(rg->failed), "title", "FAILED");
  gtk_object_set_data(GTK_OBJECT(rg->inconclusive), "title", "INCONCLUSIVE");
  gtk_object_set_data(GTK_OBJECT(rg->skip), "title", "Skipped");

  /* Verdict list window activation */
  rg->button_list[0] = lookup_widget(window, "results_verdicts");
  rg->button_list[1] = lookup_widget(window, "tool_verdicts");
  gtk_signal_connect_object(GTK_OBJECT(rg->button_list[0]), "activate",
			    GTK_SIGNAL_FUNC(report_gui_list_clicked), rg);
  gtk_signal_connect_object(GTK_OBJECT(rg->button_list[1]), "clicked",
			    GTK_SIGNAL_FUNC(report_gui_list_clicked), rg);

  /* Log viewer window activation */
  rg->log = lookup_widget(window, "results_log");
  rg->log2 = lookup_widget(window, "tool_log");
  gtk_signal_connect(GTK_OBJECT(rg->log), "activate",
                     GTK_SIGNAL_FUNC(report_gui_log_clicked), rg);
  gtk_signal_connect(GTK_OBJECT(rg->log2), "clicked",
                     GTK_SIGNAL_FUNC(report_gui_log_clicked), rg);

  /* Report generation and viewer activation */
  rg->rtv_view = lookup_widget(window, "results_view_report");
  rg->rtv_view2 = lookup_widget(window, "tool_report");
  gtk_signal_connect(GTK_OBJECT(rg->rtv_view), "activate",
                     GTK_SIGNAL_FUNC(report_gui_view_clicked), rg);
  gtk_signal_connect(GTK_OBJECT(rg->rtv_view2), "clicked",
                     GTK_SIGNAL_FUNC(report_gui_view_clicked), rg);

  gtk_signal_connect(GTK_OBJECT(rg->rtv_build), "activate",
                     GTK_SIGNAL_FUNC(report_gui_build_clicked), rg);
  gtk_signal_connect(GTK_OBJECT(lookup_widget(window, "results_report_options")), "activate",
				GTK_SIGNAL_FUNC(report_gui_conf_clicked), rg);

  report_gui_sensitive(rg, 0);

  rg->tree = NULL;
  rg->out = NULL;
  rg->log_name = NULL;
  rg->report_name = NULL;

  /* Init Verdict List control */
  rg->list.window = NULL;
  rg->list.view = NULL;
  rg->list.model = NULL;
  rg->list.spot_tree = NULL;
  rg->list.spot_log = NULL;
  rg->list.select_key = 0;
  rg->list.select_name = NULL;

  /* Init Log Viewer control */
  report_gui_proc_clear(&(rg->log_viewer));

  /* Init Report Config control */
  report_gui_proc_clear(&(rg->report_conf));

  /* Setup spot button handling */
  rg->spot_tree_func = spot_tree_func;
  rg->spot_tree_arg  = spot_tree_arg;

  /* Setup child termination handling */
  rg->child_pipe[0] = -1;
  rg->child_pipe[1] = -1;
  rg->child_input = -1;
  rg->builder = NULL;
  rg->build_then_browse = 0;

  if ( pipe(rg->child_pipe) ) {
    fprintf(stderr, "Unable to create Report Generator pipe: %s", strerror(errno));
  }
  else {
    /* Enable close-on-exec mode because it is an internal pipe */
    fcntl(rg->child_pipe[0], F_SETFD, FD_CLOEXEC);
    fcntl(rg->child_pipe[1], F_SETFD, FD_CLOEXEC);

    rg->child_input = gdk_input_add(rg->child_pipe[0], GDK_INPUT_READ,
                                    (GdkInputFunction) report_gui_build_completed, rg);
  }

  /* Reset progress bars and their friends */
  report_gui_reset(rg);

  return rg;
}


void report_gui_destroy(report_gui_t *rg)
{
  if ( rg == NULL )
    return;

  /* Close Test Report child termination handling */
  if ( rg->child_input >= 0 )
    gdk_input_remove(rg->child_input);
  rg->child_input = -1;

  if ( rg->child_pipe[0] >= 0 )
    close(rg->child_pipe[0]);
  rg->child_pipe[0] = -1;

  if ( rg->child_pipe[1] >= 0 )
    close(rg->child_pipe[1]);
  rg->child_pipe[1] = -1;

  /* Close Log Viewer */
  report_gui_proc_terminate(&(rg->log_viewer));

  /* Close Report Config */
  report_gui_proc_terminate(&(rg->report_conf));

  report_gui_clear(rg);

  free(rg);
}
