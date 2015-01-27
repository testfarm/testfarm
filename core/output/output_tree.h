/****************************************************************************/
/* TestFarm                                                                 */
/* Output Tree info                                                         */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 19-FEB-2001                                                    */
/****************************************************************************/

/*
 * $Revision: 42 $
 * $Date: 2006-06-03 15:30:01 +0200 (sam., 03 juin 2006) $
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
