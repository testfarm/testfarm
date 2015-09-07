/**********************************************************************/
/* TestFarm Virtual User                                              */
/* Grab command and utilities                                         */
/**********************************************************************/
/* Author: Sylvain Giroudon                                           */
/* Creation: 29-AUG-2007                                              */
/**********************************************************************/

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

#include "shell.h"

#include "error.h"
#include "frame_geometry.h"
#include "ppm.h"
#include "png_file.h"
#include "log.h"
#include "tstamp.h"
#include "grab.h"


/*******************************************************/
/* Frame recording                                     */
/*******************************************************/

static frame_display_t *grab_display = NULL;
static tstamp_t grab_begin = 0;        /* Grab begin date */
static tstamp_t grab_end = 0;          /* Grab end date */
static char *grab_fname_fmt = NULL;
static int grab_fname_size = 0;
static char *grab_fname = NULL;
static frame_t *grab_frame = NULL;
static frame_geometry_t grab_window = FRAME_GEOMETRY_NULL;
static unsigned int grab_n = 0;        /* Number of frames recorded */
static int grab_ppm = 0;               /* PPM.gz output format */


static int grab_save(unsigned long now)
{
  int ret;

  if ( grab_fname != NULL )
    free(grab_fname);
  grab_fname = NULL;

  if ( grab_fname_size <= 0 )
    return -1;
  if ( grab_fname_fmt == NULL )
    return -1;

  grab_fname = (char *) malloc(grab_fname_size);
  snprintf(grab_fname, grab_fname_size, grab_fname_fmt, now);

  if ( grab_ppm )
    ret = ppm_save(&(grab_frame->hdr.fb->rgb), &grab_window, grab_fname);
  else
    ret = png_save(&(grab_frame->hdr.fb->rgb), &grab_window, grab_fname);

  if ( ret == 0 )
    grab_n++;

  return ret;
}


void grab_update(frame_geometry_t *g)
{
  if ( grab_end > 0 ) {
    tstamp_t tstamp = tstamp_get();

    if ( frame_geometry_overlaps(&grab_window, g) ) {
      unsigned long now = (tstamp / 1000ULL);

      if ( now < grab_end ) {
	if ( grab_save(now - grab_begin) ) {
	  printf("%sFailed to write image file '%s'", log_hdr(tstamp, GRAB_TAG), grab_fname);
	  grab_end = 0;
	}
      }
      else {
	grab_end = 0;
      }
    }

    if ( grab_end == 0 ) {
      printf("%s%d frames recorded\n", log_hdr(tstamp, GRAB_TAG), grab_n);
      grab_n = 0;
    }
  }
}


/*******************************************************/
/* The 'grab' command                                  */
/*******************************************************/

