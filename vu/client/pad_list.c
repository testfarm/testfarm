/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* Color padding                                                            */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 03-AUG-2007                                                    */
/****************************************************************************/

/*
 * $Revision: 1017 $
 * $Date: 2008-08-13 17:00:11 +0200 (mer., 13 août 2008) $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>

#include "color.h"
#include "pad_list.h"


typedef struct {
  frame_geometry_t g;
  int x1, y1;
  int x2, y2;
  int mx1, my1;
  int mx2, my2;
} pad_area_t;


static void pad_area_update_borders(pad_area_t *area,
				    unsigned int width, unsigned int height,
				    unsigned int gap)
{
  unsigned int border = gap + 1;

  //fprintf(stderr, "-- Update pad +%d+%d|+%d+%d\n", area->x1, area->y1, area->x2, area->y2);
  area->mx1 = area->x1 - border;
  if ( area->mx1 < 0 )
    area->mx1 = 0;

  area->my1 = area->y1 - border;
  if ( area->my1 < 0 )
    area->my1 = 0;

  area->mx2 = area->x2 + border;
  if ( area->mx2 >= width )
    area->mx2 = width - 1;

  area->my2 = area->y2 + border;
  if ( area->my2 >= height )
    area->my2 = height - 1;
}


static void pad_area_set_geometry(pad_area_t *area)
{
  area->g.x = area->x1;
  area->g.y = area->y1;
  area->g.width = area->x2 - area->x1 + 1;
  area->g.height = area->y2 - area->y1 + 1;
}


static void pad_area_show(pad_area_t *area)
{
  printf(" %s", frame_geometry_str(&area->g));
}


static void pad_list_merge_neighbours(pad_list_t *list)
{
  GList *l1;

  l1 = list->list;
  while ( l1 != NULL ) {
    int overlap = 0;
    GList *l2;

    l2 = list->list;
    while ( l2 != NULL ) {
      /* Do not compare pad with itself */
      if ( l2 != l1 ) {
	pad_area_t *area1 = l1->data;
	pad_area_t *area2 = l2->data;

	/* Check if pads area1 and area2 overlap each other */
	if ( (area2->x2 >= area1->mx1) && (area2->y2 >= area1->my1) &&
	     (area1->mx2 >= area2->x1) && (area1->my2 >= area2->y1) ) {
	  //fprintf(stderr, "-- Merging pad +%d+%d|+%d+%d", area1->x1, area1->y1, area1->x2, area1->y2);
	  //fprintf(stderr, " with +%d+%d|+%d+%d", area2->x1, area2->y1, area2->x2, area2->y2);

	  /* If pads overlap, grow 1st pad so that it includes 2nd pad geometry */
	  if ( area2->x1 < area1->x1 )
	    area1->x1 = area2->x1;
	  if ( area2->y1 < area1->y1 )
	    area1->y1 = area2->y1;
	  if ( area2->x2 > area1->x2 )
	    area1->x2 = area2->x2;
	  if ( area2->y2 > area1->y2 )
	    area1->y2 = area2->y2;

	  pad_area_update_borders(area1, list->width, list->height, list->gap);

	  //fprintf(stderr, " => +%d+%d|+%d+%d\n", area1->x1, area1->y1, area1->x2, area1->y2);

	  /* Destroy 2nd pad and remove it from the list */
	  list->list = g_list_remove(list->list, area2);
	  free(area2);
	  overlap = 1;
	  break;
	}
      }

      l2 = l2->next;
    }

    /* If overlaping pads were merged, restart from the begining,
       otherwise continue checking the pad list */
    if ( overlap )
      l1 = list->list;
    else
      l1 = l1->next;
  }
}


static void pad_list_add_window(pad_list_t *list, frame_geometry_t *g)
{
  pad_area_t *area;

  /* Create a new pad containing the window */
  area = (pad_area_t *) malloc(sizeof(pad_area_t));
  area->x1 = g->x;
  area->x2 = g->x + g->width - 1;
  area->y1 = g->y;
  area->y2 = g->y + g->height - 1;
  pad_area_update_borders(area, list->width, list->height, list->gap);
  //fprintf(stderr, "-- New pad +%d+%d|+%d+%d\n", area->x1, area->y1, area->x2, area->y2);

  /* Add new pad to the list */
  list->list = g_list_append(list->list, area);

  /* Merge overlaping pads together */
  pad_list_merge_neighbours(list);
}


