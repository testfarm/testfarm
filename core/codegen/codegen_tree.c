/****************************************************************************/
/* TestFarm                                                                 */
/* Test Suite code generator : test tree                                    */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 12-APR-2000                                                    */
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
#include <glib.h>

#include "codegen_script.h"
#include "codegen_tree.h"
#include "codegen_tree_error.h"
#include "codegen_tree_init.h"
#include "codegen_tree_scan.h"
#include "codegen_tree_precond.h"


/*******************************************************/
/* Tree data                                           */
/*******************************************************/

typedef struct {
  void *ptr;
  void (*unref)(void *);
} tree_data_t;


static gboolean tree_object_destroy_data(tree_data_t *data)
{
  if ( (data->unref != NULL) && (data->ptr != NULL) )
    data->unref(data->ptr);
  data->ptr = NULL;
  data->unref = NULL;
  free(data);

  return TRUE;
}


static void tree_object_destroy_all_data(tree_object_t *tobject)
{
  if ( tobject == NULL )
    return;
  if ( tobject->data_hash == NULL )
    return;
  g_hash_table_destroy(tobject->data_hash);
  tobject->data_hash = NULL;
}


void tree_object_set_data(tree_object_t *tobject, char *key, void *ptr, void (*unref)(void *))
{
  tree_data_t *data;

  if ( tobject == NULL )
    return;
  if ( tobject->data_hash == NULL )
    tobject->data_hash = g_hash_table_new_full(g_direct_hash, g_str_equal, NULL, (GDestroyNotify) tree_object_destroy_data);

  data = (tree_data_t *) malloc(sizeof(tree_data_t));
  data->ptr = ptr;
  data->unref = unref;
  g_hash_table_replace(tobject->data_hash, key, data);
}


void tree_object_unset_data(tree_object_t *tobject, char *key)
{
  if ( tobject == NULL )
    return;
  if ( tobject->data_hash == NULL )
    return;
  g_hash_table_remove(tobject->data_hash, key);
}


void *tree_object_get_data(tree_object_t *tobject, char *key)
{
  tree_data_t *data;

  if ( tobject == NULL )
    return NULL;
  if ( tobject->data_hash == NULL )
    return NULL;

  data = g_hash_table_lookup(tobject->data_hash, key);

  return (data != NULL) ? data->ptr : NULL;
}


/*******************************************************/
/* Tree scanning                                       */
/*******************************************************/

void tree_foreach(tree_t *tree, tree_func_t *func, void *arg)
{
  int i;

  if ( tree == NULL )
    return;

  for (i = 0; i < tree->nmemb; i++) {
    tree_object_t *object = tree->items[i]->object;

    func(object, arg);
  }
}


/*******************************************************/
/* Tree items                                          */
/*******************************************************/

static tree_item_t *tree_item_link(tree_item_t *item);
static void tree_item_unlink(tree_item_t **pitem);
static int tree_item_circular(tree_item_t *item);
static void tree_item_simu(tree_item_t *item);
static void tree_object_link_seq(tree_object_t *tobject, tree_object_t *end_jump, tree_object_t *break_jump);
static int tree_item_has_seq(tree_item_t *item);


/*================= TEST CASE =========================*/

static tree_case_t *tree_case_new(void)
{
  tree_case_t *tcase = (tree_case_t *) malloc(sizeof(tree_case_t));
  tcase->num = 0;
  tcase->preconds = NULL;
  tcase->criticity = CRITICITY_NONE;
  tcase->loop = 1;
  tcase->script = NULL;

  tcase->breakpoint = 0;
  tcase->simu = VERDICT_UNEXECUTED;
  tcase->result.verdict = VERDICT_UNEXECUTED;
  tcase->result.criticity = CRITICITY_NONE;
  tcase->exec_count = 0;

  return tcase;
}


static void tree_case_destroy(tree_case_t *tcase)
{
  tree_precond_destroy(tcase);

  if ( tcase->script != NULL )
    script_destroy(tcase->script);

  free(tcase);
}


static void tree_case_link(tree_case_t *tcase, tree_object_t *object)
{
  tree_item_t *item = object->parent_item;
  char *name = item->name;

  /* Check scripts */
  if ( tcase->script == NULL )
    tree_error(item->parent_tree, &(item->loc), "CASE '%s': Missing script definition", name);

  /* Link precondition expression */
  tree_precond_link(tcase, item->parent_tree, name);
}


/*================= TEST SEQUENCE =========================*/

