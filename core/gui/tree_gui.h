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
    along with TestFarm.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __TREE_GUI_H
#define __TREE_GUI_H

#include <gtk/gtk.h>

#include "codegen.h"
#include "properties.h"
#include "output_gui.h"
#include "input_gui.h"
#include "validate_gui.h"
#include "report_gui.h"
#include "system_gui.h"


typedef struct {
  tree_object_t *object;
  GtkTreeIter iter;
} tree_gui_ref_t;


typedef struct {
  GtkWidget *window;
  GtkTreeStore *tree_model;
  GtkTreeView *tree_view;
  GtkTreeSelection *tree_selection;

  GtkWidget *reset[2];
  GtkWidget *go[2];
  GtkWidget *step[2];
  GtkWidget *abort[2];

  GtkWidget *breakpoint[2];
  GtkWidget *force_skip[2];
  GtkWidget *clear_brkpt_skip[2];
  GtkWidget *debug;
  GtkWidget *running;

  GtkWidget *spot_tree;
  GtkWidget *spot_log;

  tree_gui_ref_t current;
  tree_gui_ref_t selected;
  int exec_state;

  void *actions_arg;

  output_gui_t *og;
  input_gui_t *ig;
  validate_gui_t *vg;
  report_gui_t *rg;
  system_gui_t *sg;
} tree_gui_t;


/* Init */
extern tree_gui_t *tree_gui_init(GtkWidget *window, properties_t *prop, void *arg);
extern void tree_gui_destroy(tree_gui_t *gui);

/* Incoming execution events */
extern void tree_gui_set_state(tree_gui_t *gui, int state);
extern void tree_gui_set_current(tree_gui_t *gui, unsigned long key);
extern void tree_gui_set_report(tree_gui_t *gui, char *log_name, char *report_directory);

/* Tree rendering */
extern void tree_gui_load(tree_gui_t *gui, tree_t *tree, output_t *out);
extern void tree_gui_unload(tree_gui_t *gui);
extern void tree_gui_spot(tree_gui_t *gui, unsigned long key);
extern void tree_gui_verdict(tree_gui_t *gui, unsigned long key, tree_result_t *result);
extern void tree_gui_output(tree_gui_t *gui, unsigned long key, int channel, char *str);

/* Main window close function */
extern void tree_gui_quit(tree_gui_t *gui);

#endif /* __TREE_GUI_H */
