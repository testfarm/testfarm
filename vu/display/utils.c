/****************************************************************************/
/* TestFarm RFB Interface                                                   */
/* Remote Frame Buffer display - Miscellaneous utilities                    */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 14-JAN-2004                                                    */
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
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <gtk/gtk.h>

#include "interface.h"
#include "support.h"
#include "window_icon.h"
#include "utils.h"


unsigned int opt_debug = DEBUG_CONNECT;

GtkLabel *eprintf_label = NULL;


char *lprintf(GtkLabel *label, char *fmt, ...)
{
  va_list ap;
  static char str[80];

  va_start(ap, fmt);
  vsnprintf(str, sizeof(str), fmt, ap);
  va_end(ap);

  gtk_label_set_label(label, str);

  return str;
}


static GtkLabel *eprintf_window(void)
{
  GtkWidget *window = create_error_window();

  gtk_widget_show(window);
  set_window_icon(GTK_WINDOW(window));

  gtk_signal_connect_object(GTK_OBJECT(lookup_widget(window, "ok")), "clicked",
			    GTK_SIGNAL_FUNC(gtk_widget_destroy), (gpointer) window);

  return GTK_LABEL(lookup_widget(GTK_WIDGET(window), "label"));
}


void eprintf(char *fmt, ...)
{
  GtkLabel *label;
  va_list ap;
  static char str[1024];
  int len = 0;

  if ( eprintf_label == NULL )
    label = eprintf_window();
  else
    label = eprintf_label;

  if ( eprintf_label != NULL )
    len += snprintf(str+len, sizeof(str)-len, "<span foreground=\"red\">");

  va_start(ap, fmt);
  len += vsnprintf(str+len, sizeof(str)-len, fmt, ap);
  va_end(ap);

  if ( eprintf_label != NULL )
    len += snprintf(str+len, sizeof(str)-len, "</span>");

  gtk_label_set_label(label, str);
}


void eprintf2(char *fmt, ...)
{
  GtkLabel *label;
  va_list ap;
  static char str[1024];

  label = eprintf_window();

  va_start(ap, fmt);
  vsnprintf(str, sizeof(str), fmt, ap);
  va_end(ap);

  gtk_label_set_text(label, str);
}


char *strfilename(char *filename)
{
  char cwd[strlen(filename)+1];
  int cwdlen;

  getcwd(cwd, sizeof(cwd));
  cwdlen = strlen(cwd);

  if ( strncmp(filename, cwd, cwdlen) == 0 ) {
    filename += cwdlen;
    if ( *filename == '/' )
      filename++;
  }

  return filename;
}
