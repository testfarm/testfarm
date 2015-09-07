/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* Remote Frame Buffer display - Pattern editor                             */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 14-JAN-2004                                                    */
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

#ifndef __TVU_DISPLAY_EDITOR_H
#define __TVU_DISPLAY_EDITOR_H

#include "frame_hdr.h"
#include "pattern.h"

typedef enum {
  EDITOR_EVENT_MODIFIED=0,
  EDITOR_EVENT_UPDATE,
} editor_event_t;

typedef int editor_func_t(editor_event_t event, pattern_t *pattern);

extern int editor_init(GtkWindow *window, editor_func_t *event_fn);
extern void editor_done(void);

extern void editor_set_frame(frame_hdr_t *frame);
extern void editor_remove_frame(frame_hdr_t *frame);
extern void editor_set_selection(frame_geometry_t *g);

extern int editor_show(pattern_t *pattern);
extern void editor_show_match(int state);

#endif /* __TVU_DISPLAY_EDITOR_H */
