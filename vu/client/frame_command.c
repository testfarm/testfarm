/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* Frame Management command                                                 */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 09-NOV-2007                                                    */
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
#include "color.h"
#include "filter.h"
#include "frame.h"
#include "match.h"
#include "frame_command.h"


static frame_display_t *frame_command_display = NULL;


void frame_updated(frame_t *frame, frame_geometry_t *g)
{
  frame_display_refresh(frame_command_display, frame, g);
}


/*******************************************************/
/* The 'capture' command                               */
/*******************************************************/

static int capture_command(shell_t *shell, shell_argv_t *cmd_argv, char *tag)
{
  int argc = cmd_argv->argc;
  char **argv = cmd_argv->argv;
  char *hdr = log_hdr_(tag);
  frame_t *root = frame_command_display->root;
  capture_t *capture = root->capture;
  int ret = 0;

  /* Set error reporting tag */
  error_default_tag(tag);

  /* Check number of arguments */
  if ( check_argc(shell, cmd_argv, tag, 2, 3) )
    return -1;

  /* Operation 'geometry': change capture active window */
  if ( strcmp(argv[1], "geometry") == 0 ) {
    frame_geometry_t g;

    if ( argc > 2 ) {
      char *s_geom = argv[2];

      /* Retrieve frame geometry */
      if ( strcmp(s_geom, "full") != 0 ) {
	if ( frame_rgb_parse_geometry(&(root->hdr.fb->rgb), s_geom, &g) ) {
	  error(NULL, "geometry: Syntax error");
	  return -1;
	}
      }
      else {
	g = root->hdr.g0;
      }

      if ( frame_display_geometry(frame_command_display, &g) == 0 ) {
	capture_set_window(capture, &g);
        capture_refresh(capture);
      }
      else {
        error(tag, "geometry: Illegal parameters");
        ret = -1;
      }
    }

    capture_get_window(capture, &g);
    printf("%sGEOMETRY %s\n", hdr, frame_geometry_str(&g));
  }

  /* Operation 'refresh': change capture refresh rate */
  else if ( strcmp(argv[1], "refresh") == 0 ) {
    if ( argc > 2 ) {
      char *s = argv[2];

      if ( strcmp(s, "now") == 0 ) {
        capture_refresh(capture);
        printf("%sREFRESH now\n", hdr);
        return 0;
      }
      else {
	int period = capture_set_period(capture, atoi(s));

	if ( period >= 0 ) {
	  frame_display_period(frame_command_display, period);
	}
        else if ( period < 0 ) {
          error(tag, "Refresh period should be %d to %d ms\n", CAPTURE_PERIOD_MIN, CAPTURE_PERIOD_MAX);
          ret = -1;
        }
      }
    }

    printf("%sREFRESH %ld ms\n", hdr, capture_get_period(capture));
  }

  /* Operation 'attr': get/set capture device attribute */
  else if ( strcmp(argv[1], "attr") == 0 ) {
    char *key = NULL;
    int nmemb = 0;
    capture_attr_t *tab;
    int i;

    if ( argc > 2 ) {
      char *value;

      key = argv[2];
      value = strchr(key, '=');

      if ( value != NULL ) {
	*(value++) = '\0';
	if ( capture_attr_set(capture, key, value) ) {
	  error(tag, "Cannot set attribute '%s'\n", key);
	  return -1;
	}
      }
    }

    tab = capture_attr_get(capture, key, &nmemb);

    if ( tab == NULL ) {
      if ( key != NULL )
	error(tag, "Cannot get attribute '%s'\n", key);
      else
	error(tag, "Cannot get attributes\n");
      return -1;
    }

    for (i = 0; i < nmemb; i++) {
      printf("%sATTR %s", hdr, tab[i].key);
      if ( tab[i].value != NULL )
	printf("=%s", tab[i].value);
      printf("\n");
    }
  }

  /* Unknown operation */
  else {
    error(tag, "Unknown qualifier '%s'", argv[1]);
    shell_std_help(shell, argv[0]);
    return -1;
  }

  return ret;
}


