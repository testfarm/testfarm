/****************************************************************************/
/* TestFarm                                                                 */
/* Graphical User Interface : Manual Input user interface                   */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 05-MAR-2007                                                    */
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

#ifndef __TESTFARM_INPUT_GUI_H__
#define __TESTFARM_INPUT_GUI_H__

#include <gtk/gtk.h>

typedef void input_gui_done_t(void *arg, int verdict, char *text);

typedef struct {
  GtkWidget *w_expander;
  GtkWidget *w_box;
  GtkWidget *w_textview;
  GtkTextBuffer *w_textbuffer;
  GtkWidget *w_passed;
  GtkWidget *w_failed;
  GtkWidget *w_inconclusive;
  input_gui_done_t *done_fn;
  void *done_arg;
} input_gui_t;

extern input_gui_t *input_gui_init(GtkWidget *window, input_gui_done_t *done_fn, void *done_arg);
extern void input_gui_destroy(input_gui_t *ig);

extern void input_gui_sensitivity(input_gui_t *ig, gboolean active);
extern void input_gui_wakeup(input_gui_t *ig);

#endif /* __TESTFARM_INPUT_GUI_H__ */
