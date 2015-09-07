/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* Text Pattern matching                                                    */
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

#ifndef __TVU_MATCH_TEXT_H__
#define __TVU_MATCH_TEXT_H__

#include "pattern.h"
#include "frame.h"

extern int match_text_dump;

extern int match_text_request(pattern_t *pattern);
extern int match_text_info(pattern_t *pattern, char *str, int len);

#endif /* __TVU_MATCH_TEXT_H__ */
