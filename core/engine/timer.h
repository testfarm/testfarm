/****************************************************************************/
/* TestFarm                                                                 */
/* Test Engine - Timer messaging                                            */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 19-NOV-1999                                                    */
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

#ifndef __TESTFARM_ENGINE_TIMER_H__
#define __TESTFARM_ENGINE_TIMER_H__

#include <sys/time.h>

#include "shell.h"

extern int timer_value(shell_t *shell, char *s_value, char *s_unit, long *t);
extern void timer_tv(struct timeval *tv, unsigned long t);

#endif /* __TESTFARM_ENGINE_TIMER_H__ */
