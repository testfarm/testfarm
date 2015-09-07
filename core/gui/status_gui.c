/****************************************************************************/
/* TestFarm                                                                 */
/* Graphical User Interface : Status Bar                                    */
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

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <gtk/gtk.h>

#include "support.h"
#include "status.h"
#include "status_gui.h"


static GtkStatusbar *statusbar = NULL;
static int statusbar_id;

void status_gui_init(GtkWidget *window)
{
  statusbar = GTK_STATUSBAR(lookup_widget(window, "main_status"));
  statusbar_id = gtk_statusbar_get_context_id(statusbar, "status");
}


void status_gui_done(void)
{
  /* Nothing */
}


void status_mesg(char *fmt, ...)
{
  va_list ap;
  char buf[256];

  va_start(ap, fmt);
  vsnprintf(buf, sizeof(buf), fmt, ap);
  va_end(ap);

  if ( statusbar != NULL ) {
    gtk_statusbar_pop(statusbar, statusbar_id);
    gtk_statusbar_push(statusbar, statusbar_id, buf);
  }
  else {
    fputs(buf, stderr);
    fputs("\n", stderr);
  }
}
