/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* Pattern Matching commands                                                 */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 12-NOV-2007                                                    */
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

#include "log.h"

#include "error.h"
#include "frame.h"
#include "match.h"
#include "match_image.h"
#include "match_command.h"


static match_t *match = NULL;


/*******************************************************/
/* The 'match' shell command                           */
/*******************************************************/

typedef enum {
  PARAM_TYPE_WINDOW=0,
  PARAM_TYPE_FRAME,
  PARAM_TYPE_SOURCE,
  PARAM_TYPE_MODE,
} param_type_t;

typedef struct {
  char *id;
  param_type_t type;
  unsigned long mask;
} param_spec_t;


static param_spec_t match_command_params[] = {
  {"window",    PARAM_TYPE_WINDOW },
  {"frame",     PARAM_TYPE_FRAME },
  {"image",     PARAM_TYPE_SOURCE },
  {"text",      PARAM_TYPE_SOURCE },
  {"appear",    PARAM_TYPE_MODE, PATTERN_MODE_APPEAR },
  {"disappear", PARAM_TYPE_MODE, PATTERN_MODE_DISAPPEAR },
  {"retrigger", PARAM_TYPE_MODE, PATTERN_MODE_RETRIGGER },
  {"inverse",   PARAM_TYPE_MODE, PATTERN_MODE_INVERSE },
  {"brief",     PARAM_TYPE_MODE, PATTERN_MODE_BRIEF },
  {"dump",      PARAM_TYPE_MODE, PATTERN_MODE_DUMP },
  {NULL}
};


static char *match_command_parse_source(char *source, pattern_type_t *ptype)
{
  pattern_type_t type = PATTERN_TYPE_IMAGE;
  char *eq;

  /* Reject illegal source */
  if ( source == NULL )
    return NULL;

  /* Determine pattern type */
  eq = strchr(source, '=');
  if ( eq != NULL ) {
    *(eq++) = '\0';

    if ( strcmp(source, "text") == 0 ) {
      type = PATTERN_TYPE_TEXT;
    }
    else if ( strcmp(source, "image") != 0 ) {
      error(NULL, "Illegal pattern type '%s'", eq);
      return NULL;
    }

    source = eq;
  }
  else {
    int last = strlen(source) - 1;

    if ( (source[0] == '/') && (last > 0) && (source[last] == '/') ) {
      source[last] = '\0';
      source++;
      type = PATTERN_TYPE_TEXT;
    }
  }

  if ( ptype != NULL )
    *ptype = type;

  return source;
}


static int match_command_add(char *id, int argc, char **argv)
{
  char *hdr = log_hdr_(MATCH_TAG);
  frame_geometry_t g = FRAME_GEOMETRY_NULL;
  pattern_mode_t mode = PATTERN_MODE_APPEAR;
  pattern_type_t type = PATTERN_TYPE_NONE;
  char *source = NULL;
  frame_t *frame = NULL;
  pattern_t *pattern = NULL;
  GList *options = NULL;
  int argx;
  int err = 0;

  /* Retrieve pattern options */
  for (argx = 0; (argx < argc) && (err == 0); argx++) {
    char *args = strdup(argv[argx]);
    char *eq = strchr(args, '=');
    char *str = NULL;
    param_spec_t *spec;

    if ( eq != NULL ) {
      *eq = '\0';
      str = eq + 1;
    }

    spec = match_command_params;
    while ( spec->id != NULL ) {
      if ( strcmp(args, spec->id) == 0 ) {
	break;
      }

      spec++;
    }

    if ( spec->id != NULL ) {
      switch ( spec->type ) {
      case PARAM_TYPE_WINDOW:
	if ( str == NULL ) {
	  error(NULL, "Missing window geometry specification");
	  err = 1;
	}
	else {
	  if ( frame_geometry_parse(str, &g) ) {
	    error(NULL, "Syntax error in window geometry specification");
	    err = 1;
	  }
	}
	break;
      case PARAM_TYPE_FRAME:
	if ( str == NULL ) {
	  error(NULL, "Missing frame id");
	  err = 1;
	}
	else {
	  frame = frame_get_child_by_id(match->display->root, str);
	  if ( frame == NULL ) {
	    error(NULL, "Unknown frame '%s'", str);
	    err = 1;
	  }
	}
	break;
      case PARAM_TYPE_SOURCE:
	if ( str == NULL ) {
	  error(NULL, "Missing source specification (%s)", args);
	  err = 1;
	}
	else if ( source != NULL ) {
	  error(NULL, "Multiple source definition (%s,%s)", source, args);
	  err = 1;
	}
	else {
	  *eq = '=';
	  source = strdup(args);
	}
	break;
      case PARAM_TYPE_MODE:
	{
	  int bool = 1;

	  if ( str != NULL ) {
	    if ( strcmp(str, "yes") == 0 ) {
	      bool = 1;
	    }
	    else if ( strcmp(str, "no") == 0 ) {
	      bool = 0;
	    }
	    else {
	      error(NULL, "Option '%s' value should be 'yes' or 'no'", spec->id);
	      err = 1;
	    }
	  }

	  if ( bool )
	    mode |= spec->mask;
	  else
	    mode &= ~(spec->mask);
	}
	break;
      }

      free(args);
    }
    else {
      if ( eq != NULL )
       *eq = '=';
      options = g_list_append(options, args);
    }
  }

  if ( err ) {
    if ( source != NULL )
      free(source);
    return -1;
  }

  /* Set Root frame buffer by default */
  if ( frame == NULL )
    frame = match->display->root;

  /* Take the last argument as source specification,
     if not previously defined using the <type>=<spec> statement */
  if ( source == NULL ) {
    if ( argv[argx] == NULL ) {
      error(NULL, "Missing pattern source definition");
    }
    else {
      source = strdup(argv[argx]);
    }
  }

  /* Extract pattern type from source specification */
  if ( source != NULL ) {
    char *source2 = match_command_parse_source(source, &type);
    if ( type == PATTERN_TYPE_NONE ) {
      error(NULL, "Unrecognized pattern type");
    }
    else {
      /* Add/redefine new pattern */
      pattern = match_add(match, id, frame, NULL, mode, type, source2);
      if ( pattern == NULL ) {
	error(NULL, "Failed to add pattern %s", id);
      }
    }

    free(source);
  }

  /* Set pattern options */
  if ( pattern != NULL ) {
    char *errmsg = NULL;

    if ( pattern_set_options(pattern, options, &errmsg) ) {
      error(NULL, "%s: %s", id, errmsg);
      free(errmsg);
      err = 1;
    }
  }

  if ( err && (pattern != NULL) ) {
    match_remove_pattern(pattern);
    pattern = NULL;
  }

  if ( pattern == NULL )
    return -1;

  /* Set match window */
  frame_clip_geometry(frame, &g);

  if ( pattern_set_window(pattern, &g) ) {
    match_remove_pattern(pattern);
    return -1;
  }

  /* Update display tool pattern list */
  frame_display_pattern_add(match->display, pattern);

  match_show_pattern(pattern, hdr);
  match_process(pattern, NULL);

  return 0;
}


