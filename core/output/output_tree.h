/****************************************************************************/
/* TestFarm                                                                 */
/* Output Tree info                                                         */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 19-FEB-2001                                                    */
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

#ifndef __TESTFARM_OUTPUT_TREE_H
#define __TESTFARM_OUTPUT_TREE_H

#include "codegen.h"

typedef struct {
  tree_t *tree;
  long long begin;    /* Start date of the test suite */
  char *name;         /* Name of the test tree */
  char *description;  /* Description of the test tree */
  char *reference;    /* Reference of Test Suite specification */
  char *operator;     /* Operator name */
  char *release;      /* Release name */
} output_tree_t;

extern void output_tree_init(output_tree_t *tree, tree_t *codegen_tree);
extern void output_tree_clear(output_tree_t *tree);
extern void output_tree_name(output_tree_t *tree, char *name);
extern void output_tree_operator(output_tree_t *tree, char *name);
extern void output_tree_release(output_tree_t *tree, char *name);

#endif /* __TESTFARM_OUTPUT_TREE_H */
