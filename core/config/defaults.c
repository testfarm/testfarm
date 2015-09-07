/****************************************************************************/
/* TestFarm                                                                 */
/* Default settings storage                                                 */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 29-MAR-2005                                                    */
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
#include <unistd.h>
#include <libxml/parser.h>
#include <libxml/tree.h>

#include "install.h"
#include "defaults.h"


char *defaults_get_string(char *name)
{
  char *fname;
  xmlDocPtr doc;
  xmlNodePtr root;
  xmlNodePtr node;
  char *ret = NULL;

  fname = get_user_config("defaults");
  doc = NULL;
  if ( access(fname, R_OK) == 0 )
    doc = xmlParseFile(fname);

  if ( doc != NULL ) {
    root = xmlDocGetRootElement(doc);
    if ( root != NULL ) {
      node = root->children;
      while ( node != NULL ) {
	if ( node->type == XML_ELEMENT_NODE ) {
	  if ( strcmp((char *) node->name, name) == 0 ) {
	    xmlChar *content = xmlNodeGetContent(node);
	    ret = strdup((char *) content);
	    xmlFree(content);
	    break;
	  }
	}
	node = node->next;
      }
    }

    xmlFreeDoc(doc);
  }

  xmlCleanupParser();

  return ret;
}


char *defaults_set_string(char *name, char *content)
{
  char *fname;
  xmlDocPtr doc;
  xmlNodePtr root;
  xmlNodePtr node;

  fname = get_user_config("defaults");
  doc = NULL;
  if ( access(fname, R_OK) == 0 )
    doc = xmlParseFile(fname);
  if ( doc == NULL )
    doc = xmlNewDoc((unsigned char *) XML_DEFAULT_VERSION);

  root = xmlDocGetRootElement(doc);
  if ( root == NULL ) {
    root = xmlNewNode(NULL, (unsigned char *) "TESTFARM_DEFAULTS");
    xmlDocSetRootElement(doc, root);
  }

  node = root->children;
  while ( node != NULL ) {
    if ( node->type == XML_ELEMENT_NODE ) {
      if ( strcmp((char *) node->name, name) == 0 ) {
	if ( content != NULL ) {
	  xmlNodeSetContent(node, (unsigned char *) content);
	}
	else {
	  xmlUnlinkNode(node);
	  xmlFreeNode(node);
	  node = NULL;
	}
	break;
      }
    }
    node = node->next;
  }

  if ( content != NULL ) {
    if ( node == NULL )
      node = xmlNewTextChild(root, NULL, (unsigned char *) name, (unsigned char *) content);
  }

  if ( xmlSaveFile(fname, doc) < 0 )
    fname = NULL;

  xmlFreeDoc(doc);

  return fname;
}
