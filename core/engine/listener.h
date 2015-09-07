/****************************************************************************/
/* TestFarm                                                                 */
/* Test Engine - Incoming events listener                                   */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 11-SEP-2003 (split from trig.h)                                */
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

#ifndef __TESTFARM_LISTENER_H__
#define __TESTFARM_LISTENER_H__

#include <glib.h>

typedef struct {
  char *str;
  GList *list;
} listener_t;
#if 0
typedef GList listener_t;
#endif

extern listener_t *listener_new(int argc, char **argv);
extern void listener_destroy(listener_t *listener);
extern char *listener_str(listener_t *listener);
extern char *listener_raised(listener_t *listener);
extern int listener_check(listener_t *listener);

#endif /* __TESTFARM_LISTENER_H__ */
