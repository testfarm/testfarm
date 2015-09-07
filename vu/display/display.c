/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* Frame Buffer display tool                                                */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 05-JAN-2004                                                    */
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
#include <malloc.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>

#include "support.h"

#include "frame_ctl.h"
#include "frame_ctl_msg.h"
#include "scroll.h"
#include "utils.h"
#include "display.h"
#include "display_desktop.h"
#include "display_pattern.h"
#include "display_refresh.h"
#include "display_screenshot.h"
#include "display_record.h"
#include "display_cursor.h"
#include "display_sel.h"
#include "display_pad.h"
#include "display_command.h"


static GtkWindow *display_window = NULL;

static GtkToggleToolButton *display_button_input = NULL;
static GtkToolButton *display_button_pattern = NULL;

static GtkLabel *display_label_mouse = NULL;
static GtkLabel *display_label_pixel = NULL;
static GtkLabel *display_label_selection = NULL;

static GtkDrawingArea *display_darea = NULL;
static int display_darea_scroll = -1;

static int display_input_enabled = 0;
static int display_input_focus = 0;


/*======================================================================*/
/* Frame display                                                        */
/*======================================================================*/

static void display_title(char *id)
{
  char str[80];

  sprintf(str, "TestFarm Virtual User Display: %s", id);
  gtk_window_set_title(display_window, str);
}


static void display_redraw(display_t *d, unsigned int x, unsigned int y, unsigned int w, unsigned int h)
{
  unsigned int rowstride;
  unsigned int ofs;

  if ( display_darea == NULL )
    return;
  if ( d->rgb == NULL )
    return;

  debug(DEBUG_REFRESH, "REDRAW %ux%u+%d+%d\n", w, h, x, y);

  /* Clip refresh area to actual RGB buffer */ 
  if ( (x + w) > d->rgb_width )
    w = d->rgb_width - x;
  if ( (y + h) > d->rgb_height )
    h = d->rgb_height - y;

  /* Compute buffer offset */
  rowstride = DISPLAY_BPP * d->rgb_width;
  ofs = (rowstride * y) + (DISPLAY_BPP * x);

  gdk_draw_rgb_image(GTK_WIDGET(display_darea)->window, GTK_WIDGET(display_darea)->style->fg_gc[GTK_STATE_NORMAL],
                     x, y, w, h,
                     GDK_RGB_DITHER_NONE, d->rgb + ofs, rowstride);
}


static void display_redraw_all(display_t *d)
{
  display_redraw(d, 0, 0, d->rgb_width, d->rgb_height);
}


static void display_update(display_t *d, frame_geometry_t *g)
{
  frame_geometry_t gg = d->desktop.root.g0;
  unsigned int width0 = d->desktop.root.g0.width;
  unsigned int height0 = d->desktop.root.g0.height;
  int ctl_x1, ctl_y1, ctl_x2, ctl_y2;
  unsigned int rowstride;
  unsigned long yoffset;
  unsigned char *ytarget;
  unsigned char *ymap;
  int xi, yi;

  if ( d->desktop.root.fb == NULL )
    return;
  if ( d->rgb == NULL )
    return;

  if ( g != NULL ) {
    gg = *g;

    /* Paranoia: Clip update area to actual RGB buffer */ 
    if ( (gg.x + gg.width) > width0 )
      gg.width = width0 - gg.x;
    if ( (gg.y + gg.height) > height0 )
      gg.height = height0 - gg.y;
  }

  debug(DEBUG_REFRESH, "UPDATE %s\n", frame_geometry_str(&gg));

  ctl_x1 = d->rgb_window.x - gg.x;
  ctl_y1 = d->rgb_window.y - gg.y;
  ctl_x2 = ctl_x1 + d->rgb_window.width;
  ctl_y2 = ctl_y1 + d->rgb_window.height;

  rowstride = DISPLAY_BPP * width0;
  yoffset = (rowstride * gg.y) + (DISPLAY_BPP * gg.x);
  ytarget = d->rgb + yoffset;
  ymap = d->desktop.layout + (width0 * gg.y) + gg.x;

  for (yi = 0; yi < gg.height; yi++) {
    unsigned int yy = gg.y + yi;
    unsigned char *xtarget = ytarget;
    unsigned char *xmap = ymap;

    int ctl_ok = (yi >= ctl_y1) && (yi < ctl_y2);

    for (xi = 0; xi < gg.width; xi++) {
      unsigned int xx = gg.x + xi;
      unsigned char n = *xmap;
      frame_hdr_t *frame = d->desktop.layers[n & ~0xC0];
      long offset = DISPLAY_BPP * ((frame->g0.width * (yy - frame->g0.y)) + (xx - frame->g0.x));
      unsigned char *xsource = frame->fb->rgb.buf + offset;

      if ( ctl_ok && (xi >= ctl_x1) && (xi < ctl_x2) ) {
        xtarget[0] = xsource[0];
        xtarget[1] = xsource[1];
        xtarget[2] = xsource[2];
      }
      else {
        xtarget[0] = xsource[0] / 2;
        xtarget[1] = xsource[1] / 2;
        xtarget[2] = xsource[2] / 2;
      }

      if ( n & 0x80 ) {
	xtarget[0] = ((((unsigned int) xtarget[0]) * 7) / 10) + (display_desktop_color[0] * 3 / 10);
	xtarget[1] = ((((unsigned int) xtarget[1]) * 7) / 10) + (display_desktop_color[1] * 3 / 10);
	xtarget[2] = ((((unsigned int) xtarget[2]) * 7) / 10) + (display_desktop_color[2] * 3 / 10);
      }

      xtarget += DISPLAY_BPP;
      xmap++;
    }

    ytarget += rowstride;
    ymap += width0;
  }

  /* Show active pads */
  display_pad_render(&d->pad, &gg);
}


