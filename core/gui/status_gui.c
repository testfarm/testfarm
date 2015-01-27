/****************************************************************************/
/* TestFarm                                                                 */
/* Graphical User Interface : Status Bar                                    */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 10-MAY-2000                                                    */
/****************************************************************************/

/*
 * $Revision: 770 $
 * $Date: 2007-10-09 14:34:18 +0200 (mar., 09 oct. 2007) $
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
