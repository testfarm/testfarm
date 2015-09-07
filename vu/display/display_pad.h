/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* Graphical Front-end - Pad area rendering                                 */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 07-AUG-2007                                                    */
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