static tree_seq_t *tree_seq_new(void)
{
  tree_seq_t *tseq = (tree_seq_t *) malloc(sizeof(tree_seq_t));
  tseq->nnodes = 0;
  tseq->nodes = NULL;

  return tseq;
}


static void tree_seq_destroy(tree_seq_t *tseq)
{
  if ( tseq->nodes != NULL ) {
    int i;

    for (i = 0; i < tseq->nnodes; i++)
      tree_item_destroy(tseq->nodes[i]);

    free(tseq->nodes);
  }
  free(tseq);
}


void tree_seq_feed(tree_seq_t *tseq, tree_item_t *item)
{
  tseq->nodes = (tree_item_t **) realloc(tseq->nodes, sizeof(tree_item_t *) * (tseq->nnodes + 1));
  tseq->nodes[(tseq->nnodes)++] = item;
}


static void tree_seq_link(tree_seq_t *tseq, tree_object_t *object)
{
  int i;

  for (i = 0; i < tseq->nnodes; i++) {
    tree_item_link(tseq->nodes[i]);
    if ( tseq->nodes[i]->object != NULL )
      tseq->nodes[i]->object->parent_seq = object;
  }
}


static void tree_seq_unlink(tree_seq_t *tseq, tree_object_t *object)
{
  int i;

  for (i = 0; i < tseq->nnodes; i++) {
    tree_item_t *item = tseq->nodes[i];

    if ( item != NULL ) {
      if ( item->object == object ) {
        tree_item_unlink(&(tseq->nodes[i]));
      }
    }
  }
}


static void tree_seq_link_seq(tree_seq_t *tseq, tree_object_t *end_jump)
{
  int i;

  for (i = 0; i < tseq->nnodes; i++) {
    tree_object_t *ej;

    if ( i < (tseq->nnodes-1) )
      ej = tseq->nodes[i+1]->object;
    else
      ej = end_jump;

    tree_object_link_seq(tseq->nodes[i]->object, ej, end_jump);
  }
}


static void tree_seq_foreach(tree_seq_t *tseq, tree_func_t *func, void *arg)
{
  int i;

  for (i = 0; i < tseq->nnodes; i++) {
    if ( tseq->nodes[i] != NULL )
      tree_object_foreach(tseq->nodes[i]->object, func, arg);
  }
}


static int tree_seq_circular(tree_seq_t *tseq)
{
  int i;

  for (i = 0; i < tseq->nnodes; i++) {
    if ( tree_item_circular(tseq->nodes[i]) )
      return 1;
  }

  return 0;
}


static void tree_seq_simu(tree_seq_t *tseq)
{
  int i;

  for (i = 0; i < tseq->nnodes; i++)
    tree_item_simu(tseq->nodes[i]);
}


static tree_object_t *tree_seq_find(tree_seq_t *tseq)
{
  int i;

  for (i = 0; i < tseq->nnodes; i++) {
    tree_object_t *ret = tree_object_find(tseq->nodes[i]->object);
    if ( ret != NULL )
      return ret;
  }

  return NULL;
}


static int tree_seq_has_seq(tree_seq_t *tseq)
{
  int i;
  int retval = 0;

  for (i = 0; (i < tseq->nnodes) && (retval == 0); i++)
    retval |= tree_item_has_seq(tseq->nodes[i]);

  return retval;
}


/*================= TREE OBJECT =========================*/

tree_object_t *tree_object_new(tree_object_type_t type)
{
  tree_object_t *tobject = (tree_object_t *) malloc(sizeof(tree_object_t));

  tobject->type = type;

  switch ( type ) {
  case TYPE_CASE:
    tobject->d.Case = tree_case_new();
    break;
  case TYPE_SEQ:
    tobject->d.Seq  = tree_seq_new();
    break;
  default :
    tobject->d.ptr = NULL;
  }

  tobject->key = 0;
  tobject->parent_item = NULL;
  tobject->parent_seq = NULL;
  tobject->end_jump = NULL;
  tobject->break_jump = NULL;
  tobject->enter = 0;
  tobject->data_hash = NULL;
  tobject->flags = 0;

  return tobject;
}


void tree_object_destroy(tree_object_t *tobject)
{
  tree_object_destroy_all_data(tobject);

  switch ( tobject->type ) {
  case TYPE_CASE:
    tree_case_destroy(tobject->d.Case);
    break;
  case TYPE_SEQ:
    tree_seq_destroy(tobject->d.Seq);
    break;
  default : break;
  }

  free(tobject);
}


