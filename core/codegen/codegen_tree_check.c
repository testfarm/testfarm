/****************************************************************************/
/* Basil Dev - TestFarm                                                     */
/* Test Suite generator : Files modification checker                        */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 13-MAY-2004                                                    */
/****************************************************************************/

/*
 * $Revision: 42 $
 * $Date: 2006-06-03 15:30:01 +0200 (sam., 03 juin 2006) $
 */

#include <stdio.h>
#include <glib.h>
#include <sys/stat.h>

#include "codegen_script.h"
#include "codegen_tree.h"
#include "codegen_tree_check.h"


static void tree_check_object(tree_object_t *obj, GList **plist)
{
  tree_item_t *item = obj->parent_item;

  switch ( obj->type ) {
  case TYPE_SEQ:
    if ( item->loc.filename != NULL ) {
      struct stat stat1;

      if ( stat(item->loc.filename, &stat1) == 0 ) {
        if ( stat1.st_mtime > item->parent_tree->t0 )
          *plist = g_list_append(*plist, obj);
      }
    }
    break;

  case TYPE_CASE:
    if ( script_check_wiz(obj->d.Case->script) ) {
      *plist = g_list_append(*plist, obj);
    }
    break;

  default:
    break;
  }
}


GList *tree_check(tree_t *tree)
{
  GList *list = NULL;
  tree_foreach(tree, (tree_func_t *) tree_check_object, &list);
  return list;
}