void display_refresh(display_t *d, frame_geometry_t *g)
{
  if ( d == NULL )
    return;
  display_update(d, g);
  display_redraw(d, g->x, g->y, g->width, g->height);
}


static void display_jam(display_t *d)
{
  frame_buf_t *fb = d->desktop.root.fb;
  unsigned int w = d->desktop.root.g0.width;
  unsigned int h = d->desktop.root.g0.height;
  unsigned char *source, *target;
  int xi, yi;

  if ( display_darea == NULL )
    return;
  if ( fb == NULL )
    return;
  if ( d->rgb == NULL )
    return;

  source = d->desktop.root.fb->rgb.buf;
  target = d->rgb;

  for (yi = 0; yi < h; yi++) {
    for (xi = 0; xi < w; xi++) {
      if ( (xi % 8) && (yi % 8) ) {
        target[0] = source[0] / 2;
        target[1] = source[1] / 2;
        target[2] = source[2] / 2;
      }
      else {
        target[0] = 0;
        target[1] = 0;
        target[2] = 0;
      }
      source += DISPLAY_BPP;
      target += DISPLAY_BPP;
    }
  }

  /* Render jammed display if currently selected */
  if ( d == display_current )
    display_redraw_all(d);
}


static void display_selection_hide(display_t *d)
{
  if ( d->sel_window.width > 0 ) {
    lprintf(display_label_selection, "");
    display_sel_undraw(d, &(d->sel_window));
    display_pattern_show_selection();
  }
}


void display_selection_show(display_t *d)
{
  if ( d == NULL )
    return;

  if ( d->sel_window.width > 0 ) {
    frame_hdr_t *frame = display_desktop_get_current_frame();
    char str[64] = "";

    if ( frame != &(d->desktop.root) ) {
      frame_geometry_t g;

      if ( frame_geometry_intersect(&(d->sel_window), &(frame->g0), &g) ) {
	g.x -= frame->g0.x;
	g.y -= frame->g0.y;

	snprintf(str, sizeof(str), " <span foreground=\"" DISPLAY_DESKTOP_COLOR_S "\">(%s)</span>", frame_geometry_str(&g));
      }
    }

    lprintf(display_label_selection, "%s%s", frame_geometry_str(&(d->sel_window)), str);
    display_sel_draw(DISPLAY_SEL_COLOR_BLUE, &(d->sel_window));
  }
}


static void display_selection_update(display_t *d, int x, int y)
{
  display_selection_hide(d);

  /* Clip selection to RGB area */
  if ( x < 0 )
    x = 0;
  if ( x >= d->desktop.root.g0.width )
    x = d->desktop.root.g0.width - 1;
  if ( y < 0 )
    y = 0;
  if ( y >= d->desktop.root.g0.height )
    y = d->desktop.root.g0.height - 1;

  if ( d->sel_x <= x ) {
    d->sel_window.x = d->sel_x;
    d->sel_window.width = x - d->sel_x + 1;
  }
  else {
    d->sel_window.x = x;
    d->sel_window.width = d->sel_x - x + 1;
  }

  if ( d->sel_y <= y ) {
    d->sel_window.y = d->sel_y;
    d->sel_window.height = y - d->sel_y + 1;
  }
  else {
    d->sel_window.y = y;
    d->sel_window.height = d->sel_y - y + 1;
  }

  if ( (d->sel_window.width < 2) && (d->sel_window.height < 2) ) {
    d->sel_window.width = 0;
    d->sel_window.height = 0;
  }

  display_selection_show(d);
  display_pattern_set_selection(&(d->sel_window));
  display_refresh_set_selection(&(d->sel_window));
  display_screenshot_set_selection(&(d->sel_window));
}


