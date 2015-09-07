/****************************************************************************/
/* TestFarm                                                                 */
/* Graphical User Interface : Stdin/Stdout/Stderr display                   */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 07-MAR-2005                                                    */
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

#ifndef __TESTFARM_OUTPUT_GUI_H__
#define __TESTFARM_OUTPUT_GUI_H__

#include <gtk/gtk.h>
#include "output.h"

typedef struct {
  GtkWidget *window;
  GtkWidget *label;
  GtkWidget *clist;
  unsigned long depth;
  output_t *out;
} output_gui_t;

extern output_gui_t *output_gui_init(GtkWidget *window);
extern void output_gui_destroy(output_gui_t *og);

extern void output_gui_clear(output_gui_t *og);
extern void output_gui_set(output_gui_t *og, output_t *out);

extern void output_gui_show(output_gui_t *og, char *name, int is_root);
extern void output_gui_feed(output_gui_t *og, int channel, char *str);
extern void output_gui_scrolldown(output_gui_t *og);

#endif /* __TESTFARM_OUTPUT_GUI_H__ */