static void tree_object_link(tree_object_t *tobject)
{
  switch ( tobject->type ) {
  case TYPE_CASE:
    tree_case_link(tobject->d.Case, tobject);
    break;
  case TYPE_SEQ:
    tree_seq_link(tobject->d.Seq, tobject);
    break;
  default :
    /* Nothing to link */
    break;
  }
}


static void tree_object_unlink(tree_object_t *tobject, tree_object_t *object)
{
  if ( (tobject == NULL) || (object == NULL) )
    return;

  switch ( tobject->type ) {
  case TYPE_SEQ:
    tree_seq_unlink(tobject->d.Seq, object);
    break;
  default :
    /* Nothing to unlink */
    break;
  }
}


static void tree_object_link_seq(tree_object_t *tobject, tree_object_t *end_jump, tree_object_t *break_jump)
{
  tobject->end_jump = end_jump;
  tobject->break_jump = break_jump;

  switch ( tobject->type ) {
  case TYPE_SEQ:
    tree_seq_link_seq(tobject->d.Seq, end_jump);
    break;
  default :
    /* Nothing to link */
    break;
  }
}


int tree_object_has_seq(tree_object_t *tobject)
{
  int retval = 0;

  switch ( tobject->type ) {
  case TYPE_SEQ:
    retval = tree_seq_has_seq(tobject->d.Seq);
    break;
  default :
    break;
  }

  return retval;
}


int tree_object_force_skip(tree_object_t *tobject, int state)
{
  switch ( tobject->type ) {
  case TYPE_CASE:
  case TYPE_SEQ:
    if ( state < 0 )
      state = (tobject->flags & FLAG_FORCE_SKIP) ? 0 : 1;
    if ( state )
      tobject->flags |= FLAG_FORCE_SKIP;
    else
      tobject->flags &= ~FLAG_FORCE_SKIP;
    break;
  default:
    break;
  }

  return state;
}


void tree_object_foreach(tree_object_t *tobject, tree_func_t *func, void *arg)
{
  if ( tobject == NULL )
    return;
  if ( func == NULL )
    return;

  func(tobject, arg);

  switch ( tobject->type ) {
  case TYPE_SEQ:
    tree_seq_foreach(tobject->d.Seq, func, arg);
    break;
  default :
    break;
  }

}


static int tree_object_circular(tree_object_t *tobject)
{
  int ret = 0;

  if ( tobject == NULL )
    return 0;

  if ( tobject->enter > 0 )
    return 1;

  tobject->enter++;

  switch ( tobject->type ) {
  case TYPE_SEQ:
    ret = tree_seq_circular(tobject->d.Seq);
    break;
  default :
    break;
  }

  tobject->enter--;

  return ret;
}


static void tree_object_simu(tree_object_t *tobject)
{
  if ( tobject == NULL )
    return;

  tobject->enter++;

  switch ( tobject->type ) {
  case TYPE_CASE:
    {
      tree_case_t *tcase = tobject->d.Case;
      int valid = (!(tobject->flags & FLAG_FORCE_SKIP)) && tree_precond_evaluate(tcase, tcase->simu);

      if ( valid )
	tcase->exec_count++;
    }
    break;
  case TYPE_SEQ:
    tree_seq_simu(tobject->d.Seq);
    break;
  default :
    break;
  }
}


tree_object_t *tree_object_find(tree_object_t *tobject)
{
  tree_object_t *ret;

  if ( tobject == NULL )
    return NULL;

  switch ( tobject->type ) {
  case TYPE_CASE:
    ret = tobject;
    break;
  case TYPE_SEQ:
    if ( tobject->flags & FLAG_FORCE_SKIP )
      ret = tree_object_find(tobject->end_jump);
    else
      ret = tree_seq_find(tobject->d.Seq);
    break;
  default :
    ret = NULL;
    break;
  }

  return ret;
}


tree_object_t *tree_object_executed(tree_object_t *object, tree_verdict_t verdict, int criticity)
{
  tree_case_t *tcase;
  tree_object_t *branch;

  if ( object == NULL )
    return NULL;
  if ( object->type != TYPE_CASE )
    return NULL;
  tcase = object->d.Case;

  /* Set criticity level */
  if ( criticity < 0 )
    criticity = tcase->criticity;

  /* Set verdict value */
  tcase->result.verdict = verdict;
  tcase->result.criticity = criticity;

  /* Go to next test case */
  branch = object->end_jump;

  /* Break parent sequence if a XXXX_IF_FAILED flag is set */
  if ( (verdict == VERDICT_FAILED) || (verdict == VERDICT_INCONCLUSIVE) ) {
    if ( object->flags & FLAG_ABORT_IF_FAILED )
      branch = NULL;
    else if ( object->flags & FLAG_BREAK_IF_FAILED )
      branch = object->break_jump;
  }

  /* Seek for the first valid Test Case */
  while ( (branch != NULL) && (branch->type == TYPE_SEQ) ) {
    branch = branch->d.Seq->nodes[0]->object;
  }

  return branch;
}


