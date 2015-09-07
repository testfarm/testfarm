/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* Graphical Front-end - Display area selection                             */
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

#ifndef __TVU_DISPLAY_SEL_H__
#define __TVU_DISPLAY_SEL_H__

#include <gtk/gtk.h>
#include "frame_geometry.h"
#include "display.h"

#define DISPLAY_SEL_BORDER 2

typedef enum {
  DISPLAY_SEL_COLOR_RED=0,
  DISPLAY_SEL_COLOR_GREEN,
  DISPLAY_SEL_COLOR_BLUE,
  DISPLAY_SEL_COLOR_MAX
} display_sel_color_t;


extern void display_sel_init(GtkDrawingArea *darea);
extern void display_sel_done(void);
extern void display_sel_clip(display_t *d);

extern void display_sel_draw(display_sel_color_t color, frame_geometry_t *sel_g);
extern void display_sel_undraw(display_t *d, frame_geometry_t *sel_g);

#endif /* __TVU_DISPLAY_SEL_H__ */
