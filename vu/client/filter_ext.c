/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* External Filter Agent Management                                         */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 28-DEC-2007                                                    */
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

#include "options.h"
#include "agent.h"
#include "filter.h"
#include "filter_ext.h"


/* EXTERNAL AGENT COMMANDS:
     Commands are received from the standard input, formatted as follows:
       <op><num> [<options>...]
     The minimal command set is:
       Create filter  : +<num> <class_id> <options> ...
       Destroy filter : -<num>
       Show filter    : ?<num>
       Apply filter   : !<num> <shmid>

     If a command references an invalid <shmid>.<num> tuple,
     it should be silently ignored.

   EXTERNAL AGENT RESPONSES:
     Responses are sent to the standard output.
     When started, the agent program registers the implemented filter class(es)
       *<class_id>
     All commands should send a response back when completed:
       <op><num> [<message>]

     Messages otherwise formatted are displayed as a warning message.
*/

#define dprintf(args...) //fprintf(stderr, "[FILTER_EXT_DEBUG] " args);

static filter_t *filter_ext_alloc(char *class_id, GList *options);
static void filter_ext_destroy(filter_t *filter_);
static void filter_ext_show(filter_t *filter_, char *header, char *trailer);
static void filter_ext_apply(filter_t *filter_, frame_hdr_t *frame);


/*======================================================*/
/* Filter agents                                        */
/*======================================================*/

typedef struct {
  char *path;
  char *basename;
  agent_t *agent;
  unsigned long cnt;
  GList *filters;
} filter_ext_agent_t;

static GList *filter_ext_agents = NULL;

static void filter_ext_event(filter_ext_agent_t *xagent, char *buf, int size);


static int filter_ext_enable_agent(filter_ext_agent_t *xagent)
{
	if (xagent->agent == NULL) {
		char *argv[2] = { xagent->path, NULL };
		dprintf("ENABLE AGENT %s\n", xagent->path);
		xagent->agent = agent_create(argv, (agent_callback_t *) filter_ext_event, xagent);
	}

	return (xagent->agent != NULL) ? 0 : -1;
}


static void filter_ext_add_agent(const char *dirname, const char *basename)
{
  filter_ext_agent_t *xagent;
  int size, len;
  GList *l;

  /* Ignore duplicate agent basename */
  l = filter_ext_agents;
  while ( l ) {
    filter_ext_agent_t *xagent2 = l->data;

    if ( strcmp(xagent2->basename, basename) == 0 )
      return;

    l = g_list_next(l);
  }

  xagent = (filter_ext_agent_t *) malloc(sizeof(filter_ext_agent_t));
  memset(xagent, 0, sizeof(filter_ext_agent_t));

  size = strlen(dirname) + strlen(basename) + 4;
  xagent->path = (char *) malloc(size);
  len = snprintf(xagent->path, size, "%s" G_DIR_SEPARATOR_S, dirname);

  xagent->basename = xagent->path + len;
  snprintf(xagent->basename, size-len, "%s", basename);

  filter_ext_agents = g_list_append(filter_ext_agents, xagent);

  dprintf("ADD AGENT basename='%s' path='%s'\n", xagent->basename, xagent->path);

  /* Start agent to collect the list of filter classes */
  filter_ext_enable_agent(xagent);
}


static void filter_ext_disable_agent(filter_ext_agent_t *xagent)
{
  /* Make agent a zombie. Keep it in the list. */
  if ( xagent->agent != NULL ) {
    agent_destroy(xagent->agent);
    xagent->agent = NULL;
    dprintf("DISABLE AGENT '%s'\n", xagent->path);
  }
}


/*======================================================*/
/* Filter classes                                       */
/*======================================================*/

typedef struct {
  filter_class_t class;
  filter_ext_agent_t *xagent;
} filter_ext_class_t;

static GList *filter_ext_classes = NULL;


static filter_ext_class_t *filter_ext_get_class(const char *class_id)
{
  GList *l;

  /* Class already registered ? */
  l = filter_ext_classes;
  while ( l != NULL ) {
    filter_ext_class_t *xclass = (filter_ext_class_t *) l->data;
    if ( strcmp(xclass->class.id, class_id) == 0 )
      return xclass;
    l = g_list_next(l);
  }

  return NULL;
}


static filter_ext_class_t *filter_ext_add_class(filter_ext_agent_t *xagent, char *class_id)
{
  filter_ext_class_t *xclass;

  /* Manage duplicate class registration:
     - Ignore if it comes from the same agent;
     - Reject if it comes from a different agent. */
  xclass = filter_ext_get_class(class_id);
  if ( xclass != NULL ) {
    if ( xclass->xagent != xagent ) {
      fprintf(stderr, "WARNING: Rejected filter class '%s' registered by agent '%s'\n", class_id, xagent->path);
      fprintf(stderr, "WARNING: This class was previously registered by agent '%s'\n", xclass->xagent->path);
      return NULL;
    }
    else {
      return xclass;
    }
  }

  /* Alloc new class */
  xclass = (filter_ext_class_t *) malloc(sizeof(filter_ext_class_t));
  xclass->class.id = strdup(class_id);
  xclass->class.alloc = filter_ext_alloc;
  xclass->class.destroy = filter_ext_destroy;
  xclass->class.show = filter_ext_show;
  xclass->class.apply = filter_ext_apply;
  xclass->xagent = xagent;

  /* Append new class to class list */
  filter_ext_classes = g_list_append(filter_ext_classes, xclass);
  filter_register(&(xclass->class));

  dprintf("ADD CLASS '%s' by agent '%s'\n", xclass->class.id, xagent->path);

  return xclass;
}