/*******************************************************/
/* The 'frame' command                                 */
/*******************************************************/

static void frame_command_remove_guts(frame_t *frame, char *hdr)
{
  /* Remove attached patterns */
  match_remove_frame(frame);

  /* Remove frame from Display Tool */
  frame_display_frame_remove(frame_command_display, frame);

  /* Destroy frame */
  frame_free(frame, hdr);
}


static frame_t *frame_command_get_frame(char *id)
{
  frame_t *frame;

  frame = frame_get_child_by_id(frame_command_display->root, id);
  if ( frame == NULL ) {
    error(NULL, "Unknown frame '%s'", id);
  }

  return frame;
}


static int frame_command_add(char *id, int argc, char **argv)
{
  frame_t *parent = NULL;
  frame_geometry_t g = FRAME_GEOMETRY_NULL;
  unsigned int flags = 0;
  frame_t *frame;
  int argx;

  /* Parse command arguments */
  for (argx = 0; argx < argc; argx++) {
    char *str = argv[argx];
    char *eq = strchr(str, '=');

    if ( eq != NULL ) {
      *(eq++) = '\0';

      if ( strcmp(str, "frame") == 0 ) {
	if ( eq[0] != '\0' ) {
	  if ( (parent = frame_command_get_frame(eq)) == NULL )
	    return -1;
	}
      }
      else if ( strcmp(str, "window") == 0 ) {
	if ( frame_geometry_parse(eq, &g) ) {
	  error(NULL, "Syntax error in window geometry specification");
	  return -1;
	}
      }
      else {
	error(NULL, "Unknown option '%s'", str);
	return -1;
      }
    }
    else {
      if ( strcmp(str, "lazy") == 0 ) {
	flags |= FRAME_REFRESH_LAZY;
      }
      else {
	error(NULL, "Illegal argument '%s'", str);
	return -1;
      }
    }
  }

  /* Delete already existing frame */
  frame = frame_get_child_by_id(frame_command_display->root, id);
  if ( frame != NULL ) {
    frame_command_remove_guts(frame, NULL);
    frame = NULL;
  }

  /* Use root frame as default parent frame */
  if ( parent == NULL )
    parent = frame_command_display->root;

  /* Alloc new frame */
  frame = frame_alloc_child(id, parent, &g);
  if ( frame == NULL )
    return -1;
  frame->refresh_flags = flags;

  /* Show newly created frame */
  frame_show(frame, log_hdr_(FRAME_TAG));

  /* Refresh frame */
  frame_update_all(frame);

  /* Add newly created frame to the Display Tool */
  frame_display_frame_add(frame_command_display, frame);

  return 0;
}


static void frame_command_filter_show_classes(void)
{
  printf("%sAvailable filter classes:", log_hdr_(FRAME_TAG));
  filter_show_classes();
  printf("\n");
}


static int frame_command_filter(char *id, int argc, char **argv)
{
  frame_t *frame;
  char *class_id;
  GList *options = NULL;
  filter_t *filter;
  int argx;

  if ( id == NULL ) {
    frame_command_filter_show_classes();
    return 0;
  }

  /* Retrieve frame descriptor */
  if ( (frame = frame_command_get_frame(id)) == NULL )
    return -1;

  if ( argc < 1 ) {
    error(NULL, "Missing filter class specification");
    frame_command_filter_show_classes();
    return -1;
  }
  class_id = argv[0];

  /* Parse command arguments */
  for (argx = 1; argx < argc; argx++) {
    char *str = argv[argx];
    options = g_list_append(options, str);
  }

  /* Alloc filter */
  filter = filter_alloc(class_id, options);

  g_list_free(options);

  if ( filter == NULL ) {
    frame_command_filter_show_classes();
    return -1;
  }

  /* Add item to the filter chain */
  frame_add_filter(frame, filter);

  /* Show frame with its new filter list */
  frame_show(frame, log_hdr_(FRAME_TAG));

  /* Refresh frame for applying filters */
  frame_update_all(frame);

  return 0;
}


