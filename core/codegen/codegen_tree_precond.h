/****************************************************************************/
/* TestFarm                                                                 */
/* Test Suite code generator : Test Case precondition                       */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 11-MAY-2004                                                    */
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

#ifndef __TESTFARM_CODEGEN_TREE_PRECOND_H
#define __TESTFARM_CODEGEN_TREE_PRECOND_H

#include "codegen_tree.h"

extern int tree_precond_new(tree_case_t *tcase, char *str, tree_err_loc_t *loc);
extern void tree_precond_destroy(tree_case_t *tcase);

extern int tree_precond_link(tree_case_t *tcase, tree_t *tree, char *name);
extern int tree_precond_evaluate(tree_case_t *tcase, tree_verdict_t verdict);
extern void tree_precond_show(tree_case_t *tcase, FILE *f);

#endif /* __TESTFARM_CODEGEN_TREE_PRECOND_H */
