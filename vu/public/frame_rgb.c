/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* Display control buffer - RGB frame buffer                                */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 07-NOV-2007                                                    */
/****************************************************************************/

/*
 * $Revision: 1016 $
 * $Date: 2008-08-13 16:16:39 +0200 (mer., 13 août 2008) $
 */

#include <string.h>

#include "frame_geometry.h"
#include "frame_rgb.h"


unsigned int frame_rgb_bufsize(unsigned int width, unsigned int height)
{
  return FRAME_RGB_BPP * width * height;
}


void frame_rgb_init(frame_rgb_t *rgb, unsigned int width, unsigned int height)
{
  rgb->width = width;
  rgb->height = height;
  rgb->bpp = FRAME_RGB_BPP;
  rgb->rowstride = rgb->bpp * width;
  memset(rgb->buf, 0, rgb->rowstride * height);
}


void frame_rgb_clip_geometry(frame_rgb_t *rgb, frame_geometry_t *g)
{
  frame_geometry_t g0 = {
    x : 0,
    y : 0,
    width : rgb->width,
    height : rgb->height
  };

  frame_geometry_clip(g, &g0);
}


int frame_rgb_parse_geometry(frame_rgb_t *rgb, char *str, frame_geometry_t *g)
{
  if ( frame_geometry_parse(str, g) )
    return -1;

  frame_rgb_clip_geometry(rgb, g);

  return 0;
}