static gboolean display_event_expose(GtkWidget *widget, GdkEventExpose *event)
{
  display_t *d = display_current;
  if ( d == NULL )
    return TRUE;

  debug(DEBUG_GTK, "EXPOSE x=%u y=%u w=%u h=%u\n", event->area.x, event->area.y, event->area.width, event->area.height);

  display_redraw(d, event->area.x, event->area.y, event->area.width, event->area.height);
  display_selection_show(d);
  display_pattern_show_selection();

  return TRUE;
}


static void display_mouse(int x, int y, unsigned long time)
{
  display_t *d = display_current;
  frame_buf_t *fb = NULL;

  display_input_focus = (d != NULL) && (x >= 0) && (y >= 0);

  if ( display_input_focus ) {
    fb = d->desktop.root.fb;
    if ( (fb == NULL) || (x >= fb->rgb.width) || (y >= fb->rgb.height) )
      display_input_focus = 0;
  }

  /* Show mouse cursor position */
  if ( display_input_focus ) {
    frame_hdr_t *frame = display_desktop_get_current_frame();
    char str[64] = "";

    if ( frame != &(d->desktop.root) ) {
      int xx = x - frame->g0.x;
      int yy = y - frame->g0.y;

      if ( (xx >= 0) && (xx < frame->g0.width) &&
	   (yy >= 0) && (yy < frame->g0.height) ) {
	snprintf(str, sizeof(str), " <span foreground=\"" DISPLAY_DESKTOP_COLOR_S "\">(%+d%+d)</span>", xx, yy);
      }
    }

    lprintf(display_label_mouse, "+%d+%d%s", x, y, str);
  }
  else {
    lprintf(display_label_mouse, "");
  }

  /* Show pixel value at mouse cursor position */
  if ( display_input_focus ) {
    unsigned char *pix = fb->rgb.buf + (y * fb->rgb.rowstride) + (x * fb->rgb.bpp);
    lprintf(display_label_pixel, "#%02X%02X%02X", pix[0], pix[1], pix[2]);
  }
  else {
    lprintf(display_label_pixel, "");
  }

  if ( display_input_enabled && display_input_focus )
    display_record_pointer_position(x, y, time);
}


static gboolean display_event_crossing(GtkWidget *widget, GdkEventCrossing *event)
{
  int x = (int) event->x;
  int y = (int) event->y;

  if ( display_current == NULL )
    return TRUE;

  debug(DEBUG_GTK, "CROSSING: type=%d x=%d y=%d focus=%d\n", event->type, x, y, event->focus);
  if ( event->type == GDK_LEAVE_NOTIFY ) {
    display_mouse(-1, -1, event->time);
  }
  else {
    gtk_widget_grab_focus(GTK_WIDGET(display_darea));
    display_mouse(x, y, event->time);
  }

  return TRUE;
}


static gboolean display_event_motion(GtkWidget *widget, GdkEventMotion *event)
{
  display_t *d = display_current;
  int x = (int) event->x;
  int y = (int) event->y;

  if ( d == NULL )
    return TRUE;

  debug(DEBUG_GTK, "MOTION: type=%d x=%d y=%d\n", event->type, x, y);
  display_mouse(x, y, event->time);

  if ( ! display_input_enabled ) {
    if ( (d->sel_x >= 0) && (d->sel_y >= 0) ) {
      display_selection_update(d, event->x, event->y); 
    }
  }

  return TRUE;
}


static gboolean display_event_button(GtkWidget *widget, GdkEventButton *event)
{
  display_t *d = display_current;
  if ( d == NULL )
    return TRUE;

  if ( display_input_enabled && display_input_focus ) {
    display_record_pointer_button(event->button, (event->type == GDK_BUTTON_PRESS), event->time);
  }
  else {
    if ( event->type == GDK_BUTTON_PRESS ) {
      debug(DEBUG_GTK, "BUTTON PRESS: button=%d x=%d y=%d\n", event->button, (int) event->x, (int) event->y);
      if ( event->button == 1 ) {
	if ( (event->x >= 0) && (event->y >= 0) ) {
	  display_selection_hide(d);

	  d->sel_window.width = 0;
	  d->sel_window.height = 0;

	  d->sel_x = event->x;
	  if ( d->sel_x >= d->desktop.root.g0.width )
	    d->sel_x = d->desktop.root.g0.width - 1;

	  d->sel_y = event->y;
	  if ( d->sel_y >= d->desktop.root.g0.height )
	    d->sel_y = d->desktop.root.g0.height - 1;
	}
      }
    }
    else if ( event->type == GDK_BUTTON_RELEASE ) {
      debug(DEBUG_GTK, "BUTTON RELEASE: button=%d x=%d y=%d\n", event->button, (int) event->x, (int) event->y);
      if ( event->button == 1 ) {
	display_selection_update(d, event->x, event->y); 

	d->sel_x = -1;
	d->sel_y = -1;
      }
      else if ( event->button == 3 ) {
      }
    }
  }

  return TRUE;
}


