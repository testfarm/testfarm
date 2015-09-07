/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* Display refresh control                                                  */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 02-APR-2007                                                    */
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
#include "frame_ctl.h"
#include "frame_ctl_msg.h"
#include "frame_geometry.h"
#include "display_refresh.h"


static GtkWidget *display_refresh_box = NULL;
static GtkToggleButton *display_refresh_enable = NULL;
static unsigned long display_refresh_enable_id = 0;
static GtkSpinButton *display_refresh_period = NULL;
static unsigned long display_refresh_period_id = 0;
static int display_refresh_timeout_tag = -1;

static GtkWidget *display_refresh_geometry = NULL;
static frame_geometry_t display_refresh_sel = FRAME_GEOMETRY_NULL;

static int display_refresh_sock = -1;


void display_refresh_set_selection(frame_geometry_t *g)
{
  int enable;

  if ( g == NULL )
    display_refresh_sel = frame_geometry_null;
  else
    display_refresh_sel = *g;

  enable = (display_refresh_sel.width > 0) && (display_refresh_sel.height > 0);
  gtk_widget_set_sensitive(GTK_WIDGET(display_refresh_geometry), enable);
}


static void display_refresh_geometry_clicked(void)
{
  char cmd[80];

  if ( display_refresh_sock < 0 )
    return;
  if ( (display_refresh_sel.width <= 0) || (display_refresh_sel.height <= 0) )
    return;

  snprintf(cmd, sizeof(cmd), "capture geometry %ux%u+%d+%d",
	   display_refresh_sel.width, display_refresh_sel.height,
	   display_refresh_sel.x, display_refresh_sel.y);

  frame_ctl_command(display_refresh_sock, cmd);
}


static void display_refresh_update(gboolean active)
{
  int period = 0;
  char cmd[32];

  if ( display_refresh_sock < 0 )
    return;

  if ( active )
    period = gtk_spin_button_get_value_as_int(display_refresh_period);
  snprintf(cmd, sizeof(cmd), "capture refresh %d", period);

  frame_ctl_command(display_refresh_sock, cmd);
}


void display_refresh_updated(unsigned int period)
{
  int active = (period > 0);

  if ( active ) {
    gtk_signal_handler_block(GTK_OBJECT(display_refresh_period), display_refresh_period_id);
    gtk_spin_button_set_value(display_refresh_period, period);
    gtk_signal_handler_unblock(GTK_OBJECT(display_refresh_period), display_refresh_period_id);
  }
  gtk_widget_set_sensitive(GTK_WIDGET(display_refresh_period), active);

  gtk_signal_handler_block(GTK_OBJECT(display_refresh_enable), display_refresh_enable_id);
  gtk_toggle_button_set_active(display_refresh_enable, active);
  gtk_signal_handler_unblock(GTK_OBJECT(display_refresh_enable), display_refresh_enable_id);
}


static void display_refresh_enable_toggled(void)
{
  display_refresh_update(gtk_toggle_button_get_active(display_refresh_enable));
}


static gboolean display_refresh_timeout(void)
{
  display_refresh_timeout_tag = -1;
  display_refresh_update(TRUE);
  return FALSE;
}


static void display_refresh_timeout_stop(void)
{
  if ( display_refresh_timeout_tag >= 0 )
    g_source_remove(display_refresh_timeout_tag);
  display_refresh_timeout_tag = -1;
}


static void display_refresh_period_changed(void)
{
  display_refresh_timeout_stop();
  display_refresh_timeout_tag = g_timeout_add(1000, (GSourceFunc) display_refresh_timeout, NULL);
}


void display_refresh_now(void)
{
  if ( display_refresh_sock < 0 )
    return;
  frame_ctl_command(display_refresh_sock, "capture refresh now");
}


int display_refresh_init(GtkWindow *window)
{
  GtkWidget *widget;

  display_refresh_box = lookup_widget(GTK_WIDGET(window), "refresh_box");

  display_refresh_enable = GTK_TOGGLE_BUTTON(lookup_widget(GTK_WIDGET(window), "refresh_enable"));
  display_refresh_enable_id = gtk_signal_connect_object(GTK_OBJECT(display_refresh_enable), "toggled",
							GTK_SIGNAL_FUNC(display_refresh_enable_toggled), NULL);

  display_refresh_period = GTK_SPIN_BUTTON(lookup_widget(GTK_WIDGET(window), "refresh_period"));
  display_refresh_period_id = gtk_signal_connect_object(GTK_OBJECT(display_refresh_period), "value_changed",
							GTK_SIGNAL_FUNC(display_refresh_period_changed), NULL);

  widget = lookup_widget(GTK_WIDGET(window), "refresh_now");
  gtk_signal_connect_object(GTK_OBJECT(widget), "clicked",
                            GTK_SIGNAL_FUNC(display_refresh_now), NULL);

  display_refresh_geometry = lookup_widget(GTK_WIDGET(window), "refresh_geometry");
  gtk_signal_connect_object(GTK_OBJECT(display_refresh_geometry), "clicked",
                            GTK_SIGNAL_FUNC(display_refresh_geometry_clicked), NULL);

  display_refresh_sock = -1;
  gtk_widget_set_sensitive(display_refresh_box, 0);

  return 0;
}


void display_refresh_done(void)
{
  display_refresh_enable_id = 0;
  display_refresh_period_id = 0;
  display_refresh_timeout_stop();
}


void display_refresh_set_ctl(int sock)
{
  display_refresh_sock = sock;

  if ( display_refresh_box != NULL )
    gtk_widget_set_sensitive(display_refresh_box, (sock >= 0));
}
