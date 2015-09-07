/****************************************************************************/
/* TestFarm                                                                 */
/* Test Report Configuration                                                */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 18-DEC-2001                                                    */
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
#include <errno.h>
#include <unistd.h>
#include <limits.h>
#include <glib.h>
#include <sys/stat.h>
#include <libxml/parser.h>
#include <libxml/tree.h>

#include "install.h"
#include "defaults.h"
#include "useful.h"
#include "report_config.h"


report_config_item_t report_config_table[] = {
  { "show_standard",             REPORT_CONFIG_STANDARD },
  { "show_in_title",             REPORT_CONFIG_IN_TITLE },
  { "show_in_header",            REPORT_CONFIG_IN_HEADER },
  { "show_in_verdict",           REPORT_CONFIG_IN_VERDICT },
  { "show_operator",             REPORT_CONFIG_OPERATOR },
  { "show_duration",             REPORT_CONFIG_DURATION },
  { "show_scenario",             REPORT_CONFIG_SCENARIO },
  { "show_case",                 REPORT_CONFIG_CASE },
  { "show_validation_parens",    REPORT_CONFIG_VALIDATED },
  { "show_validation_state",     REPORT_CONFIG_VALIDATION_STATE },
  { "show_criticity",            REPORT_CONFIG_CRITICITY },
  { "show_nonsignificant",       REPORT_CONFIG_NONSIGNIFICANT },
  { "show_log",                  REPORT_CONFIG_LOG },
  { "show_verdict_passed",       REPORT_CONFIG_VERDICT_PASSED },
  { "show_verdict_failed",       REPORT_CONFIG_VERDICT_FAILED },
  { "show_verdict_inconclusive", REPORT_CONFIG_VERDICT_INCONCLUSIVE },
  { "show_verdict_skip",         REPORT_CONFIG_VERDICT_SKIP },
  { "show_dump",                 REPORT_CONFIG_DUMP },
  { "show_dump_passed",          REPORT_CONFIG_DUMP_PASSED },
  { "show_dump_failed",          REPORT_CONFIG_DUMP_FAILED },
  { "show_dump_inconclusive",    REPORT_CONFIG_DUMP_INCONCLUSIVE },
  { "show_dump_skip",            REPORT_CONFIG_DUMP_SKIP },
  { NULL, 0 }
};


static int report_config_mkdir_level(char *subdir)
{
  char *dirname;
  GDir *dir;
  int ret = 0;

  dirname = get_user_config(subdir);
  dir = g_dir_open(dirname, 0, NULL);
  if ( dir != NULL ) {
    g_dir_close(dir);
  }
  else {
    if ( mkdir(dirname, 0777) ) {
      fprintf(stderr, "Cannot create directory %s: %s\n", dirname, strerror(errno));
      ret = -1;
    }
    else {
      fprintf(stderr, "Directory \"%s\" created\n", dirname);
    }
  }

  return ret;
}


static int report_config_mkdir(void)
{
  if ( report_config_mkdir_level("") )
    return -1;
  if ( report_config_mkdir_level(REPORT_CONFIG_SUBDIR) )
    return -1;

  return 0;
}


static char *report_config_default_name(void)
{
  char *name = defaults_get_string("report");

  if ( name != NULL ) {
    name = g_strstrip(name);

    if ( name[0] == '\0' ) {
      g_free(name);
      name = NULL;
    }
  }

  return name;
}


static char *report_config_fname(char *name)
{
  char *fname = NULL;

  if ( name != NULL )
    fname = get_user_config(REPORT_CONFIG_SUBDIR G_DIR_SEPARATOR_S "%s", name);

  return (fname != NULL) ? strdup(fname) : NULL;
}


static void report_config_set_mask(report_config_t *rc, char *id, int state)
{
  /* Retrieve configuration item */ 
  report_config_item_t *item = report_config_table;
  while ( (item->id != NULL) && (strcmp(item->id, id) != 0) )
    item++;

  /* Set config flag accordingly */
  if ( item->id != NULL ) {
    if ( state )
      rc->conf |= item->mask;
    else
      rc->conf &= ~(item->mask);
  }
}


void report_config_set_stylesheet(report_config_t *rc, char *stylesheet)
{
  if ( rc == NULL )
    return;

  if ( rc->stylesheet != NULL ) {
    free(rc->stylesheet);
    rc->stylesheet = NULL;
  }

  if ( stylesheet == NULL )
    return;

  stylesheet = strskip_spaces(stylesheet);
  if ( *stylesheet == NUL )
    return;

  rc->stylesheet = strdup(stylesheet);
}


static char *report_config_standard_stylesheet = NULL;

