/****************************************************************************/
/* TestFarm                                                                 */
/* Test Suite code generator : Test Tree SKIP/BREAKPOINT Preload            */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 27-DEC-2000                                                    */
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

#ifndef __TESTFARM_CODEGEN_TREE_INIT_H
#define __TESTFARM_CODEGEN_TREE_INIT_H

#include "codegen_tree.h"

extern void tree_init_load(tree_t *tree);
extern void tree_init_save(tree_t *tree);

#endif /* __TESTFARM_CODEGEN_TREE_INIT_H */
