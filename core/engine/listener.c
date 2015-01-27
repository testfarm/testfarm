/****************************************************************************/
/* TestFarm                                                                 */
/* Test Engine - Incoming events listener                                   */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 11-SEP-2003 (split from trig.c)                                */
/****************************************************************************/

/* $Revision: 174 $ */
/* $Date: 2006-07-26 17:43:56 +0200 (mer., 26 juil. 2006) $ */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <glib.h>

#include "useful.h"
#include "debug.h"
#include "trig.h"
#include "expr.h"
#include "listener.h"


/*=======================================================*/
/* Listener Item                                         */
/*=======================================================*/

typedef struct {
  trig_t *trig;
  int count;
} listener_item_t;

static listener_item_t *listener_item_new(trig_t *trig, int count)
{
  listener_item_t *item = (listener_item_t *) malloc(sizeof(listener_item_t));
  item->trig = trig;
  item->count = count;
  return item;
}

#define listener_item_destroy free

static int listener_item_state(listener_item_t *item)
{
  int state = (item->trig->state >= abs(item->count)) ? 1:0;
  if ( item->count < 0 )
    state = 1 - state;
  return state;
}

static void listener_item_raised(listener_item_t *item, char **p)
{
  if ( item->trig->state >= abs(item->count) )
    *p += sprintf(*p, "%s ", item->trig->id);
}


/*=======================================================*/
/* Listener operations                                   */
/*=======================================================*/

static char *listener_pack_expression(int argc, char **argv)
{
  int len = 0;
  char *str;
  char *p;
  int i;

  for (i = 0; i < argc; i++)
    len += strlen(argv[i]) + 3;

  p = str = (char *) malloc(len+1);

  for (i = 0; i < argc; i++) {
    char *s = argv[i];

    /* Agregate argv's with OR operator */
    if ( i > 0 )
      *(p++) = '|';

    /* Enclose argv's within parens */
    if ( argc > 1 )
      *(p++) = '(';

    while ( *s != NUL ) {
      if ( *s > ' ' ) {
        if ( *s == ',' )
          *(p++) = '&';
        else
          *(p++) = *s;
      }
      s++;
    }

    if ( argc > 1 )
      *(p++) = ')';
  }

  *p = NUL;

  return str;
}


static void listener_new_data(expr_item_t *item, int *errcount)
{
  char *s = item->value;
  int count = 1;
  char *star;
  trig_t *trig;

  /* Set expression item data only for operands */
  if ( s == NULL )
    return;

  if ( (star = strchr(s, '*')) != NULL ) {
    char *err;

    *star = NUL;
    count = strtol(star+1, &err, 0);
    if ( *err != NUL ) {
      fprintf(stderr, NAME ": Syntax error in occurence counter value for trigger %s\n", s);
      (*errcount)++;
    }
    else if ( count < 1 ) {
      fprintf(stderr, NAME ": Illegal occurence counter value for trigger %s\n", s);
      (*errcount)++;
    }
  }

  if ( *s == '!' ) {
    count = -count;
    s++;
  }

  if ( (trig = trig_retrieve(s)) != NULL ) {
    item->data = listener_item_new(trig, count);
  }
  else {
    fprintf(stderr, "Trigger %s not defined\n", s);
    (*errcount)++;
  }

  if ( star != NULL )
    *star = '*';
}


listener_t *listener_new(int argc, char **argv)
{
  listener_t *listener = (listener_t *) malloc(sizeof(listener_t));
  int errcount = 0;

  /* Get expression string */
  listener->str = listener_pack_expression(argc, argv);

  /* Compile expression */
  listener->list = NULL;
  expr_compile(&(listener->list), listener->str);

  /* Resolve expression values */
  g_list_foreach(listener->list, (GFunc) listener_new_data, &errcount);

  /* Check for errors */
  if ( errcount > 0 ) {
    listener_destroy(listener);
    return NULL;
  }

  return listener;
}

static void listener_destroy_data(expr_item_t *item)
{
  if ( item->data != NULL )
    listener_item_destroy((listener_item_t *) item->data);
  item->data = NULL;
}

void listener_destroy(listener_t *listener)
{
  if ( listener == NULL )
    return;
  g_list_foreach(listener->list, (GFunc) listener_destroy_data, NULL);
  expr_free(listener->list);
  free(listener->str);
  free(listener);
}

char *listener_str(listener_t *listener)
{
  return strdup(listener->str);
}

static void listener_raised_data(expr_item_t *item, char **p)
{
  if ( item->data != NULL )
    listener_item_raised((listener_item_t *) item->data, p);
}

char *listener_raised(listener_t *listener)
{
  int len = strlen(listener->str) + 2;
  char *str;
  char *p;

  p = str = (char *) malloc(len+1);
  g_list_foreach(listener->list, (GFunc) listener_raised_data, &p);
  *p = NUL;

  return str;
}


static int listener_eval_data(expr_item_t *item)
{
  if ( item->data == NULL )
    return 0;
  return listener_item_state((listener_item_t *) item->data);
}


int listener_check(listener_t *listener)
{
  return expr_eval(listener->list, listener_eval_data);
}