static int grab_command(shell_t *shell, shell_argv_t *cmd_argv, char *tag)
{
  int argc = cmd_argv->argc;
  char **argv = cmd_argv->argv;
  char *hdr = log_hdr_(tag);
  int ret = -1;

  /* Set error reporting tag */
  error_default_tag(tag);

  /* The 'stop' qualifier */
  if ( (argc > 1) && (strcmp(argv[1], "stop") == 0) ) {
    /* Check number of arguments */
    if ( check_argc(shell, cmd_argv, tag, 2, 2) )
      return -1;

    grab_end = 0;

    if ( grab_n > 0 ) {
      printf("%s%d frames recorded\n", hdr, grab_n);
      grab_n = 0;
    }
    else {
      printf("%sNo recording in progress\n", hdr);
    }

    ret = 0;
  }

  /* Unknown qualifier */
  else {
    frame_t *frame = grab_display->root;
    char *window_str = NULL;
    frame_geometry_t window;
    char *fname = NULL;
    int grab_animation = 0;
    long grab_dt = 0;
    int n;

    /* Check number of arguments */
    if ( check_argc(shell, cmd_argv, tag, 1, 4) )
      return -1;

    for (n = 1; n < argc; n++) {
      char *str = argv[n];
      char *eq = strchr(str, '=');

      if ( eq != NULL ) {
	*(eq++) = '\0';
	if ( strcmp(str, "frame") == 0 ) {
	  frame = frame_get_child_by_id(grab_display->root, eq);
	  if ( frame == NULL ) {
	    error(NULL, "Unknown frame '%s'", eq);
	    return -1;
	  }
	}
	else if ( strcmp(str, "window") == 0 ) {
	  window_str = eq;
	}
	else if ( strcmp(str, "t") == 0 ) {
	  grab_begin = (tstamp_get() / 1000);
	  grab_end = 0;
	  grab_n = 0;
	  grab_dt = (atoi(eq) * 1000);
	  if ( grab_dt < 0 )
	    grab_dt = 0;

	  grab_end = grab_begin + grab_dt;
	  grab_animation = 1;
	}
	else {
	  error(tag, "Illegal qualifier '%s'", str);
	  return -1;
	}
      }
      else {
	fname = str;
      }
    }

    /* Set source frame */
    if ( window_str != NULL ) {
      if ( frame_rgb_parse_geometry(&(frame->hdr.fb->rgb), window_str, &window) ) {
	error(NULL, "Syntax error in window geometry");
	return -1;
      }
    }
    else {
      window = frame->g;
      window.x = 0;
      window.y = 0;
    }

    /* Retrieve output format */
    grab_ppm = (ppm_filename_suffix(fname) != NULL);

    /* Construct an image file name */
    fname = grab_ppm ? ppm_filename(fname) : png_filename(fname);
    if ( fname == NULL ) {
      error(tag, "Failed to construct a valid image file name");
      return -1;
    }

    if ( grab_animation ) {
      char *suffix_str;
      char *suffix;

      if ( grab_ppm ) {
	suffix_str = PPM_SUFFIX;
	suffix = ppm_filename_suffix(fname);
      }
      else {
	suffix_str = PNG_SUFFIX;
	suffix = png_filename_suffix(fname);
      }

      if ( suffix != NULL )
	*suffix = '\0';

      grab_fname_size = strlen(fname) + 16;
      if ( grab_fname_fmt != NULL )
	free(grab_fname_fmt);
      grab_fname_fmt = (char *) malloc(grab_fname_size);
      snprintf(grab_fname_fmt, grab_fname_size, "%s.%%09lu%s", fname, suffix_str);

      grab_frame = frame;
      grab_window = window;

      ret = grab_save(0);
      if ( ret == 0 ) {
	printf("%sframe=%s window=%s t=%ld %s.*%s\n", hdr,
	       frame->hdr.id, frame_geometry_str(&window),
	       grab_dt / 1000, fname, suffix_str);
      }
      else {
	printf("%sFailed to write image file '%s'", hdr, grab_fname);

	grab_end = 0;
      }
    }
    else {
      if ( grab_ppm )
	ret = ppm_save(&(frame->hdr.fb->rgb), &window, fname);
      else
	ret = png_save(&(frame->hdr.fb->rgb), &window, fname);

      if ( ret == 0 )
	printf("%sframe=%s window=%s %s\n", hdr, frame->hdr.id, frame_geometry_str(&window), fname);
      else
	printf("%sFailed to write image file '%s'", hdr, fname);
    }

    free(fname);
  }

  return ret;
}


/*******************************************************/
/* Command interpreter setup                           */
/*******************************************************/

static shell_std_help_t grab_help_tab[] = {
  { "grab",    "grab [frame=<id>] [window=<geometry>] <file>\n"
               "  Grab area <geometry> in PNG or gzip'd PPM format to <file>\n"
               "  If frame=<id> is specified, grabbing is performed on this frame (root frame by default)\n"
               "grab [frame=<id>] [window=<geometry>] t=<time>[/<div>] <file>\n"
               "  Grab area <geometry> every <div> frames during <time> seconds.\n"
               "  If frame=<id> is specified, grabbing is performed on this frame (root frame by default)\n"
               "  Grabbed frames are stored in separate PNG or gzip'd PPM files.\n"
               "grab stop\n"
               "  Stop grabbing process\n" },
  { NULL,     NULL }
};


static shell_command_t grab_cmd_tab[] = {
  { "grab",    grab_command,    GRAB_TAG  },
  { NULL,      shell_std_unknown, NULL }
};


int grab_init(shell_t *shell, frame_display_t *display)
{
  /* Setup grab commands */
  shell_set_cmd(shell, grab_cmd_tab);
  shell_std_set_help(grab_help_tab);

  /* Set display descriptor */
  grab_display = display;

  return 0;
}


void grab_done(shell_t *shell)
{
  /* Clear capture device pointer */
  grab_display = NULL;

  /* Remove grab commands */
  if ( shell != NULL ) {
    shell_std_unset_help(grab_help_tab);
    shell_unset_cmd(shell, grab_cmd_tab);
  }
}