int tree_object_precond(tree_object_t *object)
{
  int ret = 1;

  if ( object->flags & FLAG_FORCE_SKIP ) {
    ret = 0;
  }
  else {
    if ( object->type == TYPE_CASE ) {
      tree_case_t *tcase = object->d.Case;
      ret = tree_precond_evaluate(tcase, tcase->result.verdict);
    }
  }

  return ret;
}


/*================= TREE ITEM =========================*/

tree_item_t *tree_item_new(char *name, char *comment, tree_t *tree, tree_object_t *object,
                           tree_err_loc_t *loc)
{
  tree_item_t *item;

  item = (tree_item_t *) malloc(sizeof(tree_item_t));
  item->name = (name == NULL) ? NULL : strdup(name);
  item->comment = (comment == NULL) ? NULL : strdup(comment);
  item->reference = NULL;
  item->parent_tree = tree;
  item->object = object;
  if ( object != NULL ) {
    object->key = (tree->keycount)++;
    object->parent_item = item;
    if ( object->type == TYPE_CASE )
      object->d.Case->num = (tree->casecount)++;
  }
  item->links = 0;

  /* Setup error reporting info */
  item->loc.filename = NULL;
  item->loc.lineno = 0;
  if ( loc != NULL ) {
    if ( loc->filename != NULL )
      item->loc.filename = strdup(loc->filename);
    item->loc.lineno = loc->lineno;
  }

  return item;
}


void tree_item_destroy(tree_item_t *item)
{
  if ( item == NULL )
    return;

  if ( item->name != NULL )
    free(item->name);
  if ( item->comment != NULL )
    free(item->comment);
  if ( item->reference != NULL )
    free(item->reference);
  if ( item->loc.filename != NULL )
    free(item->loc.filename);
  free(item);
}


static tree_item_t *tree_item_link(tree_item_t *item)
{
  tree_item_t *target;

  if ( item == NULL )
    return NULL;

  /* Retrieve linked item */
  if ( (target = tree_retrieve(item->parent_tree, item->name)) != NULL ) {
    /* Perform object link */
    if ( target->links == 0 ) {
      item->object = target->object;
      (target->links)++;
    }
    else {
      tree_error(item->parent_tree, &(item->loc), "Multiple reference to node '%s'", target->name);
    }

    /* Check target is not a tree root */
    if ( target == item->parent_tree->head ) {
      tree_error(item->parent_tree, &(item->loc), "Link target for '%s' is the tree root sequence", item->name);
    }
  }
  else {
    tree_error(item->parent_tree, &(item->loc), "Cannot resolve link to '%s'", item->name);
  }

  return target;
}


static void tree_item_unlink(tree_item_t **pitem)
{
  tree_item_t *item = *pitem;

  if ( item == NULL )
    return;

  tree_item_destroy(item);
  *pitem = NULL;
}


static int tree_item_has_seq(tree_item_t *item)
{
  if ( item == NULL )
    return 0;
  if ( item->object == NULL )
    return 0;
  if ( item->object->type == TYPE_SEQ )
    return 1;
  return tree_object_has_seq(item->object);
}


static void tree_stack_push(tree_t *tree, tree_object_t *object);

static int tree_item_circular(tree_item_t *item)
{
  if ( item == NULL )
    return 0;

  if ( tree_object_circular(item->object) ) {
    tree_stack_push(item->parent_tree, item->object);
    return 1; 
  }

  return 0;
}


static void tree_item_simu(tree_item_t *item)
{
  if ( item == NULL )
    return;

  tree_object_simu(item->object);
}


/*******************************************************/
/* Tree                                                */
/*******************************************************/

static void tree_stack_reset(tree_t *tree)
{
  tree->stack_index = 0;
  if ( tree->stack_size > 0 )
    memset(tree->stack, 0, sizeof(tree_object_t *) * tree->stack_size);
}


