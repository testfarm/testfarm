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

#include <stdio.h>
#include <string.h>
#include <malloc.h>

#include "output_tree.h"


void output_tree_init(output_tree_t *tree, tree_t *codegen_tree)
{
  /* Init output tree structure */
  tree->tree = codegen_tree;
  tree->begin = 0;
  tree->name = strdup("?");
  tree->description = NULL;
  tree->reference = NULL;
  tree->operator = NULL;
  tree->release = NULL;

  /* Inherit codegen Tree settings if it is given */
  if ( codegen_tree == NULL )
    return;

  if ( tree->tree->head != NULL ) {
    if ( tree->tree->head->name != NULL )
      output_tree_name(tree, tree->tree->head->name);
    if ( tree->tree->head->comment != NULL )
      tree->description = strdup(tree->tree->head->comment);
    if ( tree->tree->head->reference != NULL )
      tree->reference = strdup(tree->tree->head->reference);
  }
}


void output_tree_clear(output_tree_t *tree)
{
  if ( tree->release != NULL )
    free(tree->release);
  tree->release = NULL;

  if ( tree->operator != NULL )
    free(tree->operator);
  tree->operator = NULL;

  if ( tree->reference != NULL )
    free(tree->reference);
  tree->reference = NULL;

  if ( tree->description != NULL )
    free(tree->description);
  tree->description = NULL;

  if ( tree->name != NULL )
    free(tree->name);
  tree->name = NULL;
}


void output_tree_name(output_tree_t *tree, char *name)
{
  if ( tree->name != NULL )
    free(tree->name);
  tree->name = NULL;
  if ( name != NULL )
    tree->name = strdup(name);
}


void output_tree_operator(output_tree_t *tree, char *name)
{
  if ( tree->operator != NULL )
    free(tree->operator);
  tree->operator = NULL;
  if ( name != NULL )
    tree->operator = strdup(name);
}


void output_tree_release(output_tree_t *tree, char *name)
{
  if ( tree->release != NULL )
    free(tree->release);
  tree->release = NULL;
  if ( name != NULL )
    tree->release = strdup(name);
}