static void frame_command_ocr_show(frame_t *frame, char *hdr)
{
  char *str = frame_get_ocr_agent(frame);
  printf("%s%s OCR %s\n", hdr, frame->hdr.id, str);
  free(str);
}


static int frame_command_ocr(char *id, int argc, char **argv)
{
  char *hdr = strdup(log_hdr_(FRAME_TAG));
  frame_t *frame;

  if ( id != NULL ) {
    /* Retrieve frame descriptor (root allowed) */
    if ( strcmp(id, "(root)") == 0 ) {
      frame = frame_command_display->root;
    }
    else {
      if ( (frame = frame_command_get_frame(id)) == NULL )
	return -1;
    }

    /* Set OCR agent */
    if ( argc > 0 ) {
      if ( frame_set_ocr_agent(frame, argv[0], argc-1, argv+1) ) {
	error(NULL, "Unknown OCR agent '%s'", argv[0]);
	return -1;
      }
    }

    /* Show OCR agent for this frame */
    frame_command_ocr_show(frame, hdr);
  }
  else {
    /* Show OCR agent for all frames */
    frame_command_ocr_show(frame_command_display->root, hdr);
    frame_foreach_child(frame_command_display->root, (GFunc) frame_command_ocr_show, hdr, 1);
  }

  return 0;
}


static int frame_command_remove(char *id)
{
  char *hdr = strdup(log_hdr_(FRAME_TAG));

  if ( id != NULL ) {
    frame_t *frame = frame_command_get_frame(id);

    if ( frame == NULL )
      return -1;

    frame_command_remove_guts(frame, hdr);
  }
  else {
    frame_foreach_child(frame_command_display->root, (GFunc) frame_command_remove_guts, hdr, 1);
  }

  free(hdr);

  return 0;
}


static int frame_command_show(char *id)
{
  char *hdr = log_hdr_(FRAME_TAG);

  if ( id != NULL ) {
    frame_t *frame = frame_command_get_frame(id);

    if ( frame == NULL )
      return -1;

    frame_show(frame, hdr);
  }
  else {
    g_list_foreach(frame_command_display->root->children, (GFunc) frame_show_tree, hdr);
  }

  return 0;
}


static int frame_command_refresh(char *id)
{
  char *hdr = log_hdr_(FRAME_TAG);
  frame_t *frame;

  if ( id != NULL ) {
    frame = frame_command_get_frame(id);
    if ( frame == NULL )
      return -1;
  }
  else {
    frame = frame_command_display->root;
  }

  printf("%s%s REFRESH\n", hdr, frame->hdr.id);

  frame_update_all(frame);

  return 0;
}