static void tree_stack_push(tree_t *tree, tree_object_t *object)
{
  if ( tree->stack_index >= tree->stack_size ) {
    tree->stack_size += 256;
    tree->stack = (tree_object_t **) realloc(tree->stack, sizeof(tree_object_t *) * tree->stack_size);
  }

  tree->stack[(tree->stack_index)++] = object;
}


static void tree_stack_free(tree_t *tree)
{
  if ( tree->stack != NULL )
    free(tree->stack);
  tree->stack = NULL;
  tree->stack_index = 0;
  tree->stack_size = 0;
}


tree_item_t *tree_add(tree_t *tree, tree_item_t *item)
{
  tree->items = (tree_item_t **) realloc(tree->items, sizeof(tree_item_t *) * (tree->nmemb + 1));
  tree->items[(tree->nmemb)++] = item;

  return item;
}


void tree_remove(tree_t *tree, tree_item_t *item)
{
  int i;
  int offset = -1;

  for (i = 0; i < tree->nmemb; i++) {
    if ( tree->items[i] == item ) {
      offset = i;
      tree->items[i] = NULL;
    }
    else if ( (offset >= 0) && (i > offset) ) {
      tree->items[i-1] = tree->items[i];
      tree->items[i] = NULL;
    }
  }

  if ( offset >= 0 ) {
    (tree->nmemb)--;
  }
}


tree_item_t *tree_retrieve(tree_t *tree, char *name)
{
  tree_item_t *item;
  int i;

  /* Search for requested item name */
  item = NULL;
  for (i = 0; (i < tree->nmemb) && (item == NULL); i++) {
    item = tree->items[i];
    if ( strcmp(item->name, name) != 0 )
      item = NULL;
  }

  return item;
}


tree_t *tree_new(void)
{
  /* Alloc tree descriptor */
  tree_t *tree = (tree_t *) malloc(sizeof(tree_t));
  tree->nmemb = 0;
  tree->items = NULL;
  tree->loc.filename = NULL;
  tree->loc.lineno = 0;
  tree->errcount = 0;
  tree->warncount = 0;
  tree->errlist = NULL;
  tree->system = NULL;
  tree->t0 = 0;
  tree->keycount = 1;
  tree->casecount = 0;
  tree->head = NULL;
  tree->current = NULL;
  tree->stack = NULL;
  tree->stack_size = 0;
  tree->stack_index = 0;

  return tree;
}


void tree_destroy(tree_t *tree, int save_cfg)
{
  int i;

  if ( tree == NULL )
    return;

  if ( tree->loc.filename != NULL ) {
    if ( save_cfg )
      tree_init_save(tree);
    free(tree->loc.filename);
  }
  tree->loc.filename = NULL;

  tree_error_clear(tree);

  for (i = 0; i < tree->nmemb; i++) {
    tree_item_t *item = tree->items[i];

    if ( item->object != NULL )
      tree_object_destroy(item->object);
    tree_item_destroy(item);
  }
  free(tree->items);

  if ( tree->stack != NULL )
    free(tree->stack);

  if ( tree->system != NULL )
    free(tree->system);

  free(tree);
}


static void tree_check_lost(tree_object_t *object)
{
  tree_item_t *item = object->parent_item;

  if ( (item->links == 0) &&
       (item->loc.lineno > 0) &&
       (item != item->parent_tree->head) )
    tree_error(item->parent_tree, &(item->loc), "'%s' is linked from nowhere", item->name);
}


static void tree_link(tree_t *tree)
{
  int ret;

  /* Link node together */
  tree_foreach(tree, (tree_func_t *) tree_object_link, NULL);

  /* Check for lost nodes */
  tree_foreach(tree, (tree_func_t *) tree_check_lost, NULL);

  /* Check whether a TREE node is defined */
  if ( tree->head == NULL ) {
    tree->loc.lineno = 0;
    tree_error(tree, &(tree->loc), "No TREE node is defined");
  }

  /* Check for circular links */
  do {
    /* Init stack */
    tree_stack_reset(tree);

    ret = tree_item_circular(tree->head);
    if ( ret ) {
      tree_object_t *object = tree->stack[1];
      tree_item_t *item = object->parent_item;

      /* Report circular link error */
      tree_error(tree, &(item->loc),
                 "'%s' makes a circular link through '%s'",
                 item->name, tree->stack[0]->parent_item->name);

      /* Kill this bloody link */
      tree_object_unlink(object, tree->stack[0]);
    }
  } while ( ret );

  /* Free tree stack */
  tree_stack_free(tree);
}


