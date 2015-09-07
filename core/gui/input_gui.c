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

#include <stdio.h>
#include <malloc.h>
#include <gtk/gtk.h>

#include "codegen.h"
#include "support.h"
#include "color.h"
#include "input_gui.h"


static void input_gui_button_clicked(GtkWidget *widget, input_gui_t *ig)
{
  int verdict = GPOINTER_TO_INT(gtk_object_get_data(GTK_OBJECT(widget), "verdict"));
  GtkTextIter start, end;
  char *text;

  gtk_widget_set_sensitive(ig->w_box, FALSE);

  gtk_text_buffer_get_start_iter(ig->w_textbuffer, &start);
  gtk_text_buffer_get_end_iter(ig->w_textbuffer, &end);
  text = gtk_text_buffer_get_text(ig->w_textbuffer, &start, &end, FALSE);

  if ( (ig != NULL) && (ig->done_fn != NULL) )
    ig->done_fn(ig->done_arg, verdict, text);

  g_free(text);
  gtk_text_buffer_set_text(ig->w_textbuffer, "", -1);
}


void input_gui_sensitivity(input_gui_t *ig, gboolean active)
{
  gtk_widget_set_sensitive(ig->w_box, active);
}


void input_gui_wakeup(input_gui_t *ig)
{
  if ( ig == NULL )
    return;

  gtk_text_buffer_set_text(ig->w_textbuffer, "", -1);

  gtk_expander_set_expanded(GTK_EXPANDER(ig->w_expander), TRUE);
  gtk_widget_set_sensitive(ig->w_box, TRUE);
}


input_gui_t *input_gui_init(GtkWidget *window, input_gui_done_t *done_fn, void *done_arg)
{
  input_gui_t *ig;

  /* Alloc input GUI descriptor */
  ig = (input_gui_t *) malloc(sizeof(input_gui_t));

  /* Retrieve widgets */
  ig->w_expander = lookup_widget(window, "input_expander");
  ig->w_box = lookup_widget(window, "input_box");
  ig->w_textview = lookup_widget(window, "input_textview");
  ig->w_textbuffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(ig->w_textview));
  ig->w_passed = lookup_widget(window, "input_passed");
  ig->w_failed = lookup_widget(window, "input_failed");
  ig->w_inconclusive = lookup_widget(window, "input_inconclusive");

  /* Setup input termination handler */
  ig->done_fn = done_fn;
  ig->done_arg = done_arg;

  /* Setup text color */
  gtk_widget_modify_text(ig->w_textview, GTK_STATE_NORMAL, &color_blue);

  /* Handle button events */
  gtk_object_set_data(GTK_OBJECT(ig->w_passed), "verdict", GINT_TO_POINTER(VERDICT_PASSED));
  gtk_signal_connect(GTK_OBJECT(ig->w_passed),
		     "clicked", (GtkSignalFunc) input_gui_button_clicked, ig);

  gtk_object_set_data(GTK_OBJECT(ig->w_failed), "verdict", GINT_TO_POINTER(VERDICT_FAILED));
  gtk_signal_connect(GTK_OBJECT(ig->w_failed),
		     "clicked", (GtkSignalFunc) input_gui_button_clicked, ig);

  gtk_object_set_data(GTK_OBJECT(ig->w_inconclusive), "verdict", GINT_TO_POINTER(VERDICT_INCONCLUSIVE));
  gtk_signal_connect(GTK_OBJECT(ig->w_inconclusive),
		     "clicked", (GtkSignalFunc) input_gui_button_clicked, ig);

  return ig;
}


void input_gui_destroy(input_gui_t *ig)
{
  if ( ig == NULL )
    return;

  free(ig);
}
