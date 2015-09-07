/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* Text Pattern management                                                  */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 27-JUN-2007                                                    */
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

#ifndef __TVU_PATTERN_TEXT_H__
#define __TVU_PATTERN_TEXT_H__

#include <glib.h>
#include "pattern.h"

extern int pattern_text_set(pattern_t *pattern, char *source, char **err);
extern int pattern_text_set_options(pattern_t *pattern, GList *options, char **err);
extern void pattern_text_free(pattern_t *pattern);
extern void pattern_text_copy(pattern_t *pattern, pattern_t *pattern0);
extern int pattern_text_str(pattern_t *pattern, char *buf, int size);
extern int pattern_text_diff(pattern_t *p1, pattern_t *p2);

#endif /* __TVU_PATTERN_TEXT_H__ */