static int frame_command(shell_t *shell, shell_argv_t *cmd_argv, char *tag)
{
  int argc = cmd_argv->argc;
  char **argv = cmd_argv->argv;
  char *op;
  char *id;
  int ret = 0;

  /* Set error reporting tag */
  error_default_tag(tag);

  /* Check number of arguments */
  if ( argc < 2 ) {
    shell_std_help(shell, argv[0]);
    return -1;
  }
  op = argv[1];
  id = argv[2];

  /* Operation 'geometry' and 'refresh': same as command capture
     (deprecated command compatibility) */
  if ( strcmp(op, "geometry") == 0 ) {
    capture_command(shell, cmd_argv, tag);
  }

  /* The 'add' operation */
  else if ( strcmp(op, "add") == 0 ) {
    /* Check number of arguments */
    if ( argc < 3 ) {
      shell_std_help(shell, argv[0]);
      return -1;
    }

    /* Process 'frame add' command */
    if ( frame_command_add(id, argc-3, argv+3) )
      return -1;
  }

  /* The 'remove' operation */
  else if ( strcmp(op, "remove") == 0 ) {
    /* Check number of arguments */
    if ( argc > 3 ) {
      shell_std_help(shell, argv[0]);
      return -1;
    }

    /* Process 'frame remove' command */
    if ( frame_command_remove(id) )
      return -1;
  }

  /* The 'show' operation */
  else if ( strcmp(op, "show") == 0 ) {
    /* Check number of arguments */
    if ( argc > 3 ) {
      shell_std_help(shell, argv[0]);
      return -1;
    }

    /* Process 'frame show' command */
    if ( frame_command_show(id) )
      return -1;
  }

  /* The 'refresh' operation */
  else if ( strcmp(op, "refresh") == 0 ) {
    /* Check number of arguments */
    if ( argc > 3 ) {
      shell_std_help(shell, argv[0]);
      return -1;
    }

    /* Process 'frame refresh' command */
    if ( frame_command_refresh(id) )
      return -1;
  }

  /* The 'filter' operation */
  else if ( strcmp(op, "filter") == 0 ) {
    /* Process 'frame filter' command */
    if ( frame_command_filter(id, argc-3, argv+3) )
      return -1;
  }

  /* The 'ocr' operation */
  else if ( strcmp(op, "ocr") == 0 ) {
    /* Process 'frame filter' command */
    if ( frame_command_ocr(id, argc-3, argv+3) )
      return -1;
  }

  /* Unknown operation */
  else {
    error(tag, "Unknown qualifier '%s'", argv[1]);
    shell_std_help(shell, argv[0]);
    return -1;
  }

  return ret;
}


/*******************************************************/
/* Command interpreter setup                           */
/*******************************************************/

static shell_std_help_t frame_command_help[] = {
  { "capture", "capture geometry [<geometry>|full]\n"
               "  Set and show capture active window to <geometry> (XWindow geometry format).\n"
               "capture refresh [<period>]\n"
               "  Set and show capture refresh delay to <period> milliseconds.\n"
               "capture refresh now\n"
               "  Update capture frame right now.\n"
               "capture attr [<label>[=<value>]]\n"
               "  Set and show capture device attribute <label>. Show all attributes if no label is specified.\n" },
  { "frame",   "frame add <id> [parent=<parent-id>] [window=<geometry>]\n"
               "  Create a new frame <id> attached to parent frame <parent-id> (root frame by default)\n"
               "  If window=<geometry> is specified, new frame is located at this parent's area.\n"
               "frame filter <id> <class> [lazy] [<options>...]\n"
               "  Add a filter of class <class> tuned with <options>.\n"
               "  Refer to filter documentation for class-specific options.\n"
               "frame ocr <id> <ocr-agent> [<ocr-agent-options>...]\n"
               "  Set OCR agent configuration for frame <id>.\n"
               "frame refresh [<id>]\n"
               "  Force frame <id> refresh and processing (root frame by default).\n"
               "frame remove [<id>]\n"
               "  Remove frame <id> and all depending frames. Remove all frames if <id> is omitted\n"
               "frame show [<id>]\n"
               "  Show a description of frame <id>. Show all frames if <id> is omitted\n" },
  { NULL,     NULL }
};


static shell_command_t frame_command_set[] = {
  { "capture", capture_command, CAPTURE_TAG },
  { "frame",   frame_command,   FRAME_TAG },
  { NULL,      shell_std_unknown, NULL }
};


int frame_command_init(shell_t *shell, frame_display_t *display)
{
  /* Init frame filters */
  filter_init();

  /* Setup frame commands */
  shell_set_cmd(shell, frame_command_set);
  shell_std_set_help(frame_command_help);

  /* Set display pointer */
  frame_command_display = display;

  return 0;
}


void frame_command_done(shell_t *shell)
{
  /* Shut down frame filters */
  filter_done();

  /* Clear display pointer */
  frame_command_display = NULL;

  /* Remove frame commands */
  if ( shell != NULL ) {
    shell_std_unset_help(frame_command_help);
    shell_unset_cmd(shell, frame_command_set);
  }
}
