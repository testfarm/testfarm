/****************************************************************************/
/* TestFarm                                                                 */
/* File validation check                                                    */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 29-JAN-2004                                                    */
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

#ifndef __TESTFARM_VALIDATE_H__
#define __TESTFARM_VALIDATE_H__

#include "codegen.h"
#include "md5.h"

extern int validate_max;

typedef struct {
  char *md5sum;
  int level;
  char *date;
  char *operator;
  int updated;
} validate_t;

extern int validate_init(void);
extern void validate_destroy(void);

extern validate_t *validate_alloc(void);
extern void validate_free(validate_t *v);
extern validate_t *validate_object_data(tree_object_t *object);

extern int validate_check(validate_t *v, tree_object_t *object);
extern int validate_update(validate_t *v, tree_object_t *object, int level, char *operator);

extern char *validate_get_id(int level);
extern char *validate_get_icon(int level);

#endif  /* __TESTFARM_VALIDATE_H__ */