static gboolean display_event_scroll(GtkWidget *widget, GdkEventScroll *event)
{
  unsigned char direction = 0;

  if ( display_input_enabled && display_input_focus ) {
    switch ( event->direction ) {
    case GDK_SCROLL_UP:
      direction = SCROLL_UP;
      break;
    case GDK_SCROLL_DOWN:
      direction = SCROLL_DOWN;
      break;
    case GDK_SCROLL_LEFT:
      direction = SCROLL_LEFT;
      break;
    case GDK_SCROLL_RIGHT:
      direction = SCROLL_RIGHT;
      break;
    default:
      break;
    }

    if ( direction )
      display_record_pointer_scroll(direction);
  }

  return TRUE;
}


static gboolean display_event_key(GtkWidget *widget, GdkEventKey *event)
{
  debug(DEBUG_GTK, "KEY: type=%d keyval=%u\n", event->type, event->keyval);

  if ( event->type == GDK_KEY_RELEASE ) {
    int abort = 1;

    switch ( event->keyval ) {
    case GDK_F1:
      display_input_enabled = ! display_input_enabled;
      gtk_toggle_tool_button_set_active(display_button_input, display_input_enabled);
      break;
    case GDK_F2:
      gtk_signal_emit_by_name(GTK_OBJECT(display_button_pattern), "clicked");
      break;
    case GDK_F3:
      display_refresh_now();
      break;
    case GDK_F4:
    case GDK_F5:
    case GDK_F6:
    case GDK_F7:
      display_record_fkey(event->keyval);
      break;
    default:
      abort = 0;
      break;
    }

    if ( abort )
      return FALSE;
  }

  if ( display_input_enabled  && display_input_focus ) {
    display_record_key(event->keyval, (event->type == GDK_KEY_PRESS));
  }

  return TRUE;
}


static void display_input_clicked(void)
{
  display_input_enabled = gtk_toggle_tool_button_get_active(display_button_input);
  if ( display_input_enabled ) {
    display_darea_scroll = gtk_signal_connect(GTK_OBJECT(display_darea), "scroll_event",
					      GTK_SIGNAL_FUNC(display_event_scroll), NULL);
    display_cursor_show(0);
  }
  else {
    if ( display_darea_scroll >= 0 )
      gtk_signal_disconnect(GTK_OBJECT(display_darea), display_darea_scroll);
    display_darea_scroll = -1;
    display_cursor_show(1);
  }
}


void display_setup(display_t *d)
{
  display_title(d->desktop.name);
  gtk_drawing_area_size(display_darea, d->rgb_width+DISPLAY_SEL_BORDER, d->rgb_height+DISPLAY_SEL_BORDER);
  display_sel_clip(d);
  display_redraw_all(d);
}


static int display_connected(display_t *d, frame_ctl_rsp_init *init)
{
  int size;

  /* Hook root frame buffer */
  debug(DEBUG_CONNECT, "CONNECTED: name='%s' shmid=%d cwd='%s'\n", d->desktop.name, init->shmid, init->cwd);
  if ( display_desktop_init_root(&(d->desktop), init->shmid) ) {
    error("connect: failed to map root frame buffer (shmid=%d)\n", init->shmid);
    return -1;
  }

  /* Set capture device capabilities */
  d->cap = init->cap;

  /* Chdir to client's cwd */
  if ( init->cwd[0] != '\0' ) {
    if ( chdir(init->cwd) == -1 ) {
      error("chdir('%s'): %s\n", init->cwd, strerror(errno));
    }
  }

  /* Alloc display buffers */
  d->rgb_width = d->desktop.root.g0.width;
  d->rgb_height = d->desktop.root.g0.height;
  size = DISPLAY_BPP * d->rgb_width * d->rgb_height;
  debug(DEBUG_CONNECT, "RGB: name='%s' %ux%d, %d bytes\n", d->desktop.name, d->rgb_width, d->rgb_height, size);
  if ( d->rgb != NULL )
    free(d->rgb);
  d->rgb = malloc(size);

  /* Set active window geometry to physical display */
  d->rgb_window = d->desktop.root.g0;

  /* Clear matching indicator */
  display_pad_setup(&d->pad, d->rgb, d->rgb_width, d->rgb_height);

  /* Update frame display */
  display_update(d, NULL);

  /* Show connection status */
  display_desktop_show(d, DISPLAY_DESKTOP_CONNECTED);

  /* Setup drawing area size */
  display_setup(d);

  /* Setup and clear pattern management */
  display_pattern_set_ctl(d->sock);
  display_pattern_set_selection(&(d->sel_window));
  display_pattern_clear();

  /* Setup refresh control */
  display_refresh_set_ctl(d->sock);
  display_refresh_set_selection(&(d->sel_window));

  /* Setup screenshot management */
  display_screenshot_set_dir();
  display_screenshot_set_selection(&(d->sel_window));

  /* Setup input control */
  gtk_toggle_tool_button_set_active(display_button_input, 0);
  gtk_widget_set_sensitive(GTK_WIDGET(display_button_input), (d->cap & CAPTURE_CAP_INPUT));
  display_input_clicked();

  display_record_set_ctl(d->sock, d->cap);

  /* Enable manual command entry */
  display_command_connect(d->sock);

  return 0; 
}


