/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* Color padding                                                            */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 06-AUG-2007                                                    */
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

#ifndef __TVU_PAD_H__
#define __TVU_PAD_H__

#include <glib.h>
#include "frame_display.h"
#include "pad_list.h"

#define PAD_TAG "PAD    "

typedef struct {
  frame_display_t *display;
  GList *pads;
} pad_t;

extern pad_t *pad_alloc(frame_display_t *display);
extern void pad_destroy(pad_t *pad);

extern pad_list_t *pad_add(pad_t *pad, char *id,
			   frame_t *frame, frame_geometry_t *window,
			   unsigned long color_value, fuzz_t *fuzz,
			   unsigned int gap,
			   unsigned int min_width, unsigned int min_height);

extern pad_list_t *pad_retrieve(pad_t *pad, char *id);

extern int pad_remove(pad_t *pad, char *id);

extern void pad_show(pad_t *pad, char *id, char *hdr);

#endif /* __TVU_PAD_H__ */
