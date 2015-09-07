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
