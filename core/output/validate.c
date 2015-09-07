/****************************************************************************/
/* TestFarm                                                                 */
/* File validation check                                                    */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 29-JAN-2004                                                    */
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
#include <time.h>
#include <libxml/parser.h>
#include <libxml/tree.h>

#include "install.h"
#include "useful.h"
#include "codegen.h"
#include "md5.h"
#include "validate.h"


typedef struct {
  char *id;
  char *icon;
} validate_item_t;

int validate_max = -1;

static int validate_nb = 0;
static validate_item_t *validate_tab = NULL;


int validate_init(void)
{
  char *fname;
  xmlDocPtr doc;
  xmlNodePtr root;
  xmlNodePtr node;

  /* Clear old validation state definitions */
  validate_destroy();

  /* Get validation state definition file name */
  fname = get_lib("validate.xml");
  if ( fname == NULL ) {
    fprintf(stderr, "*WARNING* Unable to find validation state definition file 'validate.xml'\n");
    return -1;
  }
  fprintf(stderr, "Validation States definition file: %s\n", fname);

  /* Parse validation state definitions */
  doc = xmlParseFile(fname);
  if ( doc != NULL ) {
    root = xmlDocGetRootElement(doc);
    if ( root != NULL ) {
      node = root->children;
      while ( node != NULL ) {
	if ( node->type == XML_ELEMENT_NODE ) {
	  if ( strcmp((char *) node->name, "VALIDATE") == 0 ) {
	    xmlChar *level = xmlGetProp(node, (xmlChar *) "level");
	    xmlChar *id = xmlGetProp(node, (xmlChar *) "id");
	    xmlChar *icon = xmlGetProp(node, (xmlChar *) "icon");
	    int i;

	    i = atoi((char *) level);
	    if ( i >= validate_nb ) {
	      int nb = i + 5;
	      validate_tab = (validate_item_t *) realloc(validate_tab, nb * sizeof(validate_item_t));
	      while ( validate_nb < nb ) {
		validate_tab[validate_nb].id = NULL;
		validate_tab[validate_nb].icon = NULL;
		validate_nb++;
	      }
	    }

	    validate_tab[i].id = strdup((char *) id);
	    validate_tab[i].icon = strdup((char *) icon);
	    if ( i > validate_max )
	      validate_max = i;

	    xmlFree(level);
	    xmlFree(id);
	    xmlFree(icon);
	  }
	}
	node = node->next;
      }
    }

    xmlFreeDoc(doc);
  }

  xmlCleanupParser();
  free(fname);

  return 0;
}


void validate_destroy(void)
{
  int i;

  if ( validate_tab != NULL ) {
    for (i = 0; i < validate_nb; i++) {
      validate_item_t *item = &(validate_tab[i]);
      if ( item->id != NULL )
	free(item->id);
      if ( item->icon != NULL )
	free(item->icon);
    }

    free(validate_tab);
    validate_tab = NULL;
  }

  validate_nb = 0;
  validate_max = -1;
}


char *validate_get_id(int level)
{
  if ( level > validate_max )
    level = 0;
  return validate_tab[level].id;
}


char *validate_get_icon(int level)
{
  if ( level > validate_max )
    level = 0;
  return validate_tab[level].icon;
}


static void validate_clear(validate_t *v)
{
  if ( v->md5sum != NULL )
    free(v->md5sum);
  v->md5sum = NULL;

  v->level = 0;

  if ( v->date != NULL )
    free(v->date);
  v->date = NULL;

  if ( v->operator != NULL )
    free(v->operator);
  v->operator = NULL;

  v->updated = 0;
}


static char *validate_set_date(validate_t *v, char *date)
{
  char *p;

  if ( v->date != NULL )
    free(v->date);
  v->date = strdup(date);

  p = strchr(v->date, ',');
  if ( p != NULL )
    *p = ' ';

  return v->date;
}


static void validate_set(validate_t *v, char *md5sum, char *info)
{
  char *p1, *p2;

  /* Set MD5 sum */
  if ( v->md5sum != NULL )
    free(v->md5sum);
  v->md5sum = strdup(md5sum);

  /* Set validation level */
  p1 = strskip_spaces(info);
  p2 = strskip_chars(p1);
  if ( *p2 != NUL )
    *(p2++) = NUL;
  v->level = atoi(p1);

  /* Set date */
  p1 = strskip_spaces(p2);
  p2 = strskip_chars(p1);
  if ( *p2 != NUL )
    *(p2++) = NUL;
  validate_set_date(v, p1);

  /* Set validator name */
  p1 = strskip_spaces(p2);
  *strskip_chars(p1) = NUL;
  if ( v->operator != NULL )
    free(v->operator);
  v->operator = strdup(p1);
}


static int md5check(char *script, unsigned char *check)
{
  unsigned char signature[MD5_SIGNATURE_LENGTH];

  if ( md5_sign(script, signature) )
    return 0;
  return (strcmp((char *) signature, (char *) check) == 0) ? 1:0;
}


