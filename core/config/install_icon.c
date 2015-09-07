/****************************************************************************/
/* TestFarm                                                                 */
/* Standard Window Icon                                                     */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 10-JUN-20040                                                    */
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
