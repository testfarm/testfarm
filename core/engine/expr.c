/****************************************************************************/
/* TestFarm                                                                 */
/* Test Engine - Trigger expression evaluator                               */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 11-SEP-2003                                                    */
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

#include "useful.h"
#include "debug.h"
#include "expr.h"


static char *expr_operators[] = { "&", "|", NULL };
static int expr_priorities[]  = {  2,   1,  0    };


/*================================================*/
/* Expression item                                */
/*================================================*/

static expr_item_t *expr_item_alloc(int op, char *value)
{
  expr_item_t *item = (expr_item_t *) malloc(sizeof(expr_item_t));
  item->op = op;
  item->value = (value != NULL) ? strdup(value) : NULL;
  item->data = NULL;

  if ( item->value != NULL )
    strupper(item->value);

  return item;
}

static void expr_item_free(expr_item_t *item)
{
  if ( item == NULL )
    return;
  if ( item->value != NULL )
    free(item->value);
  free(item);
}


/*================================================*/
/* Expression list                                */
/*================================================*/

static GList *expr_add(GList *list, int op, char *value)
{
  return g_list_append(list, expr_item_alloc(op, value));
}


void expr_free(GList *list)
{
  g_list_foreach(list, (GFunc) expr_item_free, NULL);
  g_list_free(list);
}


static char *expr_parse(GList **plist, char *str, int op0)
{
  char *left = str;
  int op1 = EXPR_NOP;
  char *p = str;

  while ( (op1 == EXPR_NOP) && (*p != NUL) && (*p != ')') ) {
    if ( *p == '(' ) {
      p = expr_parse(plist, p+1, EXPR_NOP);
      left = NULL;
    }
    else {
      int i;

      for (i = 0; expr_operators[i] != NULL; i++) {
        if ( strncmp(p, expr_operators[i], strlen(expr_operators[i])) == 0 )
          op1 = i;
      }

      if ( op1 == EXPR_NOP )
        p++;
    }
  }

  if ( *p == ')' )
    *(p++) = NUL;

  if ( op1 != EXPR_NOP ) {
    *p = NUL;
    p += strlen(expr_operators[op1]);
  }

  if ( left != NULL ) {
    left = strskip_spaces(left);
    *strskip_chars(left) = NUL;
    if ( *left != NUL )
      *plist = expr_add(*plist, EXPR_NOP, left);
  }

  if ( op0 != EXPR_NOP ) {
    if ( (op1 == EXPR_NOP) || (expr_priorities[op0] >= expr_priorities[op1]) ) {
      *plist = expr_add(*plist, op0, NULL);
      op0 = EXPR_NOP;
    }
  }

  if ( *p != NUL )
    p = expr_parse(plist, p, op1);

  if ( op0 != EXPR_NOP )
    *plist = expr_add(*plist, op0, NULL);

  return p;
}


void expr_compile(GList **plist, char *expression)
{
  char str[strlen(expression)+1];

  /* Parse expression */
  strcpy(str, expression);
  expr_parse(plist, str, EXPR_NOP);

  //g_list_foreach(*plist, (GFunc) expr_item_show, NULL);
}


/*================================================*/
/* Expression evaluator                           */
/*================================================*/

typedef struct {
  expr_eval_hdl_t *hdl;
  int *stack;
  int ptr;
  expr_item_t *err;
} expr_eval_t;

static void expr_eval_item(expr_item_t *item, expr_eval_t *eval)
{
  if ( eval->err != NULL )
    return;

  if ( item->value != NULL ) {
    eval->stack[eval->ptr++] = eval->hdl(item);
  }

  if ( item->op != EXPR_NOP ) {
    if ( eval->ptr < 2 ) {
      eval->err = item;
    }
    else {
      int v1 = eval->stack[--eval->ptr];
      int v2 = eval->stack[--eval->ptr];

      switch ( item->op ) {
      case EXPR_OR:  eval->stack[eval->ptr++] = v1 | v2; break;
      case EXPR_AND: eval->stack[eval->ptr++] = v1 & v2; break;
      }
    }
  }
}

int expr_eval(GList *list, expr_eval_hdl_t *hdl)
{
  int result = 0;
  expr_eval_t eval;

  eval.hdl = hdl;
  eval.stack = (int *) malloc(sizeof(int) * g_list_length(list));
  eval.ptr = 0;
  eval.err = NULL;

  g_list_foreach(list, (GFunc) expr_eval_item, &eval);

  if ( eval.err != NULL ) {
    fprintf(stderr, NAME ": Syntax error in WAIT expression near '%s'\n", eval.err->value);
  }
  else if ( eval.ptr != 1 ) {
    fprintf(stderr, NAME ": Syntax error in WAIT expression\n");
  }
  else {
    result = eval.stack[--eval.ptr];
  }

  free(eval.stack);

  return result;
}
