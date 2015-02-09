/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* Display control buffer - Display window geometry                         */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 25-JUN-2004                                                    */
/****************************************************************************/

/*
 * $Revision: 942 $
 * $Date: 2008-02-07 15:45:54 +0100 (jeu., 07 févr. 2008) $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>

#include "frame_geometry.h"

const frame_geometry_t frame_geometry_null = FRAME_GEOMETRY_NULL;


int frame_geometry_parse_position(char *s, frame_geometry_t *g)
{
  char *sy;
  char cy;

  sy = s;
  while ( (*sy == '+') || (*sy == '-') )
    sy++;
  while ( (*sy != '\0') && (*sy != '+') && (*sy != '-') )
    sy++;

  cy = *sy;

  if ( cy != '\0' ) {
    g->y = atoi(sy);
    *sy = '\0';
  }

  g->x = atoi(s);

  *sy = cy;

  return 0;
}


int frame_geometry_parse(char *str, frame_geometry_t *g)
{
  char *s = strdup(str);
  char *sx = NULL;
  char *sheight = NULL;

  /* Retrieve window size */
  if ( (sheight = strchr(s, 'x')) != NULL ) {
    char cx;
    int v;

    *(sheight++) = '\0';
    v = atoi(s);
    if ( v < 0 )
      v = 0;
    g->width = v;

    sx = sheight;
    while ( (*sx != '\0') && (*sx != '+') && (*sx != '-') )
      sx++;

    cx = *sx;
    *sx = '\0';
    v = atoi(sheight);
    *sx = cx;

    if ( v < 0 )
      v = 0;
    g->height = v;
  }
  else {
    sx = s;
  }

  /* Retrieve window position */
  frame_geometry_parse_position(sx, g);

  free(s);

  return 0;
}


void frame_geometry_clip(frame_geometry_t *g, frame_geometry_t *g0)
{
  if ( abs(g->x) >= g0->width ) {
    g->width = 0;
  }
  else {
    if ( g->x < 0 )
      g->x += g0->width;
    if ( g->x < 0 )
      g->x = 0;
    if ( (g->x + g->width) > g0->width )
      g->width = g0->width - g->x;
  }

  if ( abs(g->y) >= g0->height ) {
    g->height = 0;
  }
  else {
    if ( g->y < 0 )
      g->y += g0->height;
    if ( g->y < 0 )
      g->y = 0;
    if ( (g->y + g->height) > g0->height )
      g->height = g0->height - g->y;
  }
}


char *frame_geometry_str(frame_geometry_t *g)
{
  static char buf[32];
  snprintf(buf, sizeof(buf), "%ux%u%+d%+d", g->width, g->height, g->x, g->y);
  return buf;
}


int frame_geometry_overlaps( frame_geometry_t *g1, frame_geometry_t *g2)
{
  if ( (g1->x + g1->width) < g2->x )
    return 0;
  if ( (g1->y + g1->height) < g2->y )
    return 0;
  if ( (g2->x + g2->width) < g1->x )
    return 0;
  if ( (g2->y + g2->height) < g1->y )
    return 0;

  return 1;
}


int frame_geometry_intersect(frame_geometry_t *g1, frame_geometry_t *g2, frame_geometry_t *gi)
{
  int v1, v2;
  int x1, y1;
  int x2, y2;

  x1 = (g1->x > g2->x) ? g1->x : g2->x;
  v1 = g1->x + g1->width;
  v2 = g2->x + g2->width;
  x2 = (v1 < v2) ? v1 : v2;
  if ( x1 >= x2 )
    return 0;

  y1 = (g1->y > g2->y) ? g1->y : g2->y;
  v1 = g1->y + g1->height;
  v2 = g2->y + g2->height;
  y2 = (v1 < v2) ? v1 : v2;
  if ( y1 >= y2 )
    return 0;

  if ( gi != NULL ) {
    gi->x = x1;
    gi->y = y1;
    gi->width = x2 - x1;
    gi->height = y2 - y1;
  }

  return 1;
}


void frame_geometry_union(frame_geometry_t *g, frame_geometry_t *g2)
{
  if ( (g2->width > 0) && (g2->height > 0) ) {
    if ( (g->width > 0) && (g->height > 0) ) {
      int xx, x1, x2;
      int yy, y1, y2;

      x2 = g->x + g->width;
      xx = g2->x + g2->width;
      if ( xx > x2 )
	x2 = xx;

      x1 = (g->x < g2->x) ? g->x : g2->x;
      g->x = x1;
      g->width = x2 - x1;

      y2 = g->y + g->height;
      yy = g2->y + g2->height;
      if ( yy > y2 )
	y2 = yy;

      y1 = (g->y < g2->y) ? g->y : g2->y;
      g->y = y1;
      g->height = y2 - y1;
    }
    else {
      *g = *g2;
    }
  }
}
