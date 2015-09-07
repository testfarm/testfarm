/****************************************************************************/
/* TestFarm Core                                                            */
/* Test Progress dump                                                       */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 02-OCT-2007                                                    */
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

#ifndef __TESTFARM_REPORT_PROGRESS_H__
#define __TESTFARM_REPORT_PROGRESS_H__

#include <glib.h>

typedef struct {
  guint timeout_tag;
  int value;
  char *msg;
} report_progress_t;

extern void report_progress_init(report_progress_t *rp);

extern void report_progress_msg(report_progress_t *rp, char *msg);
extern void report_progress_value(report_progress_t *rp, int value);
extern void report_progress_clear(report_progress_t *rp);

#endif /* __TESTFARM_REPORT_PROGRESS_H__ */