static int display_ctl_read(display_t *d, int fd, void *buf, int size)
{
  int ret = read(fd, buf, size);

  /* Check for I/O error */
  if ( ret < 0 ) {
    if ( (errno == ECONNRESET) || (errno == ENETRESET) || (errno == ECONNABORTED) )
      ret = 0;
    else
      error("read(%s): %s\n", d->sockname, strerror(errno));
  }

  /* Check for disconnect */
  if ( ret == 0 ) {
    debug(DEBUG_CONNECT, "READ: '%s' Disconnected\n", d->sockname);
    display_desktop_available(d, 0);
  }

  return ret;
}


static int display_ctl_header(display_t *d, int fd)
{
  frame_ctl_hdr *hdr = (frame_ctl_hdr *) d->rsp_buf;
  int size;
  int dsize = 0;

  /* Read message header */
  size = display_ctl_read(d, fd, hdr, sizeof(frame_ctl_hdr));
  if ( size <= 0 )
    return size;

  debug(DEBUG_CTL, "READ HEADER [%d] ctl=%02X len=%d\n", size, hdr->ctl, hdr->len);

  if ( size != sizeof(frame_ctl_hdr) ) {
    error("Wrong control message header length: %d (expecting %u)\n", size, sizeof(frame_ctl_hdr));
    return -1;
  }

  switch ( hdr->ctl ) {
  case FRAME_CTL_INIT:
    dsize = sizeof(frame_ctl_rsp_init) - 1;
    break;
  case FRAME_CTL_WINDOW:
    dsize = sizeof(frame_ctl_rsp_window) - 1;
    break;
  case FRAME_CTL_REFRESH:
    dsize = sizeof(frame_ctl_rsp_refresh);
    break;
  case FRAME_CTL_PATTERN:
    dsize = sizeof(frame_ctl_rsp_pattern);
    break;
  case FRAME_CTL_MATCH:
    dsize = sizeof(frame_ctl_rsp_match);
    break;
  case FRAME_CTL_PERIOD:
    dsize = sizeof(frame_ctl_rsp_period);
    break;
  case FRAME_CTL_SOURCE:
    dsize = sizeof(frame_ctl_rsp_source);
    break;
  case FRAME_CTL_PAD:
    dsize = sizeof(frame_ctl_rsp_pad);
    break;
  case FRAME_CTL_FRAME:
    dsize = sizeof(frame_ctl_rsp_frame);
    break;
  case FRAME_CTL_HISTORY:
    dsize = sizeof(frame_ctl_rsp_history);
    break;
  default:
    error("Unknown control message type: ctl=%02X\n", hdr->ctl);
    return -1;
  }

  if ( hdr->len < dsize ) {
    error("Illegal control message body length: ctl=%02X len=%d (expecting more than %d)\n", hdr->ctl, hdr->len, dsize);
    return -1;
  }

  /* Grow rsp buffer as needed */
  if ( d->rsp_size < hdr->len ) {
    debug(DEBUG_CTL, "READ BUFFER [%d] -> [%d]\n", d->rsp_size, hdr->len);
    d->rsp_size = hdr->len;
    d->rsp_buf = realloc(d->rsp_buf, d->rsp_size);
  }

  d->rsp_idx = size;

  return 0;
}


