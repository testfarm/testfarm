/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* Graphical Front-end - Display area selection                             */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 07-AUG-2007                                                    */
/****************************************************************************/

/*
 * $Revision: 1040 $
 * $Date: 2009-04-19 17:31:42 +0200 (dim., 19 avril 2009) $
 */

#include <stdio.h>

#include "utils.h"
#include "style.h"
#include "display_sel.h"


static GtkDrawingArea *display_sel_darea = NULL;
static GdkColor *display_sel_color[DISPLAY_SEL_COLOR_MAX] = { &red, &green, &blue };
static GdkGC *display_sel_gc[DISPLAY_SEL_COLOR_MAX] = {};


void display_sel_draw(display_sel_color_t color, frame_geometry_t *sel_g)
{
  if ( display_sel_darea == NULL )
    return;
  if ( sel_g->width <= 0 )
    return;
  if ( sel_g->height <= 0 )
    return;

  debug(64, "SEL DRAW x=%u y=%u w=%u h=%u\n", sel_g->x, sel_g->y, sel_g->width, sel_g->height);

  /* Clip selection into RGB area */
  if ( sel_g->x < 0 )
    sel_g->x = 0;
  if ( sel_g->y < 0 )
    sel_g->y = 0;

  gdk_draw_rectangle(GTK_WIDGET(display_sel_darea)->window,
                     display_sel_gc[color],
                     FALSE, /* Not filled */
                     sel_g->x, sel_g->y,
		     sel_g->width,
		     sel_g->height);
}


void display_sel_undraw(display_t *d, frame_geometry_t *sel_g)
{
  frame_geometry_t g;
  int v;

  if ( d == NULL )
    return;
  if ( display_sel_darea == NULL )
    return;
  if ( sel_g->width <= 0 )
    return;
  if ( sel_g->height <= 0 )
    return;

  debug(64, "SEL UNDRAW x=%u y=%u w=%u h=%u\n", sel_g->x, sel_g->y, sel_g->width, sel_g->height);

  /* Horizontal, top */
  v = sel_g->x - DISPLAY_SEL_BORDER/2;
  g.x = (v > 0) ? v : 0;
  v = sel_g->y - DISPLAY_SEL_BORDER/2;
  g.y = (v > 0) ? v : 0;
  g.width = sel_g->width + DISPLAY_SEL_BORDER;
  g.height = DISPLAY_SEL_BORDER;
  display_refresh(d, &g);

  /* Vertical, left */
  g.width = DISPLAY_SEL_BORDER;
  g.height = sel_g->height + DISPLAY_SEL_BORDER;
  display_refresh(d, &g);

  /* Vertical, right */
  v = sel_g->x + sel_g->width - DISPLAY_SEL_BORDER/2;
  g.x = (v > 0) ? v : 0;
  display_refresh(d, &g);

  if ( (sel_g->x + sel_g->width) >= d->desktop.root.g0.width ) {
    gdk_draw_rectangle(GTK_WIDGET(display_sel_darea)->window,
		       GTK_WIDGET(display_sel_darea)->style->bg_gc[GTK_STATE_NORMAL],
		       TRUE, /* Filled */
		       d->desktop.root.g0.width, g.y,
		       DISPLAY_SEL_BORDER, g.height);
  }

  /* Horizontal, bottom */
  v = sel_g->x - DISPLAY_SEL_BORDER/2;
  g.x = (v > 0) ? v : 0;
  v = sel_g->y + sel_g->height - DISPLAY_SEL_BORDER/2;
  g.y = (v > 0) ? v : 0;
  g.width = sel_g->width + DISPLAY_SEL_BORDER;
  g.height = DISPLAY_SEL_BORDER;
  display_refresh(d, &g);

  if ( (sel_g->y + sel_g->height) >= d->desktop.root.g0.height ) {
    gdk_draw_rectangle(GTK_WIDGET(display_sel_darea)->window,
		       GTK_WIDGET(display_sel_darea)->style->bg_gc[GTK_STATE_NORMAL],
		       TRUE, /* Filled */
		       g.x, d->desktop.root.g0.height,
		       g.width, DISPLAY_SEL_BORDER);
  }
}


void display_sel_clip(display_t *d)
{
  GdkRectangle rect;
  int i;

  if ( display_sel_darea == NULL )
    return;

  rect.x = 0;
  rect.y = 0;
  rect.width = d->desktop.root.g0.width + DISPLAY_SEL_BORDER;
  rect.height = d->desktop.root.g0.height + DISPLAY_SEL_BORDER;
  for (i = 0;  i < DISPLAY_SEL_COLOR_MAX; i++)
    gdk_gc_set_clip_rectangle(display_sel_gc[i], &rect);

  gdk_draw_rectangle(GTK_WIDGET(display_sel_darea)->window,
		     GTK_WIDGET(display_sel_darea)->style->bg_gc[GTK_STATE_NORMAL],
		     TRUE, /* Filled */
		     d->desktop.root.g0.width, 0,
		     DISPLAY_SEL_BORDER, rect.height);
}


void display_sel_init(GtkDrawingArea *darea)
{
  int i;

  display_sel_darea = darea;

  for (i = 0;  i < DISPLAY_SEL_COLOR_MAX; i++) {
    display_sel_gc[i] = gdk_gc_new(GTK_WIDGET(darea)->window);
    gdk_gc_set_foreground(display_sel_gc[i], display_sel_color[i]);
    gdk_gc_set_line_attributes(display_sel_gc[i], DISPLAY_SEL_BORDER, GDK_LINE_ON_OFF_DASH, GDK_CAP_NOT_LAST, GDK_JOIN_MITER);
    gdk_gc_set_clip_origin(display_sel_gc[i], 0, 0);
  }
}


void display_sel_done(void)
{
  int i;

  display_sel_darea = NULL;

  for (i = 0;  i < DISPLAY_SEL_COLOR_MAX; i++) {
    gdk_gc_destroy(display_sel_gc[i]);
    display_sel_gc[i] = NULL;
  }
}
