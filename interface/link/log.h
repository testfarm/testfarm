/****************************************************************************/
/* TestFarm                                                                 */
/* Test Interface library: log management                                   */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 30-MAR-2000                                                    */
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

#ifndef __INTERFACE_LOG_H__
#define __INTERFACE_LOG_H__

#include "tstamp.h"

#define TAG_INIT  "INIT   "
#define TAG_DONE  "DONE   "

extern char *log_hdr(tstamp_t tstamp, char *tag);
extern char *log_hdr_(char *tag);

extern void log_error(char *tag, char *fmt, ...);
extern void log_dump(char *hdr, char *s);

#endif /* __INTERFACE_LOG_H__ */
