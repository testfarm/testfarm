/****************************************************************************/
/* TestFarm                                                                 */
/* Test Suite code generator : Test Case precondition                       */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 11-MAY-2004                                                    */
/****************************************************************************/

/*
 * $Revision: 42 $
 * $Date: 2006-06-03 15:30:01 +0200 (sam., 03 juin 2006) $
 */

#ifndef __CODEGEN_TREE_PRECOND_H
#define __CODEGEN_TREE_PRECOND_H

#include "codegen_tree.h"

extern int tree_precond_new(tree_case_t *tcase, char *str, tree_err_loc_t *loc);
extern void tree_precond_destroy(tree_case_t *tcase);

extern int tree_precond_link(tree_case_t *tcase, tree_t *tree, char *name);
extern int tree_precond_evaluate(tree_case_t *tcase, tree_verdict_t verdict);
extern void tree_precond_show(tree_case_t *tcase, FILE *f);

#endif /* __CODEGEN_TREE_PRECOND_H */
