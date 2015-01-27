/****************************************************************************/
/* TestFarm                                                                 */
/* Standard Window Icon                                                     */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 10-JUN-20040                                                    */
/****************************************************************************/

/*
 * $Revision: 42 $
 * $Date: 2006-06-03 15:30:01 +0200 (sam., 03 juin 2006) $
 */

#include <stdio.h>
#include <string.h>
#include <gtk/gtk.h>

#include "install.h"

void set_window_icon(GtkWindow *window)
{
  char *home = get_home();
  int len = strlen(home);
  char pixmap[len+32];
  GdkPixbuf *pixbuf;

  strcpy(pixmap, home);
  strcpy(pixmap+len, "/icons/testfarm.png");

  pixbuf = gdk_pixbuf_new_from_file(pixmap, NULL);
  if ( pixbuf != NULL ) {
    gtk_window_set_icon(window, pixbuf);
    gdk_pixbuf_unref(pixbuf);
  }
}
