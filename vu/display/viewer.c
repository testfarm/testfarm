/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* Remote Frame Buffer display - Image viewer                               */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 14-JAN-2004                                                    */
/****************************************************************************/

/*
 * $Revision: 1014 $
 * $Date: 2008-08-13 15:36:56 +0200 (mer., 13 août 2008) $
 */

#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <gtk/gtk.h>

#include "image_file.h"
#include "color.h"
#include "interface.h"
#include "support.h"
#include "utils.h"
#include "viewer.h"

#undef USE_GTK_IMAGE

#define VIEWER_BPP 3

static guint viewer_timeout_tag = 0;
static char *viewer_fname = NULL;

static GtkToggleButton *viewer_enable = NULL;
#ifdef USE_GTK_IMAGE
static GtkImage *viewer_image = NULL;
#else
static GtkDrawingArea *viewer_image = NULL;
static unsigned int viewer_width = 0;
static unsigned int viewer_height = 0;
#endif
static unsigned char *viewer_buf = NULL;


#ifndef USE_GTK_IMAGE
static gboolean viewer_expose(GtkWidget *widget, GdkEventExpose *event)
{
  unsigned int x, y, w, h;
  unsigned int rowstride;
  unsigned int ofs;

  if ( viewer_image == NULL )
    return TRUE;
  if ( viewer_buf == NULL )
    return TRUE;
  if ( ! gtk_toggle_button_get_active(viewer_enable) )
    return TRUE;

  x = event->area.x;
  y = event->area.y;
  w = event->area.width;
  h = event->area.height;
  //fprintf(stderr, "-- VIEWER EXPOSE x=%u y=%u w=%u h=%u\n", x, y, w, h);

  /* Clip refresh area to actual RGB buffer */
  if ( (x + w) > viewer_width )
    w = viewer_width - x;
  if ( (y + h) > viewer_height )
    h = viewer_height - y;

  /* Compute buffer offset */
  rowstride = VIEWER_BPP * viewer_width;
  ofs = (rowstride * y) + (VIEWER_BPP * x);

  //fprintf(stderr, "   => x=%u y=%u w=%u h=%u ofs=%u\n", x, y, w, h, ofs);

  gdk_draw_rgb_image(GTK_WIDGET(viewer_image)->window, GTK_WIDGET(viewer_image)->style->fg_gc[GTK_STATE_NORMAL],
		     x, y, w, h,
                     GDK_RGB_DITHER_NONE, viewer_buf + ofs, rowstride);

  return TRUE;
}
#endif


static void viewer_free(void)
{
  if ( viewer_timeout_tag > 0 ) {
    g_source_remove(viewer_timeout_tag);
    viewer_timeout_tag = 0;
  }

  if ( viewer_fname != NULL ) {
    free(viewer_fname);
    viewer_fname = NULL;
  }

  viewer_width = 0;
  viewer_height = 0;
}


static void viewer_load(char *fname)
{
#ifdef USE_GTK_IMAGE
  gtk_image_set_from_file(viewer_image, fname);
#else
  if ( viewer_buf != NULL ) {
    free(viewer_buf);
    viewer_buf = NULL;
  }

  if ( fname != NULL ) {
    int ret;

    if ( fname[0] == '#' ) {
      ret = color_fill_from_spec(fname, &viewer_width, &viewer_height, &viewer_buf);
    }
    else {
      ret = image_load(fname, &viewer_width, &viewer_height, &viewer_buf, NULL, NULL);
    }

    if ( ret == 0 )
      gtk_drawing_area_size(viewer_image, viewer_width, viewer_height);
  }
#endif
}


static void viewer_undraw(void)
{
#ifdef USE_GTK_IMAGE
  gtk_image_clear(viewer_image);
#else
  gtk_drawing_area_size(viewer_image, 0, 0);
#endif
}


static void viewer_draw(void)
{
  if ( viewer_fname == NULL ) {
    viewer_undraw();
  }
  else {
    viewer_load(viewer_fname);
  }
}


static gboolean viewer_timeout(void)
{
	viewer_timeout_tag = 0;

	if (viewer_image != NULL) {
		viewer_draw();
	}

	return FALSE;
}


int viewer_show(char *fname)
{
  int active;

  viewer_free();

  if ( viewer_image == NULL )
    return 0;

  if ( (fname != NULL) && (fname[0] != '\0') )
    viewer_fname = strdup(fname);

  active = gtk_toggle_button_get_active(viewer_enable);

  if ( active ) {
    if ( viewer_timeout_tag > 0 )
      g_source_remove(viewer_timeout_tag);
    viewer_timeout_tag = g_timeout_add(500, (GSourceFunc) viewer_timeout, NULL);
  }

  return 0;
}


static void viewer_enable_toggled(void)
{
  int active = gtk_toggle_button_get_active(viewer_enable);

  if ( active )
    viewer_draw();
  else
    viewer_undraw();
}


int viewer_init(GtkWindow *window)
{
  viewer_fname = NULL;
  viewer_buf = NULL;

  viewer_enable = GTK_TOGGLE_BUTTON(lookup_widget(GTK_WIDGET(window), "viewer_enable"));
  gtk_signal_connect_object(GTK_OBJECT(viewer_enable), "toggled",
			    GTK_SIGNAL_FUNC(viewer_enable_toggled), NULL);

#ifdef USE_GTK_IMAGE
  viewer_image = GTK_IMAGE(lookup_widget(GTK_WIDGET(window), "viewer_image"));
#else
  viewer_image = GTK_DRAWING_AREA(lookup_widget(GTK_WIDGET(window), "viewer_drawingarea"));
  gtk_signal_connect(GTK_OBJECT(viewer_image), "expose_event",
		     GTK_SIGNAL_FUNC(viewer_expose), NULL);
#endif

  viewer_enable_toggled();

  return 0;
}


void viewer_done(void)
{
  viewer_load(NULL);
  viewer_free();
  viewer_image = NULL;
}