static char *filepath(tree_object_t *object, char **pbname)
{
  char *fname = NULL;

  if ( (object != NULL) && (object->type == TYPE_CASE) ) {
    char *script = object->d.Case->script->name;
    int len = strlen(script);
    char *p;

    /* Retrieve .check file name and script basename */
    fname = (char *) malloc(len+8);
    strcpy(fname, script);
    p = strrchr(fname, '/');
    if ( p == NULL )
      p = fname;
    else
      p++;

    if ( pbname != NULL ) {
      *pbname = (char *) malloc(len+1);
      strcpy(*pbname, p);
    }

    strcpy(p, ".check");
  }

  return fname;
}


int validate_check(validate_t *v, tree_object_t *object)
{
  char *bname = NULL;
  int validated = 0;
  char *fname;
  FILE *f;

  /* Retrieve file names */
  fname = filepath(object, &bname);
  if ( fname == NULL )
    return 0;

  /* Open check-list file */
  f = fopen(fname, "r");
  if ( f != NULL ) {
    while ( ! feof(f) ) {
      char buf[BUFSIZ];
      if ( fgets(buf, sizeof(buf), f) != NULL ) {
        char *md5sum = strskip_spaces(buf);
        char *s1, *s2;

        s1 = strskip_chars(md5sum);
        if ( *s1 != NUL )
          *(s1++) = NUL;
        s1 = strskip_spaces(s1);

        s2 = strskip_chars(s1);
        if ( *s2 != NUL )
          *(s2++) = NUL;
        s2 = strskip_spaces(s2);

        if ( strcmp(s1, bname) == 0 ) {
          validated = md5check(object->d.Case->script->name, (unsigned char *) buf);

          if ( v != NULL )
            validate_set(v, md5sum, s2);

          break;
        }
      }
    }

    fclose(f);
  }

  if ( bname != NULL )
    free(bname);
  free(fname);

  //if ( validated ) fprintf(stderr, "-- %s is validated\n", object->parent_item->name);
  return validated;
}


int validate_update(validate_t *v, tree_object_t *object, int level, char *operator)
{
  char *bname = NULL;
  char *bak_fname;
  FILE *bak_f = NULL;
  char *fname;
  FILE *f;
  int ret = 0;

  /* Retrieve check-list and script file names */
  fname = filepath(object, &bname);
  if ( fname == NULL )
    return -1;

  /* Construct backup check-list name */
  bak_fname = malloc(strlen(fname)+5);
  sprintf(bak_fname, "%s.bak", fname);

  if ( rename(fname, bak_fname) == 0 ) {
    bak_f = fopen(bak_fname, "r");
  }
  else {
    free(bak_fname);
    bak_fname = NULL;
  }

  /* Open new check-list */
  f = fopen(fname, "w");
  if ( f == NULL ) {
    ret = -1;
  }
  else {
    /* Copy the check-list entries for the other scripts */
    if ( bak_f != NULL ) {
      while ( ! feof(bak_f) ) {
        char buf[BUFSIZ];
        if ( fgets(buf, sizeof(buf), bak_f) != NULL ) {
          char *s1 = strskip_spaces(strskip_chars(strskip_spaces(buf)));
          char *s2 = strskip_chars(s1);
          char c = *s2;

          *s2 = NUL;

          if ( strcmp(s1, bname) ) {
            *s2 = c;
            if ( fputs(buf, f) < 0 )
              break;
          }
        }
      }

      fclose(bak_f);
    }

    /* Update validation state descriptor */
    if ( v != NULL ) {
      validate_clear(v);
      v->updated = 1;
    }

    /* Add check-list entry and Update validation state descriptor */
    if ( level > 0 ) {
      unsigned char signature[MD5_SIGNATURE_LENGTH];

      if ( md5_sign(object->d.Case->script->name, signature) == 0 ) {
        time_t t = time(NULL);
        char date[32];

        strftime(date, sizeof(date), "%d-%b-%Y,%H:%M:%S", localtime(&t));

        if ( v != NULL ) {
          v->md5sum = strdup((char *) signature);
          v->level = level;
          validate_set_date(v, date);
          v->operator = strdup(operator);
        }

        fprintf(f, "%s %s %d %s %s\n", signature, bname, level, date, operator);
      }
    }

    /* Open new check-list file */
    fclose(f);
  }

  /* Remove backup check-list if any */
  if ( bak_fname != NULL ) {
    remove(bak_fname);
    free(bak_fname);
  }

  if ( bname != NULL )
    free(bname);
  free(fname);

  return ret;
}


validate_t *validate_alloc(void)
{
  validate_t *v = (validate_t *) malloc(sizeof(validate_t));

  v->md5sum = NULL;
  v->level = 0;
  v->date = NULL;
  v->operator = NULL;
  v->updated = 0;

  return v;
}


void validate_free(validate_t *v)
{
  if ( v == NULL )
    return;

  validate_clear(v);
  free(v);
}


validate_t *validate_object_data(tree_object_t *object)
{
  validate_t *v = tree_object_get_data(object, "validate");

  if ( v == NULL ) {
    v = validate_alloc();
    tree_object_set_data(object, "validate", v, (void *) validate_free);
  }

  return v;
}
