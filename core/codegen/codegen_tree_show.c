/****************************************************************************/
/* TestFarm                                                                 */
/* Test Suite code generator : Test Tree structure display                  */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 12-NOV-2006                                                    */
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

#include "codegen_script.h"
#include "codegen_tree.h"
#include "codegen_tree_precond.h"


/*******************************************************/
/* Tree object display                                 */
/*******************************************************/

static void tree_case_show(tree_case_t *tcase, FILE *f)
{
  fprintf(f, "    Precondition:");
  tree_precond_show(tcase, f);
  fprintf(f, "\n");

  fprintf(f, "    Criticity: %s\n", criticity_id(tcase->criticity));
  fprintf(f, "    Loop: %d\n", tcase->loop);
  fprintf(f, "    Script:\n");
  script_show(tcase->script, f);
}


static void tree_seq_show(tree_seq_t *tseq, FILE *f)
{
  int i;

  fprintf(f, "    Branches: %d\n", tseq->nnodes);

  for (i = 0; i < tseq->nnodes; i++) {
    if ( tseq->nodes[i] != NULL )
      fprintf(f, "      %s\n", tseq->nodes[i]->name);
  }
}


void tree_object_show(tree_object_t *tobject, FILE *f)
{
  fprintf(f, "    Type: ");

  switch ( tobject->type ) {
  case TYPE_CASE:
    fprintf(f, "CASE\n");
    tree_case_show(tobject->d.Case, f);
    break;
  case TYPE_SEQ:
    if ( tobject->parent_item == tobject->parent_item->parent_tree->head )
      fprintf(f, "TREE\n");
    else
      fprintf(f, "SEQ\n");
    tree_seq_show(tobject->d.Seq, f);
    break;
  default :
    fprintf(f, "*UNKNOWN*\n");
    break;
  }

  /*fprintf(f, "    Parent: %s\n", (tobject->parent_item == NULL) ? "(none)" : tobject->parent_item->name);*/
  fprintf(f, "    ForceSkip: %s\n", (tobject->flags & FLAG_FORCE_SKIP) ? "yes":"no");
  fprintf(f, "    Parent-Seq: %s\n", (tobject->parent_seq == NULL) ? "(none)" : tobject->parent_seq->parent_item->name);
  fprintf(f, "    End-jump: %s\n", (tobject->end_jump == NULL) ? "(none)" : tobject->end_jump->parent_item->name);
  fprintf(f, "    Break-jump: %s\n", (tobject->break_jump == NULL) ? "(none)" : tobject->break_jump->parent_item->name);
}


/*******************************************************/
/* Tree item display                                   */
/*******************************************************/

void tree_item_show_name(tree_item_t *item, FILE *f)
{
  if ( item == NULL )
    fprintf(f, "(none)\n");
  else {
    fprintf(f, "%s \"%s\"\n", item->name, (item->comment == NULL) ? "" : item->comment);
    if ( item->reference != NULL )
      fprintf(f, "    Reference: \"%s\"\n", item->reference);
  }
}

void tree_item_show(tree_item_t *item, FILE *f)
{
  fprintf(f, "  ");
  tree_item_show_name(item, f);
  if ( item->loc.filename != NULL )
    fprintf(f, "    Location: %s:%d\n", item->loc.filename, item->loc.lineno);
  fprintf(f, "    Links: %d\n", item->links);
  tree_object_show(item->object, f);
}


/*******************************************************/
/* Tree content display                                */
/*******************************************************/

void tree_show(tree_t *tree, FILE *f)
{
  int i;

  if ( tree == NULL )
    return;

  fprintf(f, "File: %s\n", (tree->loc.filename == NULL) ? "(none)" : tree->loc.filename);
  fprintf(f, "Errors: %d\n", tree->errcount);
  fprintf(f, "Items: %d\n", tree->nmemb);

  for (i = 0; i < tree->nmemb; i++)
    tree_item_show(tree->items[i], f);
}
