/****************************************************************************/
/* TestFarm                                                                 */
/* Test Suite code generator : Test Case precondition                       */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 11-MAY-2004                                                    */
/****************************************************************************/

/*
 * $Revision: 761 $
 * $Date: 2007-10-01 14:13:31 +0200 (lun., 01 oct. 2007) $
 */

#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <glib.h>

#include "useful.h"
#include "codegen_tree.h"
#include "codegen_tree_error.h"
#include "codegen_tree_precond.h"


typedef struct {
  tree_err_loc_t loc;
  char *name;
  int neq;
  tree_verdict_t value;
  tree_case_t *tcase;
} tree_precond_t;


static const char *tree_precond_str[] = {"PASSED", "FAILED", "INCONCLUSIVE", "SKIP", "EXECUTED"};


static void tree_precond_destroy_item(tree_precond_t *precond)
{
  if ( precond->loc.filename != NULL )
    free(precond->loc.filename);
  if ( precond->name != NULL )
    free(precond->name);
  free(precond);
}


void tree_precond_destroy(tree_case_t *tcase)
{
  g_list_foreach(tcase->preconds, (GFunc) tree_precond_destroy_item, NULL);
  g_list_free(tcase->preconds);
  tcase->preconds = NULL;
}


static char *tree_precond_parse(char *s, tree_precond_t *precond)
{
  tree_verdict_t value;
  char *end;
  char *op;

  /* Skip leading blanks */
  s = strskip_spaces(s);
  if ( *s == NUL )
    return s;

  /* Find end of subexpression */
  if ( (end = strchr(s, ',')) != NULL )
    *(end++) = NUL;
  else
    end = s + strlen(s);

  /* Retrieve operator */
  if ( (op = strstr(s, "!=")) != NULL ) {
    precond->neq = 1;
    *op = NUL;
    op += 2;
  }
  else if ( (op = strstr(s, "=")) != NULL ) {
    precond->neq = 0;
    *op = NUL;
    op += 1;
  }
  else
    return NULL;

  /* Retrieve name */
  *strskip_chars(s) = NUL;
  precond->name = strdup(s);

  /* Retrieve value */
  op = strupper(strskip_spaces(op));
  *strskip_chars(op) = NUL;
  for (value = VERDICT_PASSED; value < VERDICT_MAX; value++) {
    if ( strncmp(tree_precond_str[value], op, strlen(op)) == 0 )
      break;
  }
  if ( value >= VERDICT_MAX )
    return NULL;
  precond->value = value;

  precond->tcase = NULL;

  return end; 
}


int tree_precond_new(tree_case_t *tcase, char *str, tree_err_loc_t *loc)
{
  char *s = str;

  while ( (s != NULL) && (*s != NUL) ) {
    /* Allocate new item */
    tree_precond_t *precond = (tree_precond_t *) malloc(sizeof(tree_precond_t));

    precond->loc.filename = NULL;
    precond->loc.lineno = 0;
    if ( loc != NULL ) {
      if ( loc->filename != NULL )
        precond->loc.filename = strdup(loc->filename);
      precond->loc.lineno = loc->lineno;
    }
    precond->name = NULL;

    /* Parse item */
    s = tree_precond_parse(s, precond);

    /* Check error */
    if ( s == NULL ) {
      tree_precond_destroy(tcase);
      return -1;
    }

    tcase->preconds = g_list_append(tcase->preconds, precond);
  }

  return 0;
}


typedef struct {
  tree_t *tree;
  char *name;
  int err;
} tree_precond_link_data_t;

static void tree_precond_link_item(tree_precond_t *precond, tree_precond_link_data_t *p)
{
  tree_item_t *target = tree_retrieve(p->tree, precond->name);

  if ( (target != NULL) && (target->object->type == TYPE_CASE) )
    precond->tcase = target->object->d.Case;
  else {
    tree_error(p->tree, &(precond->loc), "CASE '%s': precondition refers to '%s', which is not a CASE node", p->name, precond->name);
    (p->err)++;
  }
}


int tree_precond_link(tree_case_t *tcase, tree_t *tree, char *name)
{
  tree_precond_link_data_t d = {
    tree : tree,
    name: name,
    err : 0,
  };

  g_list_foreach(tcase->preconds, (GFunc) tree_precond_link_item, &d);

  return d.err;
}


int tree_precond_evaluate(tree_case_t *tcase, tree_verdict_t verdict)
{
  GList *p;

  p = tcase->preconds;
  while ( p != NULL ) {
    tree_precond_t *precond = p->data;

    if ( precond->neq ) {
      if ( verdict == precond->value )
        return 0;
    }
    else {
      if ( verdict != precond->value )
        return 0;
    }

    p = p->next;
  }

  return 1;
}


void tree_precond_show(tree_case_t *tcase, FILE *f)
{
  if ( tcase->preconds != NULL ) {
    GList *p = tcase->preconds;
    while ( p != NULL ) {
      tree_precond_t *precond = p->data;

      fprintf(f, " (%s %s %s)", precond->name, precond->neq ? "!=":"=", tree_precond_str[precond->value]);

      p = p->next;

      if ( p != NULL )
        fprintf(f, ",");
    }
  }
  else {
    fprintf(f, " (none)");
  }
}
