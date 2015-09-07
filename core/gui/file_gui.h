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

#ifndef __TESTFARM_FILE_GUI_H__
#define __TESTFARM_FILE_GUI_H__

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

#endif /* __TESTFARM_FILE_GUI_H__ */
