/****************************************************************************/
/* TestFarm                                                                 */
/* Test Suite code generator : Test Tree Error Recording                    */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 22-JUN-2000                                                    */
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

#ifndef __TESTFARM_CODEGEN_TREE_ERROR_H
#define __TESTFARM_CODEGEN_TREE_ERROR_H

#include "codegen_tree.h"

extern void tree_error_clear(tree_t *tree);
extern void tree_error_show(tree_t *tree, FILE* f);

extern void tree_info(tree_t *tree, tree_err_loc_t *loc, char *fmt, ...);
extern tree_err_t *tree_warning(tree_t *tree, tree_err_loc_t *loc, char *fmt, ...);
extern void tree_error(tree_t *tree, tree_err_loc_t *loc, char *fmt, ...);
extern void tree_panic(tree_t *tree, tree_err_loc_t *loc, char *fmt, ...);

#endif /* __TESTFARM_CODEGEN_TREE_ERROR_H */