static void tree_simu_clear(tree_object_t *object, void *arg)
{
  object->enter = 0;
  if ( object->type == TYPE_CASE ) {
    object->d.Case->simu = VERDICT_UNEXECUTED;
    object->d.Case->exec_count = 0;
  }
}


static void tree_simu_analyse(tree_object_t *object, void *arg)
{
  tree_item_t *item = object->parent_item;
  tree_t *tree = item->parent_tree;
  char *name = item->name;
  tree_err_t *err = NULL;

  if ( object->enter == 0 ) {
    err = tree_warning(tree, &(item->loc), "Test %s '%s' will never be executed", name, (object->type == TYPE_CASE) ? "Case":"Sequence");
  }
  else if ( object->type == TYPE_CASE ) {
    tree_case_t *tcase = object->d.Case;

    if ( tcase->exec_count == 0 ) {
      err = tree_warning(tree, &(item->loc), "Preconditions will make Test Case '%s' always be skipped", name);
    }
  }

  if ( err != NULL )
    err->object = object;
}


void tree_simu(tree_t *tree)
{
  /* Clear simulation data */
  tree_foreach(tree, tree_simu_clear, NULL);

  /* Perform simulation */
  tree_item_simu(tree->head);

  /* Analyse simulation result */
  tree_foreach(tree, tree_simu_analyse, NULL);
}


tree_t *tree_load(tree_t *tree, char *filename)
{
  int filename_len = strlen(filename);
  char filename_buf[filename_len+strlen(TREE_DEF_SUFFIX)+1];
  char *basename, *suffix;

  /* Clear file information */
  if ( tree->loc.filename != NULL )
    free(tree->loc.filename);
  tree->loc.filename = NULL;
  tree->loc.lineno = 0;

  /* Clear error list */
  tree_error_clear(tree);

  /* Initial load state */
  tree->current = NULL;

  /* Determine file type */
  strcpy(filename_buf, filename);
  filename = filename_buf;

  basename = strrchr(filename, G_DIR_SEPARATOR);
  if ( basename == NULL )
    basename = filename;

  suffix = strrchr(basename, '.');
  if ( suffix == NULL ) {
    suffix = filename + filename_len;
    strcpy(suffix, TREE_DEF_SUFFIX);
  }

  /* Load tree file */
  if ( tree_scan_root(tree, filename) )
    return NULL;

  tree->loc.filename = strdup(filename);

  /* Check tree health */
  if ( tree->errcount > 0 )
    return tree;

  /* Link whole tree */
  tree_link(tree);
  if ( tree->errcount > 0 )
    return tree;

  /* Setup parent sequence links */
  tree_object_link_seq(tree->head->object, NULL, NULL);

  /* Check for dead execution nodes */
  tree_simu(tree);

  /* Load initial breakpoint and force_skip states from init file */
  tree_init_load(tree);

  tree->current = tree->head;
  tree->t0 = time(NULL);

  return tree;
}


/*******************************************************/
/* Tree Scenarii and Test Cases counting               */
/*******************************************************/

void tree_count(tree_t *tree, int *_ncase, int *_nscenario)
{
  int ncase = 0;
  int nscenario = 0;
  int i;

  for (i = 0; i < tree->nmemb; i++) {
    tree_object_t *object = tree->items[i]->object;
    
    switch ( object->type ) {
    case TYPE_CASE:
      ncase++;
      break;
    case TYPE_SEQ:
      if ( ! tree_seq_has_seq(object->d.Seq) )
	nscenario++;
      break;
    default:
      break;
    }
  }

  if ( _ncase != NULL )
    *_ncase = ncase;
  if ( _nscenario != NULL )
    *_nscenario = nscenario;
}


/*******************************************************/
/* Tree Object lookup                                  */
/*******************************************************/

tree_object_t *tree_lookup(tree_t *tree, unsigned long key)
{
  int i;

  if ( key == 0 )
    return NULL;

  /* Search for requested object key */
  for (i = 0; i < tree->nmemb; i++) {
    tree_object_t *object = tree->items[i]->object;
    if ( (object != NULL) && (object->key == key) )
      return object;
  }

  return NULL;
}


tree_object_t *tree_lookup_name(tree_t *tree, char *name)
{
  tree_object_t *object = NULL;

  if ( name != NULL ) {
    tree_item_t *item = tree_retrieve(tree, name);
    if ( item != NULL )
      object = item->object;
  }

  return object;
}
