/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* Screenshot Grab Management                                               */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 03-MAR-2008                                                    */
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

#ifndef __TVU_DISPLAY_SCREENSHOT_H__
#define __TVU_DISPLAY_SCREENSHOT_H__

#include "frame_geometry.h"
#include "frame_hdr.h"

extern void display_screenshot_init(GtkWindow *window);
extern void display_screenshot_done(void);

extern void display_screenshot_set_frame(frame_hdr_t *frame);
extern void display_screenshot_set_selection(frame_geometry_t *g);
extern void display_screenshot_set_dir(void);

#endif /* __TVU_DISPLAY_SCREENSHOT_H__ */
