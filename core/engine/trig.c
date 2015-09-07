/****************************************************************************/
/* TestFarm                                                                 */
/* Test Engine - Incoming events triggering                                 */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 11-AUG-2000                                                    */
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
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <glib.h>

#include "useful.h"
#include "shell_argv.h"
#include "result.h"
#include "trig.h"


/*=======================================================*/
/* Trigger retrieval                                     */
/*=======================================================*/

static GList *trigs = NULL;


static int trig_retrieve_compare(trig_t *trig, char *id)
{
  return strcmp(trig->id, id);
}


trig_t *trig_retrieve(char *id)
{
  GList *l = g_list_find_custom(trigs, id, (GCompareFunc) trig_retrieve_compare);
  return (l != NULL) ? l->data : NULL;
}


/*=======================================================*/
/* Trigger display                                       */
/*=======================================================*/

static void trig_show(trig_t *trig, char *id)
{
  int size;
  char *s, *p;

  if ( (id != NULL) && (strcmp(trig->id, id) != 0) )
    return;

  size = strlen(trig->id) + strlen(trig->regex) + 20;
  if ( trig->periph != NULL )
    size += strlen(trig->periph->id) + 8;
  if ( trig->info != NULL )
    size += strlen(trig->info) + 10;
  s = (char *) malloc(size);

  p = s;
  p += sprintf(p, "%s %d ", trig->id, trig->state);
  if ( trig->periph != NULL )
    p += sprintf(p, "periph=%s ", trig->periph->id);
  p += sprintf(p, "regex='%s'", trig->regex);
  if ( trig->info != NULL )
    p += sprintf(p, " info='%s'", trig->info);

  result_dump_engine(TAG_TRIG, s);

  free(s);
}


/*=======================================================*/
/* Trigger definition                                    */
/*=======================================================*/

static void trig_undef_item(trig_t *trig)
{
  regfree(&(trig->pattern));
  free(trig->regex);
  if ( trig->info != NULL )
    free(trig->info);
  free(trig->id);
  free(trig);
}

int trig_undef(char **argv, int argc)
{
  /* No arguments: remove all entries */
  if ( argc == 0 ) {
    g_list_foreach(trigs, (GFunc) trig_undef_item, NULL);
    g_list_free(trigs);
    trigs = NULL;
  }

  /* Arguments: remove listed entries */
  else {
    int i;

    for (i = 0; i < argc; i++) {
      trig_t *trig = trig_retrieve(strupper(argv[i]));

      if ( trig != NULL ) {
        trigs = g_list_remove(trigs, trig);
        trig_undef_item(trig);
      }
    }
  }

  return 0;
}


int trig_def(shell_t *shell, char *id, periph_item_t *periph, char **argv, int argc)
{
  trig_t *trig;
  int reglen;
  int i;
  int errcode;

  if ( id != NULL )
    strupper(id);

  if ( argc <= 0 ) {
    g_list_foreach(trigs, (GFunc) trig_show, id);
    return 0;
  }

  /* Alloc trigger entry */
  trig = trig_retrieve(id);
  if ( trig == NULL ) {
    trig = (trig_t *) malloc(sizeof(trig_t));
    trig->id = strdup(id);
  }
  else {
    trigs = g_list_remove(trigs, trig);
    regfree(&(trig->pattern));
    free(trig->regex);
  }

  /* Clear trigger info buffer */
  trig->info = NULL;

  /* Clear trigger counter */
  trig->state = 0;

  /* Setup peripheral source */
  trig->periph = periph;

  /* Setup trigger regex */
  reglen = 0;
  for (i = 0; i < argc; i++)
    reglen += (strlen(argv[i]) + 1);

  trig->regex = (char *) malloc(reglen);

  reglen = 0;
  for (i = 0; i < argc; i++) {
    int len = strlen(argv[i]);
    strcpy(trig->regex + reglen, argv[i]);
    reglen += len;

    trig->regex[reglen++] = ' ';
  }
  trig->regex[--reglen] = '\0';

  /* Alloc pattern */
  errcode = regcomp(&(trig->pattern), trig->regex, REG_EXTENDED | REG_NOSUB);
  if ( errcode != 0 ) {
    char errbuf[256];

    regerror(errcode, &(trig->pattern), errbuf, sizeof(errbuf));
    shell_error(shell, "regex error for trigger %s: %s\n", trig->id, errbuf);

    trig_undef_item(trig);
    return -1;
  }

  /* Add new trigger to list */
  trigs = g_list_append(trigs, trig);

  /* Show new trigger */
  trig_show(trig, NULL);

  return 0;
}


/*=======================================================*/
/* Trigger clear                                         */
/*=======================================================*/

static void trig_clear_item(trig_t *trig)
{
  if ( trig->info != NULL ) {
    free(trig->info);
    trig->info = NULL;
  }

  trig->state = 0;
}


int trig_clear(char **argv, int argc)
{
  /* No arguments: clear all entries */
  if ( argc == 0 ) {
    g_list_foreach(trigs, (GFunc) trig_clear_item, NULL);
  }

  /* Arguments: clear listed entries */
  else {
    int i;

    for (i = 0; i < argc; i++) {
      trig_t *trig = trig_retrieve(strupper(argv[i]));

      if ( trig != NULL )
        trig_clear_item(trig);
    }
  }

  return 0;
}


/*=======================================================*/
/* Trigger count                                         */
/*=======================================================*/

int trig_count(char *id)
{
  trig_t *trig = trig_retrieve(id);

  if ( trig == NULL )
    return -1;

  return trig->state;
}


/*=======================================================*/
/* Trigger info fetch                                    */
/*=======================================================*/

int trig_info(char *id, char **info)
{
  trig_t *trig = trig_retrieve(id);

  if ( trig == NULL )
    return -1;

  if ( info != NULL )
    *info = trig->info;

  return 0;
}


/*=======================================================*/
/* Trigger update with regex pattern matching            */
/*=======================================================*/

typedef struct {
  periph_item_t *periph;
  char *buf;
  int raised;
} trig_update_t;


static void trig_update_item_info(trig_t *trig, char *info)
{
  char *s;

  /* Free previous info buffer */
  if ( trig->info != NULL )
    free(trig->info);

  /* Allocate and feed new info buffer */
  trig->info = strdup(info);

  /* Cleanup info buffer to remove non-pritable characters */
  s = trig->info;
  while ( *s != NUL ) {
    if ( *s < ' ' )
      *s = ' ';
    s++;
  }

  s--;
  while ( (s >= trig->info) && (*s == ' ') )
    *(s--) = NUL;
}

static void trig_update_item(trig_t *trig, trig_update_t *p)
{
  if ( (trig->periph == NULL) || (trig->periph == p->periph) ) {
    int match;

    match = (regexec(&(trig->pattern), p->buf, 0, NULL, 0) == 0);

    if ( match ) {
      /* Here we are. The incoming message matches with
         the wait pattern: increment trigger counter */
      (trig->state)++;
      p->raised = 1;

      /* Also feed the info buffer */
      trig_update_item_info(trig, p->buf);
    }
  }
}


int trig_update(periph_item_t *periph, char *buf)
{
  trig_update_t d = {periph, buf, 0};

  g_list_foreach(trigs, (GFunc) trig_update_item, &d);

  return d.raised;
}