static void pad_list_add_pixel(pad_list_t *list, unsigned int x, unsigned int y)
{
  frame_geometry_t g = {
    x: x,
    y: y,
    width: 1,
    height: 1
  };

  pad_list_add_window(list, &g);
}


static void pad_list_remove_too_small(pad_list_t *list, unsigned int min_width, unsigned int min_height)
{
  GList *l = list->list;

  while ( l ) {
    pad_area_t *area = l->data;

    l = g_list_next(l);

    if ( (area->g.width < min_width) || (area->g.height < min_height) ) {
      list->list = g_list_remove(list->list, area);
    }
  }
}


pad_list_t *pad_list_add(char *id,
			 frame_t *frame, frame_geometry_t *g,
			 unsigned long color_value, fuzz_t *fuzz,
			 unsigned int gap,
			 unsigned int min_width, unsigned int min_height)
{
  pad_list_t *list;
  frame_rgb_t *rgb = &(frame->hdr.fb->rgb);
  unsigned char *py = rgb->buf + (g->y * rgb->rowstride) + (rgb->bpp * g->x);
  color_rgb_t color;
  int xi, yi;

  list = (pad_list_t *) malloc(sizeof(pad_list_t));
  list->id = strdup(id);
  list->frame_id = strdup(frame->hdr.id);
  list->x0 = frame->hdr.g0.x;
  list->y0 = frame->hdr.g0.y;
  list->width = rgb->width;
  list->height = rgb->height;
  list->gap = gap;
  list->list = NULL;

  color_set_rgb(color_value, color);

  for (yi = 0; yi < g->height; yi++) {
    unsigned char *px = py;

    for (xi = 0; xi < g->width; xi++) {
#if 0
      fprintf(stderr, "++ +%d+%d : %02X%02X%02X / %02X%02X%02X / %02X%02X%02X : %s\n", xi, yi,
	      px[0], px[1], px[2],
	      color[0], color[1], color[2],
	      fuzz_str(fuzz),
	      abs(px[0] - color[0]), abs(px[1] - color[1]), abs(px[2] - color[2]));
#endif

      /* If pixel color is ok, merge it to the pad list */
      if ( fuzz_check(fuzz, px, color) ) {
	pad_list_add_pixel(list, g->x + xi, g->y + yi);
      }

      px += rgb->bpp;
    }

    py += rgb->rowstride;
  }

  /* Compute pad geometry */
  g_list_foreach(list->list, (GFunc) pad_area_set_geometry, NULL);

  /* Remove pads that are too small */
  pad_list_remove_too_small(list, min_width, min_height);

  return list;
}


void pad_list_destroy(pad_list_t *list)
{
  g_list_foreach(list->list, (GFunc) free, NULL);
  g_list_free(list->list);
  list->list = NULL;

  free(list->frame_id);
  free(list->id);

  free(list);
}


void pad_list_show(pad_list_t *list, char *hdr)
{
  printf("%s%s frame=%s ", hdr, list->id, list->frame_id);
  g_list_foreach(list->list, (GFunc) pad_area_show, NULL);
  printf("\n");

}


frame_geometry_t *pad_list_tab(pad_list_t *list, int *_nmemb)
{
  frame_geometry_t *gtab;
  int nmemb;
  GList *l1;
  int i;

  nmemb = g_list_length(list->list);
  gtab = calloc(nmemb, sizeof(frame_geometry_t));

  i = 0;
  l1 = list->list;
  while ( (l1 != NULL) && (i < nmemb) ) {
    pad_area_t *area1 = l1->data;

    gtab[i].x = list->x0 + area1->g.x;
    gtab[i].y = list->y0 + area1->g.y;
    gtab[i].width = area1->g.width;
    gtab[i].height = area1->g.height;

    i++;
    l1 = g_list_next(l1);
  }

  *_nmemb = nmemb;

  return gtab;
}
