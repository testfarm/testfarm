/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* Mouse cursor display                                                     */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 03-APR-2007                                                    */
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
#include <gtk/gtk.h>

#include "support.h"
#include "display_cursor.h"


static GtkWidget *display_cursor_widget = NULL;
static GtkToggleButton *display_cursor_button = NULL;

static GdkCursor *display_cursor_blank = NULL;
static GdkCursor *display_cursor_cross = NULL;

static int display_cursor_force = 1;


void display_cursor_show(int visible)
{
  GdkCursor *cursor;

  if ( visible >= 0 )
    display_cursor_force = visible;

  cursor = gtk_toggle_button_get_active(display_cursor_button) || display_cursor_force ?
    display_cursor_cross :
    display_cursor_blank;
  gdk_window_set_cursor(display_cursor_widget->window, cursor);
}


static void display_cursor_button_toggled(void)
{
  display_cursor_show(-1);
}


void display_cursor_init(GtkWindow *window, GtkWidget *widget)
{
  gchar bits[] = {0};
  GdkColor  color = {0, 0, 0, 0};
  GdkPixmap *pixmap = gdk_bitmap_create_from_data(NULL, bits, 1, 1);
  display_cursor_blank = gdk_cursor_new_from_pixmap(pixmap, pixmap, &color, &color, 0, 0);

  display_cursor_cross = gdk_cursor_new(GDK_CROSS);

  display_cursor_button = GTK_TOGGLE_BUTTON(lookup_widget(GTK_WIDGET(window), "record_cursor"));
  gtk_signal_connect_object(GTK_OBJECT(display_cursor_button), "toggled",
			    GTK_SIGNAL_FUNC(display_cursor_button_toggled), NULL);

  display_cursor_widget = widget;
  gdk_window_set_cursor(widget->window, display_cursor_cross);
}
