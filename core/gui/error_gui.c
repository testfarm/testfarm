/****************************************************************************/
/* TestFarm                                                                 */
/* Graphical User Interface : File validation user interface                */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 19-APR-2004                                                    */
/****************************************************************************/

/*
 * $Revision: 42 $
 * $Date: 2006-06-03 15:30:01 +0200 (sam., 03 juin 2006) $
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
