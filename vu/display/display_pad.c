/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* Graphical Front-end - Pad area rendering                                 */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 07-AUG-2007                                                    */
/****************************************************************************/

/*
 * $Revision: 1095 $
 * $Date: 2009-12-28 10:59:04 +0100 (lun., 28 déc. 2009) $
 */

#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <glib.h>

#include "frame_geometry.h"
#include "utils.h"
#include "display.h"
#include "display_pattern.h"
#include "display_pad.h"


typedef struct {
  frame_geometry_t g;
  unsigned int tag;
  int x2, y2;
  color_rgb_t color;
} display_pad_item_t;


color_rgb_t display_pad_color = {255,0,0};


static void display_pad_clear_mask(display_pad_t *pad, frame_geometry_t *g)
{
  int width = pad->width;
  unsigned char *mask0 = pad->mask + (width * g->y) + g->x;
  int xi, yi;

  for (yi = 0; yi < g->height; yi++) {
    for (xi = 0; xi < g->width; xi++)
      mask0[xi] = 0;
    mask0 += width;
  }
}


void display_pad_render(display_pad_t *pad, frame_geometry_t *g)
{
  GList *l = pad->list;
  int width = pad->width;
  int rowstride = DISPLAY_BPP * width;
  int gx1 = g->x;
  int gy1 = g->y;
  int gx2 = g->x + g->width;
  int gy2 = g->y + g->height;
  int cleared = 0;

  //fprintf(stderr, "-- display_pad_render %s\n", frame_geometry_str(g));

  while ( l != NULL ) {
    display_pad_item_t *item = l->data;
    int x1 = item->g.x;
    int y1 = item->g.y;

    /* Do nothing if the pad item does not interesect the refresh window */
    if ( (item->x2 >= gx1) && (item->y2 >= gy1) &&
	 (gx2 >= x1) && (gy2 >= y1) ) {
      unsigned char *ybuf = pad->buf + (rowstride * y1) + (DISPLAY_BPP * x1);
      unsigned char *ymask = pad->mask + (width * y1) + x1;
      int xmax, ymax;
      int xi, yi;

      if ( ! cleared ) {
	display_pad_clear_mask(pad, g);
	cleared = 1;
      }

      xmax = ((int) item->g.width) - 1;
      if ( xmax < 0 )
	xmax = 0;

      ymax = ((int) item->g.height) - 1;
      if ( ymax < 0 )
	ymax = 0;

      for (yi = 0; yi <= ymax; yi++) {
	unsigned char *xbuf = ybuf;
	unsigned char *xmask = ymask;

	for (xi = 0; xi <= xmax; xi++) {
	  if ( (yi == 0) || (yi == ymax) || (xi == 0) || (xi == xmax) ) {
	    xbuf[0] = item->color[0];
	    xbuf[1] = item->color[1];
	    xbuf[2] = item->color[2];
	  }
	  else if ( ! *xmask ) {
	    xbuf[0] = ((((unsigned int) xbuf[0]) * 8) / 10) + (((unsigned int) item->color[0]) * 2 / 10);
	    xbuf[1] = ((((unsigned int) xbuf[1]) * 8) / 10) + (((unsigned int) item->color[1]) * 2 / 10);
	    xbuf[2] = ((((unsigned int) xbuf[2]) * 8) / 10) + (((unsigned int) item->color[2]) * 2 / 10);
	  }

	  xbuf += DISPLAY_BPP;
	  *(xmask++) = 1;
	}

	ybuf += rowstride;
	ymask += width;
      }
    }

    l = l->next;
  }
}


static void display_pad_refresh(frame_geometry_t *g)
{
  display_refresh(display_current, g);
  display_pattern_show_selection();
  display_selection_show(display_current);
}


static gboolean display_pad_timeout(display_pad_item_t *item)
{
  frame_geometry_t prev;

  debug(DEBUG_PATTERN, "PAD TIMEOUT %s tag=%u\n", frame_geometry_str(&item->g), item->tag);

  prev = item->g;
  item->g = frame_geometry_null;

  if ( prev.width > 0 ) {
    display_pad_refresh(&prev);
  }

  return FALSE;
}


static void display_pad_timeout_stop(display_pad_item_t *item)
{
  if ( item->tag > 0 ) {
    g_source_remove(item->tag);
    item->tag = 0;
  }
}


static void display_pad_remove(display_pad_t *pad, display_pad_item_t *item)
{
  display_pad_timeout_stop(item);
  pad->list = g_list_remove(pad->list, item);
  free(item);
}


static void display_pad_clean(display_pad_t *pad)
{
  GList *l = pad->list;

  while ( l != NULL ) {
    display_pad_item_t *item = l->data;

    if ( item->g.width == 0 ) {
      display_pad_remove(pad, item);
      l = pad->list;
    }
    else {
      l = l->next;
    }
  }
}


void display_pad_setup(display_pad_t *pad, unsigned char *buf, unsigned int width, unsigned int height)
{
  int size;

  /* Clear pad list */
  g_list_foreach(pad->list, (GFunc) display_pad_timeout_stop, NULL);
  g_list_foreach(pad->list, (GFunc) free, NULL);
  g_list_free(pad->list);
  pad->list = NULL;

  /* Store display settings */
  pad->buf = buf;
  pad->width = width;
  pad->height = height;

  /* Free rendering mask buffer */
  if ( pad->mask != NULL ) {
    free(pad->mask);
    pad->mask = NULL;
  }

  size = width * height;
  if ( size > 0 ) {
    pad->mask = malloc(size);
    memset(pad->mask, 0, size);
  }
}


void display_pad_add(display_pad_t *pad, frame_geometry_t *g, color_rgb_t color)
{
  display_pad_item_t *item;

  display_pad_clean(pad);

  item = (display_pad_item_t *) malloc(sizeof(display_pad_item_t));
  item->g = *g;
  item->tag = g_timeout_add(5000, (GSourceFunc) display_pad_timeout, item);
  item->x2 = g->x + g->width;
  item->y2 = g->y + g->height;

  item->color[0] = color[0];
  item->color[1] = color[1];
  item->color[2] = color[2];

  debug(DEBUG_PATTERN, "PAD ADD %s color=#%02X%02X%02X tag=%u\n",
	frame_geometry_str(g),
	color[0], color[1], color[2],
	item->tag);

  pad->list = g_list_append(pad->list, item);

  display_pad_refresh(g);
}


void display_pad_init(display_pad_t *pad)
{
  pad->list = NULL;
  pad->buf = NULL;
  pad->width = 0;
  pad->height = 0;
  pad->mask = NULL;
}