/*======================================================*/
/* Filter operations                                    */
/*======================================================*/

#define FILTER_EXT(_ptr_) ((filter_ext_t *)(_ptr_))

typedef struct {
  filter_t hdr;
  unsigned long num;
  filter_ext_class_t *xclass;
  char *show_header;
  char *show_trailer;
  gboolean apply_requested;
} filter_ext_t;


static filter_ext_t *filter_ext_retrieve(filter_ext_agent_t *xagent, unsigned long num)
{
  GList *l = xagent->filters;

  while ( l ) {
    filter_ext_t *filter = l->data;
    if ( filter->num == num )
      return filter;
    l = g_list_next(l);
  }

  return NULL;
}


static filter_t *filter_ext_alloc(char *class_id, GList *options)
{
  filter_ext_class_t *xclass;
  filter_ext_agent_t *xagent;
  unsigned long num0, num;
  int size, len;
  char *buf;
  filter_ext_t *filter;

  /* Retrieve filter class descriptor */
  xclass = filter_ext_get_class(class_id);
  if ( xclass == NULL ) {
    fprintf(stderr, "WARNING: Referencing unknown filter class '%s'\n", class_id);
    return NULL;
  }
  xagent = xclass->xagent;

  /* Choose an unsed filter num for the agent */
  num0 = xagent->cnt;
  do {
    num = ++(xagent->cnt);
  } while ( (num != num0) && (filter_ext_retrieve(xagent, num) != NULL) );

  if ( num == num0 ) {
    fprintf(stderr, "WARNING: Too many filters allocated for agent '%s'\n", xagent->path);
    return NULL;
  }

  dprintf("ALLOC FILTER class='%s' num=%lu\n", xclass->class.id, num);

  /* Launch filter agent (if not already done) */
  if (filter_ext_enable_agent(xagent)) {
	  fprintf(stderr, "WARNING: Failed to start agent '%s'\n", xagent->path);
	  return NULL;
  }

  /* Construct filter agent creation request */
  size = strlen(xclass->class.id) + 80;
  buf = malloc(size);
  len = snprintf(buf, size, "+%lu %s", num, xclass->class.id);

  while ( options ) {
    char *str = options->data;
    unsigned int size2 = len + strlen(str) + 2;
    
    /* Grow request string buffer if needed */
    if ( size2 > size ) {
      size = size2 + 80;
      buf = realloc(buf, size);
    }

    len += snprintf(buf+len, size-len, " %s", str);

    options = g_list_next(options);
  }

  buf[len++] = '\n';
  buf[len] = '\0';

  /* Send filter agent creation request */
  agent_request(xagent->agent, buf, len);

  free(buf);

  /* Alloc local filter descriptor */
  filter = (filter_ext_t *) malloc(sizeof(filter_ext_t));
  memset(filter, 0, sizeof(filter_ext_t));

  filter->num = num;
  filter->xclass = xclass;

  /* Append new descriptor to agent filter list */
  xagent->filters = g_list_append(xagent->filters, filter);

  return FILTER(filter);
}


static void filter_ext_show_setup(filter_ext_t *filter, char *header, char *trailer)
{
  if ( filter->show_header != NULL ) {
    free(filter->show_header);
    filter->show_header = NULL;
  }

  if ( filter->show_trailer != NULL ) {
    free(filter->show_trailer);
    filter->show_trailer = NULL;
  }

  if ( header != NULL ) {
    filter->show_header = strdup(header);
  }

  if ( trailer != NULL ) {
    filter->show_trailer = strdup(trailer);
  }
}


static void filter_ext_destroy(filter_t *filter_)
{
  filter_ext_t *filter = FILTER_EXT(filter_);
  filter_ext_agent_t *xagent = filter->xclass->xagent;
  char buf[16];
  int len;

  dprintf("DESTROY FILTER num=%lu\n", filter->num);

  /* Send request to filter agent */
  len = snprintf(buf, sizeof(buf), "-%lu\n", filter->num);
  agent_request(xagent->agent, buf, len);

  /* Remove descriptor from agent filter list */
  xagent->filters = g_list_remove(xagent->filters, filter);

  /* Kill agent if not used by any filter */
  if (xagent->filters == NULL) {
	  filter_ext_disable_agent(xagent);
  }

  /* Free show header/trailer strings (if any) */
  filter_ext_show_setup(filter, NULL, NULL);

  /* Free filter descriptor */
  free(filter);
}


