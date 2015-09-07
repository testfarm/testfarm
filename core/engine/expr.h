/****************************************************************************/
/* TestFarm                                                                 */
/* Test Engine - Trigger expression evaluator                               */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 11-SEP-2003                                                    */
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

#ifndef __TESTFARM_EXPR_H__
#define __TESTFARM_EXPR_H__

#include <glib.h>

#define EXPR_NOP -1
#define EXPR_AND  0
#define EXPR_OR   1

typedef struct {
  int op;
  char *value;
  void *data;
} expr_item_t;

extern void expr_compile(GList **plist, char *expression);
extern void expr_free(GList *list);

typedef int expr_eval_hdl_t(expr_item_t *);

extern int expr_eval(GList *list, expr_eval_hdl_t *hdl);

#endif /* __TESTFARM_EXPR_H__ */