static int display_ctl_data(display_t *d, int fd)
{
  frame_ctl_hdr *hdr = (frame_ctl_hdr *) d->rsp_buf;
  unsigned char *dbuf = d->rsp_buf + d->rsp_idx;
  int dsize = hdr->len - d->rsp_idx;
  int size;

  /* Ensure a client is connected */
  if ( (hdr->ctl != FRAME_CTL_INIT) && (d->desktop.root.fb == NULL) ) {
    error("Control message received before initialization\n");
    return -1;
  }

  /* Read message body */
  size = display_ctl_read(d, fd, dbuf, dsize);
  if ( size <= 0 )
    return size;

  debug(DEBUG_CTL, "READ DATA [%d] ctl=%02X len=%d idx=%d\n", size, hdr->ctl, hdr->len, d->rsp_idx);

  d->rsp_idx += size;

  return 0;
}


static int display_ctl_process(display_t *d)
{
  frame_ctl_rsp *rsp = (frame_ctl_rsp *) d->rsp_buf;
  frame_ctl_hdr *hdr = (frame_ctl_hdr *) d->rsp_buf;
  int ret = 0;

  debug(DEBUG_CTL, "READ PROCESS ctl=%02X\n", hdr->ctl);

  switch ( hdr->ctl ) {
  case FRAME_CTL_INIT:
    if ( display_connected(d, &(rsp->init)) ) {
      display_desktop_available(d, 0);
      ret = 1;
    }
    break;
  case FRAME_CTL_WINDOW:  /* Display active geometry updated */
    d->rgb_window = rsp->window.g;
    display_update(d, NULL);
    display_redraw_all(d);
    display_pattern_show_selection();
    display_selection_show(d);
    break;
  case FRAME_CTL_REFRESH:  /* Display refresh requested */
    {
      /* Paranoia: Reject unacceptable refresh window geometry */
      if ( (rsp->refresh.g.x < d->desktop.root.g0.width) && (rsp->refresh.g.y < d->desktop.root.g0.height) ) {
	/* Paranoia: Clip refresh window geometry into physical display */
	unsigned int max;

	max = d->desktop.root.g0.width - rsp->refresh.g.x;
	if ( rsp->refresh.g.width > max )
	  rsp->refresh.g.width = max;

	max = d->desktop.root.g0.height - rsp->refresh.g.y;
	if ( rsp->refresh.g.height > max )
	  rsp->refresh.g.height = max;

	display_refresh(d, &(rsp->refresh.g));
	display_pattern_show_selection();
	display_selection_show(d);
      }
      else {
        error("Got a Refresh control message with illegal geometry\n");
      }
    }
    break;
  case FRAME_CTL_PATTERN:  /* Pattern declaration added */
    {
      char *id = rsp->pattern.id_source;
      char *source = id + strlen(id) + 1;
      frame_hdr_t *frame = NULL;

      if ( rsp->pattern.shmid > 0 ) {
	frame = display_desktop_get_frame(rsp->pattern.shmid);
	if ( frame == NULL ) {
	  error("Cannot retrieve frame shmid=%d required by pattern %s\n", rsp->pattern.shmid, id);
	}
      }

      display_pattern_update(id, frame, source, &(rsp->pattern.g), rsp->pattern.mode, rsp->pattern.type,
			     rsp->pattern.fuzz, rsp->pattern.loss);
    }
    break;
  case FRAME_CTL_MATCH:  /* Pattern matched */
    {
      char *id = rsp->match.id;
      display_pattern_match(d, id, rsp->match.state);
    }
    break;
  case FRAME_CTL_PERIOD:  /* Refresh period changed */
    {
      display_refresh_updated(rsp->period.period);
    }
    break;
  case FRAME_CTL_SOURCE:  /* Source playback done */
    {
      display_record_stop();
    }
    break;
  case FRAME_CTL_PAD:  /* Show pad list */
    {
      int i;

      for (i = 0; i < rsp->pad.nmemb; i++) {
	display_pad_add(&d->pad, &rsp->pad.gtab[i], display_pad_color);
      }
    }
    break;
  case FRAME_CTL_FRAME:  /* Add frame */
    {
      if ( rsp->frame.parent_shmid >= 0 )
	display_desktop_add_frame(rsp->frame.id, rsp->frame.shmid, &(rsp->frame.g0), rsp->frame.parent_shmid);
      else
	display_desktop_remove_frame(rsp->frame.shmid);
    }
    break;
  case FRAME_CTL_HISTORY:  /* Command history event */
    {
	    display_command_history(rsp->history.id, rsp->history.cmd);
    }
    break;
  }

  return ret;
}


