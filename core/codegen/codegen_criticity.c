/****************************************************************************/
/* TestFarm                                                                 */
/* Test Suite code generator : test case criticity identifiers              */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 07-DEC-2001                                                    */
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
#include <libxml/parser.h>
#include <libxml/tree.h>

#include "install.h"
#include "useful.h"
#include "codegen_criticity.h"


typedef struct {
  char *id;
  char *id_uc;
  char *color;
} criticity_item_t;

static int criticity_nb = 0;
static criticity_item_t *criticity_tab = NULL;


int criticity_init(void)
{
  char *fname;
  xmlDocPtr doc;
  xmlNodePtr root;
  xmlNodePtr node;

  /* Clear old criticity level definitions */
  criticity_destroy();

  /* Get criticity level definition file name */
  fname = get_lib("criticity.xml");
  if ( fname == NULL ) {
    fprintf(stderr, "*WARNING* Unable to find criticity definition file 'criticity.xml'\n");
    return -1;
  }
  fprintf(stderr, "Criticity Levels definition file: %s\n", fname);

  /* Parse criticity level definitions */
  doc = xmlParseFile(fname);
  if ( doc != NULL ) {
    root = xmlDocGetRootElement(doc);
    if ( root != NULL ) {
      node = root->children;
      while ( node != NULL ) {
	if ( node->type == XML_ELEMENT_NODE ) {
	  if ( strcmp((char *) node->name, "CRITICITY") == 0 ) {
	    xmlChar *id = xmlGetProp(node, (xmlChar *) "id");
	    xmlChar *color = xmlGetProp(node, (xmlChar *) "color");
	    xmlChar *level = xmlGetProp(node, (xmlChar *) "level");
	    int i;

	    i = atoi((char *) level);
	    if ( i >= criticity_nb ) {
	      int nb = i + 5;
	      criticity_tab = (criticity_item_t *) realloc(criticity_tab, nb * sizeof(criticity_item_t));
	      while ( criticity_nb < nb ) {
		criticity_tab[criticity_nb].id = NULL;
		criticity_tab[criticity_nb].id_uc = NULL;
		criticity_tab[criticity_nb].color = NULL;
		criticity_nb++;
	      }
	    }

	    criticity_tab[i].id = strdup((char *) id);
	    criticity_tab[i].id_uc = strupper(strdup((char *) id));
	    criticity_tab[i].color = strdup((char *) color);

	    xmlFree(level);
	    xmlFree(color);
	    xmlFree(id);
	  }
	}
	node = node->next;
      }
    }

    xmlFreeDoc(doc);
  }

  xmlCleanupParser();
  free(fname);

  /* Set CRITICITY_NONE entry */
  if ( criticity_nb > 0 ) {
    criticity_tab[0].id = strdup("");
    criticity_tab[0].id_uc = strdup("");
    criticity_tab[0].color = strdup("black");
  }

  return 0;
}


void criticity_destroy(void)
{
  int i;

  if ( criticity_tab != NULL ) {
    for (i = 0; i < criticity_nb; i++) {
      criticity_item_t *item = &(criticity_tab[i]);
      if ( item->id != NULL )
	free(item->id);
      if ( item->id_uc != NULL )
	free(item->id_uc);
      if ( item->color != NULL )
	free(item->color);
    }

    free(criticity_tab);
    criticity_tab = NULL;
  }

  criticity_nb = 0;
}


char *criticity_id(criticity_t level)
{
  if ( (level < CRITICITY_NONE) || (level >= criticity_nb) )
    return "?";

  return criticity_tab[level].id;
}


char *criticity_color(criticity_t level)
{
  if ( (level < CRITICITY_NONE) || (level >= criticity_nb) )
    return "black";

  return criticity_tab[level].color;
}


criticity_t criticity_level(char *id)
{
  int level = CRITICITY_NONE;

  while ( level < criticity_nb ) {
    char *id2 = criticity_tab[level].id_uc;
    if ( (id2 != NULL) && (strcmp(id, id2) == 0) )
      break;
    level++;
  }

  if ( level >= criticity_nb )
    level = CRITICITY_UNKNOWN;

  return level;
}
