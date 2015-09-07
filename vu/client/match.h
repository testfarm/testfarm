/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* Display Frame pattern matching                                           */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 23-DEC-2003                                                    */
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

#ifndef __TVU_MATCH_H__
#define __TVU_MATCH_H__

#include <glib.h>

#include "frame_display.h"
#include "pattern.h"

#define MATCH_TAG "MATCH  "

typedef struct {
  frame_display_t *display; /* Display Tool communication hook */
  GList *list;              /* List of patterns */
  GList *removed;           /* Patterns marked for deletion */
} match_t;

extern match_t *match_alloc(frame_display_t *display);
extern void match_destroy(match_t *match);

extern pattern_t *match_add(match_t *match, char *id,
			    frame_t *frame, frame_geometry_t *g,
			    pattern_mode_t mode,
			    pattern_type_t type, char *source);
extern int match_reset(match_t *match, char *id);
extern int match_remove(match_t *match, char *id);
extern int match_show(match_t *match, char *id, char *hdr);

extern void match_show_pattern(pattern_t *pattern, char *hdr);
extern void match_remove_pattern(pattern_t *pattern);
extern void match_remove_frame(frame_t *frame);

extern void match_process(pattern_t *pattern, frame_geometry_t *updated);
extern void match_process_reply(pattern_t *pattern, int result);

#endif /* __TVU_MATCH_H__ */
