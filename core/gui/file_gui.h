/****************************************************************************/
/* Basil Dev - TestFarm                                                     */
/* Test Suite User Interface : file management                              */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 10-MAY-2000                                                    */
/****************************************************************************/

/*
 * $Revision: 323 $
 * $Date: 2006-11-29 14:54:38 +0100 (mer., 29 nov. 2006) $
 */

#ifndef __FILE_GUI_H__
#define __FILE_GUI_H__

#include <gtk/gtk.h>
#include "codegen.h"

#include "tree_run.h"


typedef struct {
  GtkWidget *button;
  GtkWidget *window;
  GtkListStore *list;
  GtkWidget *spot;
  tree_object_t *object;
} file_gui_report_t;

typedef struct {
  GtkWidget *window;
  tree_run_t *tree_run;
  file_gui_report_t report;
  GtkWidget *reload;
  GtkWidget *check;
} file_gui_t;

extern file_gui_t *file_gui_init(GtkWidget *window, tree_run_t *tree_run);
extern void file_gui_done(file_gui_t *file);
extern void file_gui_load(file_gui_t *file, char *filename);
extern void file_gui_reload(file_gui_t *file);

#endif /* __FILE_GUI_H__ */