static int display_ctl_handler(display_t *d, int fd, GdkInputCondition condition)
{
  frame_ctl_hdr *hdr = (frame_ctl_hdr *) d->rsp_buf;
  int ret = 0;

  if ( hdr->ctl == FRAME_CTL_NONE ) {
    int ret2 = display_ctl_header(d, fd);

    /* rsp_buf may have been reallocated */
    hdr = (frame_ctl_hdr *) d->rsp_buf;

    if ( ret2 ) {
      hdr->ctl = FRAME_CTL_NONE;
      return -1;
    }
  }

  if ( hdr->ctl != FRAME_CTL_NONE ) {
    if ( display_ctl_data(d, fd) ) {
      hdr->ctl = FRAME_CTL_NONE;
      return -1;
    }

    if ( d->rsp_idx >= hdr->len ) {
      ret = display_ctl_process(d);
      hdr->ctl = FRAME_CTL_NONE;
    }
  }

  return ret;
}


void display_disconnect(display_t *d)
{
  if ( d == display_current ) {
    /* Clear pattern list */
    display_pattern_clear();
    display_pattern_set_selection(NULL);
    display_pattern_set_ctl(-1);

    /* Clear refresh control */
    display_refresh_set_selection(NULL);
    display_refresh_set_ctl(-1);

    /* Clear screenshot management */
    display_screenshot_set_selection(NULL);

    /* Clear input control */
    display_record_set_ctl(-1, 0);

    /* Disable manual command entry */
    display_command_disconnect();
  }

  if ( d == NULL )
    return;

  /* Clear pad areas */
  display_pad_setup(&d->pad, NULL, 0, 0);

  /* Release frame control buffer */
  if ( d->desktop.root.fb != NULL ) {
    display_jam(d);
    display_desktop_init_root(&(d->desktop), -1);
  }

  /* Clear selection */
  d->sel_x = -1;
  d->sel_y = -1;
  d->sel_window = frame_geometry_null;

  /* Stop input event monitoring */
  if ( d->sock_tag >= 0 ) {
    gdk_input_remove(d->sock_tag);
    d->sock_tag = -1;
  }

  /* Close control connection */
  if ( d->sock >= 0 ) {
    shutdown(d->sock, 2);
    close(d->sock);
    d->sock = -1;
  }

  /* Show desktop connection status */
  display_desktop_show(d, DISPLAY_DESKTOP_DISCONNECTED);
}


int display_connect(display_t *d)
{
  struct sockaddr_un sun;
  int flags;

  if ( d->sock >= 0 )
    return 0; /* Already connected */

  /* Show desktop connection status */
  display_desktop_show(d, DISPLAY_DESKTOP_CONNECTING);

  if ( (d->sock = socket(PF_UNIX, SOCK_STREAM, 0)) == -1 ) {
    error("socket: %s\n", strerror(errno));
    return -1;
  }

  sun.sun_family = AF_UNIX;
  strcpy(sun.sun_path, d->sockname);
  if ( connect(d->sock, (struct sockaddr *) &sun, sizeof(struct sockaddr_un)) == -1 ) {
    error("connect(%s): %s\n", d->sockname, strerror(errno));
    display_disconnect(d);
    return -1;
  }

  if ( (flags = fcntl(d->sock, F_GETFL, 0)) == -1 ) {
    error("fcntl(%s,F_GETFL): %s\n", d->sockname, strerror(errno));
    display_disconnect(d);
    return -1;
  }
  if ( fcntl(d->sock, F_SETFL, O_NONBLOCK | flags) == -1 ) {
    error("fcntl(%s,F_SETFL): %s\n", d->sockname, strerror(errno));
    display_disconnect(d);
    return -1;
  }

  d->sock_tag = gdk_input_add(d->sock, GDK_INPUT_READ,
                              (GdkInputFunction) display_ctl_handler, d);

  return 0;
}


display_t *display_alloc(char *name)
{
  display_t *d = (display_t *) malloc(sizeof(display_t));
  memset(d, 0, sizeof(display_t));

  d->desktop.name = strdup(name);
  d->desktop.root.id = "(root)";
  d->desktop.root.shmid = -1;
  d->desktop.root.fb = NULL;
  d->desktop.root.g0 = frame_geometry_null;
  d->desktop.available = 0;
  d->desktop.nlayers = 0;
  d->desktop.layers = NULL;
  d->desktop.layout = NULL;

  d->sockname = frame_ctl_sockname(name);

  d->sock = -1;
  d->sock_tag = -1;
  d->rsp_size = sizeof(frame_ctl_hdr);
  d->rsp_buf = malloc(d->rsp_size);
  d->rsp_idx = 0;
  ((frame_ctl_hdr *) d->rsp_buf)->ctl = FRAME_CTL_NONE;
  d->rgb = NULL;
  d->rgb_window = frame_geometry_null;

  display_pad_init(&d->pad);

  d->sel_x = -1;
  d->sel_y = -1;
  d->sel_window = frame_geometry_null;

  return d;
}