char *report_config_get_stylesheet(report_config_t *rc)
{
  char *stylesheet = NULL;

  if ( rc != NULL ) {
    if ( (rc->conf & REPORT_CONFIG_STANDARD) == 0 )
      if ( rc->stylesheet != NULL )
        if ( access(rc->stylesheet, R_OK) == 0 )
          stylesheet = rc->stylesheet;
  }

  if ( stylesheet == NULL ) {
    if ( report_config_standard_stylesheet == NULL ) {
      char *home = get_home();
      report_config_standard_stylesheet = (char *) malloc(strlen(home)+20);
      sprintf(report_config_standard_stylesheet, "%s/lib/output.xsl", home);
    }

    stylesheet = report_config_standard_stylesheet;
  }

  return stylesheet;
}


static void report_config_parse(report_config_t *rc, char *fname)
{
  xmlDocPtr doc;
  xmlNodePtr node;

  doc = xmlParseFile(fname);
  if ( doc == NULL )
    return;

  node = xmlDocGetRootElement(doc);
  if ( node != NULL ) {
    node = node->children;
    while ( node != NULL ) {
      if ( node->type == XML_ELEMENT_NODE ) {
	if ( (node->children != NULL) && (node->children->content != NULL) ) {
	  char *content = (char *) node->children->content;

	  if ( strcmp((char *) node->name, "stylesheet") == 0 ) {
	    report_config_set_stylesheet(rc, content);
	  }
	  else {
	    report_config_set_mask(rc, (char *) node->name, atoi(content));
	  }
	}
      }
      node = node->next;
    }
  }

  xmlFreeDoc(doc);
}


static void report_config_set_name(report_config_t *rc, char *name)
{
  /* Free previous settings */
  if ( rc->name != NULL )
    free(rc->name);
  rc->name = NULL;

  if ( rc->fname != NULL )
    free(rc->fname);
  rc->fname = NULL;

  /* Setup new names */
  if ( name != NULL ) {
    rc->name = strdup(name);
    rc->fname = report_config_fname(rc->name);
  }
}


char *report_config_load(report_config_t *rc, char *name)
{
  char *fname;

  /* Set new name if specified */
  if ( name != NULL ) {
    report_config_set_name(rc, name);
  }

  /* Clear report config settings */
  if ( rc->stylesheet != NULL )
    free(rc->stylesheet);
  rc->stylesheet = NULL;
  rc->conf = REPORT_CONFIG_DEFAULT;

  /* Get report configuration: */
  /* If config file is not accessible, assume the default layout */
  fname = rc->fname;
  if ( fname != NULL ) {
    if ( access(fname, R_OK) )
      fname = NULL;
    else
      report_config_parse(rc, fname);
  }

  return fname;
}


int report_config_save(report_config_t *rc, char *name)
{
  FILE *f;
  report_config_item_t *item;

  if ( rc == NULL )
    return -1;

  /* Create directory if not already done */
  if ( report_config_mkdir() )
    return -1;

  /* Set new name if specified */
  if ( name != NULL ) {
    report_config_set_name(rc, name);

    /* Update defaults */
    defaults_set_string("report", rc->name);
  }

  if ( rc->fname == NULL )
    return -1;

  if ( (f = fopen(rc->fname, "w")) == NULL ) {
    fprintf(stderr, "Cannot write Report Configuration file %s: %s\n", rc->fname, strerror(errno));
    return -1;
  }

  fprintf(f, "<REPORT_CONFIG>\n");

  if ( rc->stylesheet != NULL )
    fprintf(f, "  <stylesheet>%s</stylesheet>\n", rc->stylesheet);

  item = report_config_table;
  while ( item->id != NULL ) {
    fprintf(f, "  <%s>%d</%s>\n", item->id, (rc->conf & item->mask) ? 1:0, item->id);
    item++;
  }

  fprintf(f, "</REPORT_CONFIG>\n");
  fclose(f);

  return 0;
}


void report_config_clear(report_config_t *rc)
{
  report_config_set_name(rc, NULL);
  report_config_set_stylesheet(rc, NULL);
  rc->conf = REPORT_CONFIG_DEFAULT;

  defaults_set_string("report", NULL);
}


report_config_t *report_config_alloc(void)
{
  report_config_t *rc;

  /* Alloc report config descriptor */
  rc = (report_config_t *) malloc(sizeof(report_config_t));

  /* Set default Report Configuration name */
  rc->name = report_config_default_name();
  rc->fname = report_config_fname(rc->name);
  rc->stylesheet = NULL;
  rc->conf = REPORT_CONFIG_DEFAULT;

  return rc;
}


void report_config_destroy(report_config_t *rc)
{
  if ( rc == NULL )
    return;
  report_config_set_name(rc, NULL);
  if ( rc->stylesheet != NULL )
    free(rc->stylesheet);
  free(rc);
}
