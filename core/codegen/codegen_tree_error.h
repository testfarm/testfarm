/****************************************************************************/
/* TestFarm                                                                 */
/* Test Suite code generator : Test Tree Error Recording                    */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 22-JUN-2000                                                    */
/****************************************************************************/

/* $Revision: 42 $ */
/* $Date: 2006-06-03 15:30:01 +0200 (sam., 03 juin 2006) $ */

#ifndef __CODEGEN_TREE_ERROR_H
#define __CODEGEN_TREE_ERROR_H

#include "codegen_tree.h"

extern void tree_error_clear(tree_t *tree);
extern void tree_error_show(tree_t *tree, FILE* f);

extern void tree_info(tree_t *tree, tree_err_loc_t *loc, char *fmt, ...);
extern tree_err_t *tree_warning(tree_t *tree, tree_err_loc_t *loc, char *fmt, ...);
extern void tree_error(tree_t *tree, tree_err_loc_t *loc, char *fmt, ...);
extern void tree_panic(tree_t *tree, tree_err_loc_t *loc, char *fmt, ...);

#endif /* __CODEGEN_TREE_ERROR_H */