static int match_command(shell_t *shell, shell_argv_t *cmd_argv, char *tag)
{
  int argc = cmd_argv->argc;
  char **argv = cmd_argv->argv;
  char *hdr = log_hdr_(tag);
  char *op;
  char *id;

  /* Set error reporting tag */
  error_default_tag(tag);

  if ( argc < 2 ) {
    shell_std_help(shell, argv[0]);
    return -1;
  }
  op = argv[1];
  id = argv[2];

  /* Operation 'add' */
  if ( strcmp(op, "add") == 0 ) {
    /* Check number of arguments */
    if ( check_argc(shell, cmd_argv, tag, 3, 100) )
      return -1;

    /* Process 'match add' command */
    if ( match_command_add(id, argc-3, argv+3) )
      return -1;
  }

  /* Operation 'reset' */
  else if ( strcmp(op, "reset") == 0 ) {
    /* Check number of arguments */
    if ( check_argc(shell, cmd_argv, tag, 2, 3) )
      return -1;

    if ( match_reset(match, id) ) {
      error(NULL, "reset: Unknown pattern %s", id);
      return -1;
    }
  }

  /* Operation 'remove' */
  else if ( strcmp(op, "remove") == 0 ) {
    /* Check number of arguments */
    if ( check_argc(shell, cmd_argv, tag, 2, 3) )
      return -1;

    if ( match_remove(match, id) ) {
      error(NULL, "remove: Unknown pattern %s", id);
      return -1;
    }
  }

  /* Operation 'show' */
  else if ( strcmp(op, "show") == 0 ) {
    /* Check number of arguments */
    if ( check_argc(shell, cmd_argv, tag, 2, 3) )
      return -1;

    match_show(match, id, hdr);
  }

  /* Unknown operation */
  else {
    error(NULL, "Unknown operation '%s'", op);
    shell_std_help(shell, argv[0]);
    return -1;
  }

  return 0;
}


/*******************************************************/
/* Command interpreter setup                           */
/*******************************************************/

static shell_std_help_t match_command_help[] = {
  { "match",   "match add <id> [frame=<id>] [window=<geometry>] [appear[=yes|no]] [disappear[=yes|no]] [retrigger[=yes|no]] [<options>[=<value>]...] <source>\n"
               "  Start a job that matches display frames with <source> specification (image file or text).\n"
               "  The pattern matching job is given the identifier <id>.\n"
               "  If frame=<id> is specified, pattern matching is performed on this frame (root frame by default)\n"
               "  If window=<geometry> is specified, pattern matching is performed within this window.\n"
               "  Detection mode is specified by setting flags appear/disappear/retrigger.\n"
               "match reset [<id>]\n"
               "  Reset pattern matching job <id>, so that it will be (re)scaned for new matching.\n"
               "  All jobs are resetted if <id> is omitted.\n"
               "match remove [<id>]\n"
               "  Stop and remove pattern matching job <id>, or all jobs if <id> is omitted.\n"
               "match show [<id>]\n"
               "  Show pattern matching job <id>, or all jobs if <id> is omitted.\n" },
  { NULL,     NULL }
};


static shell_command_t match_command_set[] = {
  { "match",   match_command,   MATCH_TAG },
  { NULL,      shell_std_unknown, NULL }
};


int match_command_init(shell_t *shell, frame_display_t *display)
{
  /* Setup pattern matching gears */
  match = match_alloc(display);
  if ( match == NULL )
    exit(1);

  /* Init image pattern matching */
  match_image_init();

  /* Setup match commands */
  shell_set_cmd(shell, match_command_set);
  shell_std_set_help(match_command_help);

  return 0;
}


void match_command_done(shell_t *shell)
{
  /* Terminate image pattern matching */
  match_image_done();

  /* Remove match commands */
  if ( shell != NULL ) {
    shell_std_unset_help(match_command_help);
    shell_unset_cmd(shell, match_command_set);
  }

  /* Free pattern matching */
  if ( match != NULL )
    match_destroy(match);
  match = NULL;
}
