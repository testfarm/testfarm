/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* Mouse cursor display                                                     */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 03-APR-2007                                                    */
/****************************************************************************/

/*
 * $Revision: 451 $
 * $Date: 2007-04-05 11:21:29 +0200 (jeu., 05 avril 2007) $
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
