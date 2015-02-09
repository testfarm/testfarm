/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* Graphical Front-end - Pad area rendering                                 */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 07-AUG-2007                                                    */
/****************************************************************************/

/*
 * $Revision: 817 $
 * $Date: 2007-11-27 12:37:15 +0100 (mar., 27 nov. 2007) $
 */

#ifndef __TVU_DISPLAY_PAD_H__
#define __TVU_DISPLAY_PAD_H__

#include "color.h"
#include "frame_geometry.h"

typedef struct {
  GList *list;
  unsigned int width, height;
  unsigned char *buf;
  unsigned char *mask;
} display_pad_t;

extern color_rgb_t display_pad_color;

extern void display_pad_init(display_pad_t *pad);
extern void display_pad_setup(display_pad_t *pad,
			      unsigned char *buf, unsigned int width, unsigned int height);
extern void display_pad_add(display_pad_t *pad, frame_geometry_t *g, color_rgb_t color);

extern void display_pad_render(display_pad_t *pad, frame_geometry_t *g);

#endif /* __TVU_DISPLAY_PAD_H__ */
