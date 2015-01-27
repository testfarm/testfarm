/****************************************************************************/
/* TestFarm                                                                 */
/* Test Suite code generator : Test Tree structure display                  */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 12-NOV-2006                                                    */
/****************************************************************************/

/*
 * $Revision: 278 $
 * $Date: 2006-11-12 15:47:31 +0100 (dim., 12 nov. 2006) $
 */

#ifndef __CODEGEN_TREE_SHOW_H
#define __CODEGEN_TREE_SHOW_H

/*================= TREE OBJECT =========================*/
extern void tree_object_show(tree_object_t *tobject, FILE *f);

/*================= TREE ITEM =========================*/
extern void tree_item_show_name(tree_item_t *item, FILE *f);
extern void tree_item_show(tree_item_t *item, FILE *f);

/*================= TREE =========================*/
extern void tree_show(tree_t *tree, FILE *f);

#endif /* __CODEGEN_TREE_SHOW_H */