void display_free(display_t *d)
{
  if ( d == NULL )
    return;

  display_disconnect(d);

  if ( d->rsp_buf != NULL )
    free(d->rsp_buf);

  if ( d->rgb != NULL )
    free(d->rgb);

  free(d->desktop.name);
  free(d->sockname);
  free(d);
}


int display_init(GtkWindow *window)
{
  /* Retrieve widgets */
  display_window = window;
  display_label_mouse = GTK_LABEL(lookup_widget(GTK_WIDGET(window), "label_mouse"));
  gtk_label_set_use_markup(display_label_mouse, TRUE);
  display_label_pixel = GTK_LABEL(lookup_widget(GTK_WIDGET(window), "label_pixel"));
  display_label_selection = GTK_LABEL(lookup_widget(GTK_WIDGET(window), "label_selection"));
  gtk_label_set_use_markup(display_label_selection, TRUE);

  /* Display refresh management */
  display_refresh_init(window);

  /* Pattern display management */
  display_pattern_init();
  display_button_pattern = GTK_TOOL_BUTTON(lookup_widget(GTK_WIDGET(window), "button_pattern"));
  gtk_signal_connect_object(GTK_OBJECT(display_button_pattern), "clicked",
                            GTK_SIGNAL_FUNC(display_pattern_popup), NULL);

  /* Screenshot grab management */
  display_screenshot_init(window);

  /* Connect drawing area events */
  display_darea = GTK_DRAWING_AREA(lookup_widget(GTK_WIDGET(window), "drawing_area"));
  display_darea_scroll = -1;
  gtk_signal_connect(GTK_OBJECT(display_darea), "expose_event",
                     GTK_SIGNAL_FUNC(display_event_expose), NULL);
  gtk_signal_connect(GTK_OBJECT(display_darea), "enter_notify_event",
                     GTK_SIGNAL_FUNC(display_event_crossing), NULL);
  gtk_signal_connect(GTK_OBJECT(display_darea), "leave_notify_event",
                     GTK_SIGNAL_FUNC(display_event_crossing), NULL);
  gtk_signal_connect(GTK_OBJECT(display_darea), "motion_notify_event",
                     GTK_SIGNAL_FUNC(display_event_motion), NULL);
  gtk_signal_connect(GTK_OBJECT(display_darea), "button_press_event",
                     GTK_SIGNAL_FUNC(display_event_button), NULL);
  gtk_signal_connect(GTK_OBJECT(display_darea), "button_release_event",
                     GTK_SIGNAL_FUNC(display_event_button), NULL);
  gtk_signal_connect(GTK_OBJECT(display_darea), "key_press_event",
                     GTK_SIGNAL_FUNC(display_event_key), NULL);
  gtk_signal_connect(GTK_OBJECT(display_darea), "key_release_event",
                     GTK_SIGNAL_FUNC(display_event_key), NULL);
  gtk_widget_set_events(GTK_WIDGET(display_darea),
                        GDK_EXPOSURE_MASK |
			GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK |	GDK_POINTER_MOTION_MASK |
			GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK |
			GDK_KEY_PRESS_MASK | GDK_KEY_RELEASE_MASK);

  /* Record/playback management */
  display_record_init(window);

  /* Setup selection box GC */
  gtk_widget_realize(GTK_WIDGET(display_darea));
  display_sel_init(display_darea);

  /* Init desktop selection */
  display_desktop_init(window);

  /* Setup mouse/keyboard input management */
  display_cursor_init(window, GTK_WIDGET(display_darea));

  display_input_enabled = 0;
  display_input_focus = 0;
  display_button_input = GTK_TOGGLE_TOOL_BUTTON(lookup_widget(GTK_WIDGET(window), "button_input"));
  gtk_signal_connect_object(GTK_OBJECT(display_button_input), "clicked",
                            GTK_SIGNAL_FUNC(display_input_clicked), NULL);
  gtk_toggle_tool_button_set_active(display_button_input, 0);
  gtk_widget_set_sensitive(GTK_WIDGET(display_button_input), 0);
  display_input_clicked();

  /* Manual command entry */
  display_command_init(window);

  return 0;
}


void display_done(void)
{
  display_window = NULL;
  display_darea = NULL;

  display_sel_done();
  display_screenshot_done();
  display_refresh_done();
  display_record_done();
  display_pattern_done();
  display_desktop_done();
  display_command_done();
}