static void filter_ext_show(filter_t *filter_, char *header, char *trailer)
{
  filter_ext_t *filter = FILTER_EXT(filter_);
  filter_ext_agent_t *xagent = filter->xclass->xagent;
  char buf[16];
  int len;

  dprintf("SHOW FILTER num=%lu header='%s' trailer='%s'\n", filter->num, header, trailer);

  /* Set message header and trailer */
  filter_ext_show_setup(filter, header, trailer);

  /* Send request to filter agent */
  len = snprintf(buf, sizeof(buf), "?%lu\n", filter->num);
  agent_request(xagent->agent, buf, len);
}


static void filter_ext_apply(filter_t *filter_, frame_hdr_t *frame)
{
  filter_ext_t *filter = FILTER_EXT(filter_);
  filter_ext_agent_t *xagent = filter->xclass->xagent;
  char buf[32];
  int len;

  dprintf("APPLY FILTER num=%lu frame='%s'\n", filter->num, frame->id);

  /* Send request to filter agent */
  len = snprintf(buf, sizeof(buf), "!%lu %d\n", filter->num, frame->shmid);
  agent_request(xagent->agent, buf, len);

  filter->apply_requested = TRUE;
}


static void filter_ext_event(filter_ext_agent_t *xagent, char *buf, int size)
{
  if ( buf != NULL ) {
    char op = buf[0];

    g_strchomp(buf);

    switch ( op ) {
    case '!':  /* Filter applied */
    case '?':  /* Filter information */
      {
	char *str1 = buf + 1;
	char *str2 = strchr(str1, ' ');
	filter_ext_t *filter;

	if ( str2 != NULL ) {
	  *(str2++) = '\0';
	  g_strchug(str2);
	}

	filter = filter_ext_retrieve(xagent, strtoul(str1, NULL, 0));

	if ( filter != NULL ) {
	  switch ( op ) {
	  case '!':  /* Filter applied */
	    dprintf("DONE APPLY FILTER num=%lu\n", filter->num);
	    if ( filter->apply_requested ) {
	      filter->apply_requested = FALSE;
	      filter_applied(FILTER(filter));
	    }
	    break;

	  case '?':  /* Filter information */
	    dprintf("DONE SHOW FILTER num=%lu msg='%s'\n", filter->num, str2);
	    if ( filter->show_header != NULL )
	      fputs(filter->show_header, stdout);
	    if ( str2 != NULL )
	      fputs(str2, stdout);
	    if ( filter->show_trailer != NULL )
	      fputs(filter->show_trailer, stdout);
	    break;
	  }
	}
      }
      break;
    case '+':  /* Filter added */
    case '-':  /* Filter removed */
      /* Ignored */
      break;
    case '*':  /* Filter class registration */
      if (buf[1] != '\0') {
	      filter_ext_add_class(xagent, buf+1);
      }
      else {
	      /* Empty class identifier means the class registration list is
		 complete. So we can disable the filter subprocess if it is
		 not in use */
	      if (xagent->filters == NULL) {
		      filter_ext_disable_agent(xagent);
	      }
      }
      break;
    default:
      fprintf(stderr, "WARNING: %s: %s\n", xagent->path, buf);
      break;
    }
  }
  else {
    filter_ext_disable_agent(xagent);
  }
}


static void filter_ext_init_dir(char *dirname)
{
  GDir *dir;

  dprintf("SCAN DIRECTORY %s\n", dirname);

  dir = g_dir_open(dirname, 0, NULL);
  if ( dir != NULL ) {
    char *basename;

    while ( (basename = (char *) g_dir_read_name(dir)) != NULL ) {
      char filename[strlen(dirname)+strlen(basename)+4];

      snprintf(filename, sizeof(filename), "%s" G_DIR_SEPARATOR_S "%s", dirname, basename);

      if ( g_file_test(filename, G_FILE_TEST_IS_EXECUTABLE) ) {
	filter_ext_add_agent(dirname, basename);
      }
    }

    g_dir_close(dir);
  }
}


static void filter_ext_init_pgm(char *pgmname)
{
  char *dirname = g_path_get_dirname(pgmname);
  char *basename = g_path_get_basename(pgmname);

  filter_ext_add_agent(dirname, basename);

  free(basename);
  free(dirname);
}


void filter_ext_init(void)
{
  const gchar *home = g_get_home_dir();
  char dirname[strlen(home)+32];

  /* Firstly, add programs explicitely specified as command line arguments */
  g_list_foreach(opt_filters, (GFunc) filter_ext_init_pgm, NULL);

  /* Secondly, search in local user's lib dir ~/.testfarm-vu/filters */
  snprintf(dirname, sizeof(dirname), "%s" G_DIR_SEPARATOR_S ".testfarm-vu" G_DIR_SEPARATOR_S "filters", home);
  filter_ext_init_dir(dirname);

  /* Finally, search in global testfarm-vu lib dir */
  filter_ext_init_dir("/usr/lib/testfarm-vu/filters");
}


void filter_ext_done(void)
{
  g_list_foreach(filter_ext_agents, (GFunc) filter_ext_disable_agent, NULL);
}
