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
