/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* Screenshot Grab Management                                               */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 03-MAR-2008                                                    */
/****************************************************************************/

/*
 * $Revision: 1130 $
 * $Date: 2010-03-31 11:48:21 +0200 (mer., 31 mars 2010) $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <gtk/gtk.h>

#include "support.h"
#include "utils.h"
#include "frame_geometry.h"
#include "png_file.h"
#include "display_screenshot.h"


#define SCREENSHOT_BASENAME "Screenshot"

static GtkWidget *display_screenshot_button = NULL;
static frame_geometry_t display_screenshot_sel = FRAME_GEOMETRY_NULL;
static frame_hdr_t *display_screenshot_frame = NULL;
static char *display_screenshot_basename = NULL;


void display_screenshot_set_dir(void)
{
  char *dirname;

  if (display_screenshot_basename != NULL)
	  free(display_screenshot_basename);

  /* Setup screenshot base name */
  dirname = getenv("TESTFARM_SCREENSHOT_DIR");
  if (dirname == NULL) {
	  GDir *dir;

	  /* If env variable not found,
	     try the default target directory 'objects/'... */
	  dirname = "objects";
	  dir = g_dir_open(dirname, 0, NULL);
	  if (dir != NULL)
		  g_dir_close(dir);
	  else
		  dirname = NULL;
  }

  if (dirname != NULL) {
	  int size = strlen(dirname) + strlen(G_DIR_SEPARATOR_S) + strlen(SCREENSHOT_BASENAME) + 1;
	  display_screenshot_basename = malloc(size);
	  snprintf(display_screenshot_basename, size, "%s" G_DIR_SEPARATOR_S SCREENSHOT_BASENAME, dirname);
  }
  else {
	  display_screenshot_basename = strdup(SCREENSHOT_BASENAME);
  }

  debug(DEBUG_CONNECT, "Screenshots stored as '%s*.png'\n", display_screenshot_basename);
}


void display_screenshot_set_selection(frame_geometry_t *g)
{
  if ( g == NULL )
    display_screenshot_sel = frame_geometry_null;
  else
    display_screenshot_sel = *g;
}


static void display_screenshot_get_selection(frame_geometry_t *g)
{
  if ( (display_screenshot_sel.width > 0) && (display_screenshot_sel.height > 0) ) {
    int ret = frame_geometry_intersect(&(display_screenshot_frame->g0), &display_screenshot_sel, g);

    if ( ret ) {
      g->x -= display_screenshot_frame->g0.x;
      g->y -= display_screenshot_frame->g0.y;
    }
    else {
      eprintf2("Please select an area in pattern frame");
    }
  }
  else {
    g->x = 0;
    g->y = 0;
    g->width = display_screenshot_frame->g0.width;
    g->height = display_screenshot_frame->g0.height;
  }
}


void display_screenshot_set_frame(frame_hdr_t *frame)
{
  if ( frame->fb == NULL )
    frame = NULL;
  //fprintf(stderr, "-- set_frame %s\n", frame ? frame->id : "(null)");
  display_screenshot_frame = frame;
  gtk_widget_set_sensitive(display_screenshot_button, (frame != NULL));
}


static void display_screenshot_grab(void)
{
  frame_geometry_t g;
  char *fname;

  if ( display_screenshot_frame == NULL )
    return;
  if (display_screenshot_basename == NULL)
    return;

  display_screenshot_get_selection(&g);
  //fprintf(stderr, "-- grab frame=%s window=%s\n", display_screenshot_frame->id, frame_geometry_str(&g));

  fname = png_filename(display_screenshot_basename);

  if ( fname != NULL ) {
    //fprintf(stderr, "   => %s\n", fname);
    png_save(&(display_screenshot_frame->fb->rgb), &g, fname);
    free(fname);
  }
}


static void display_init_toolbar_button(GtkToolButton *button, char *icon_name)
{
  GdkPixbuf *pixbuf = create_pixbuf(icon_name);

  if ( pixbuf != NULL ) {
    GtkWidget *image = gtk_image_new_from_pixbuf(pixbuf);
    gtk_widget_show(image);
    gtk_tool_button_set_icon_widget(button, image);
    gdk_pixbuf_unref(pixbuf);
  }
}


void display_screenshot_init(GtkWindow *window)
{
  display_screenshot_button = lookup_widget(GTK_WIDGET(window), "button_screenshot");
  display_init_toolbar_button(GTK_TOOL_BUTTON(display_screenshot_button), "screenshot.png");
  gtk_signal_connect_object(GTK_OBJECT(display_screenshot_button), "clicked",
                            GTK_SIGNAL_FUNC(display_screenshot_grab), NULL);

  display_screenshot_done();
}


void display_screenshot_done(void)
{
  display_screenshot_sel = frame_geometry_null;
  display_screenshot_frame = NULL;
  gtk_widget_set_sensitive(display_screenshot_button, 0);

  /* Free screenshot basename */
  if (display_screenshot_basename != NULL) {
	  free(display_screenshot_basename);
	  display_screenshot_basename = NULL;
  }
}
