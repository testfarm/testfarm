/****************************************************************************/
/* TestFarm                                                                 */
/* Graphical User Interface : File validation user interface                */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 19-APR-2004                                                    */
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
#include <stdarg.h>
#include <string.h>
#include <gtk/gtk.h>

#include "interface.h"
#include "support.h"
#include "error_gui.h"


void eprintf(char *fmt, ...)
{
  GtkWidget *window;
  GtkLabel *label;
  va_list ap;
  static char str[1024];

  window = create_error_window();
  gtk_widget_show(window);

  gtk_signal_connect_object(GTK_OBJECT(lookup_widget(window, "ok")), "clicked",
                            GTK_SIGNAL_FUNC(gtk_widget_destroy), (gpointer) window);

  label = GTK_LABEL(lookup_widget(GTK_WIDGET(window), "label"));

  va_start(ap, fmt);
  vsnprintf(str, sizeof(str), fmt, ap);
  va_end(ap);

  gtk_label_set_text(label, str);
}
