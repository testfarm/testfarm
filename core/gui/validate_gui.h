/****************************************************************************/
/* TestFarm                                                                 */
/* Graphical User Interface : File validation user interface                */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 16-APR-2004                                                    */
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

#ifndef __TESTFARM_VALIDATE_GUI_H__
#define __TESTFARM_VALIDATE_GUI_H__

#include <gtk/gtk.h>
#include "codegen.h"
#include "properties.h"
#include "validate.h"

typedef struct {
  properties_t *prop;
  GtkWidget *w_expander;
  GtkWidget *w_box;
  GtkWidget *w_icon;
  GtkLabel *w_script;
  GtkLabel *w_last;
  GtkWidget *w_script_modified;
  GtkWidget *w_menu;
  GtkWidget *w_updated;

  tree_object_t *selected;
  tree_object_t *checked;
  int validated;
  int level;
} validate_gui_t;


extern validate_gui_t *validate_gui_init(GtkWidget *window, properties_t *prop);
extern void validate_gui_destroy(validate_gui_t *vg);

extern void validate_gui_select(validate_gui_t *vg, tree_object_t *object);

#endif  /* __TESTFARM_VALIDATE_GUI_H__ */
