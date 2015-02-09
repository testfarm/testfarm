/*
 * $Revision: 749 $
 * $Date: 2007-09-23 14:17:06 +0200 (dim., 23 sept. 2007) $
 */

#include <stdio.h>
#include <gtk/gtk.h>

#include "support.h"
#include "window_icon.h"


void set_window_icon(GtkWindow *window)
{
  GdkPixbuf *pixbuf;

  pixbuf = create_pixbuf("testfarm-vu.png");
  if ( pixbuf != NULL ) {
    gtk_window_set_icon(window, pixbuf);
    gdk_pixbuf_unref(pixbuf);
  }
}
