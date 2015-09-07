/****************************************************************************/
/* TestFarm                                                                 */
/* Test Engine - Incoming events triggering                                 */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 11-AUG-2000                                                    */
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

#ifndef __TESTFARM_TRIG_H__
#define __TESTFARM_TRIG_H__

#include <pcreposix.h>

#include "shell.h"
#include "periph_item.h"

#define TAG_TRIG  "TRIG   "

typedef struct {
  char *id;
  int state;
  char *info;
  periph_item_t *periph;
  char *regex;
  regex_t pattern;
} trig_t;

extern int trig_def(shell_t *shell, char *id, periph_item_t *periph, char **argv, int argc);
extern int trig_undef(char **argv, int argc);
extern int trig_clear(char **argv, int argc);
extern int trig_update(periph_item_t *periph, char *buf);

extern int trig_count(char *id);
extern int trig_info(char *id, char **info);

extern trig_t *trig_retrieve(char *id);

#endif /* __TESTFARM_TRIG_H__ */
