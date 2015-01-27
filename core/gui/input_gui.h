/****************************************************************************/
/* TestFarm                                                                 */
/* Graphical User Interface : Manual Input user interface                   */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 05-MAR-2007                                                    */
/****************************************************************************/

/*
 * $Revision: 398 $
 * $Date: 2007-03-07 12:21:06 +0100 (mer., 07 mars 2007) $
 */

#ifndef __INPUT_GUI_H__
#define __INPUT_GUI_H__

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

#endif /* __INPUT_GUI_H__ */
