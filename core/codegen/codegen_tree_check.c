/****************************************************************************/
/* Basil Dev - TestFarm                                                     */
/* Test Suite generator : Files modification checker                        */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 13-MAY-2004                                                    */
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
