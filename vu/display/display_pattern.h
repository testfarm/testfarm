/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* Display Tool - pattern management                                        */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 22-MAR-2007                                                    */
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

#ifndef __TVU_DISPLAY_PATTERN_H__
#define __TVU_DISPLAY_PATTERN_H__

#include "fuzz.h"
#include "display.h"
#include "frame_hdr.h"

extern int display_pattern_init(void);
extern void display_pattern_done(void);
extern void display_pattern_popup(void);

extern void display_pattern_set_ctl(int sock);
extern void display_pattern_set_selection(frame_geometry_t *g);
extern void display_pattern_set_frame(frame_hdr_t *frame);
extern void display_pattern_remove_frame(frame_hdr_t *frame);

extern void display_pattern_show_selection(void);
extern void display_pattern_clear(void);
extern void display_pattern_update(char *id, frame_hdr_t *frame, char *source, frame_geometry_t *g, unsigned int mode, int type,
				   fuzz_t fuzz, unsigned char loss[2]);
extern void display_pattern_match(display_t *d, char *id, int state);

#endif /* __TVU_DISPLAY_PATTERN_H__ */
