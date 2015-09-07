/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* Image file name constructor                                              */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 04-MAR-2008                                                    */
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
#include <ctype.h>
#include <glib.h>

#include "filelist.h"

#include "filelist.h"
#include "frame_rgb.h"
#include "ppm.h"
#include "png_file.h"
#include "jpeg.h"


#define eprintf(args...) fprintf(stderr, "testfarm-vu (image): " args)


char *image_filename(char *filename,
		     char *suffix_str,
		     char * (*suffix_fn)(char *filename),
		     char *suffix_re)
{
  int dirlen = 0;
  char *dirname = NULL;
  char *basename = NULL;
  char *prefix = NULL;
  char *number = NULL;
  unsigned long n0 = 0;
  char *fname = NULL;
  GList *list;
  int size;

  if ( filename == NULL ) {
    prefix = strdup("tvu");
    n0 = 1;
  }
  else {
    char *suffix;
    int i;

    /* Get dirname and basename */
    if ( strchr(filename, G_DIR_SEPARATOR) ) {
      dirname = g_path_get_dirname(filename);
      dirlen = strlen(dirname) + 1;
    }

    prefix = g_path_get_basename(filename);

    /* Strip suffix */
    suffix = suffix_fn(prefix);
    if ( suffix )
      *suffix = '\0';

    /* Strip backup number (if any) */
    i = strlen(prefix) - 1;
    while ( (i >= 0) && isdigit(prefix[i]) ) {
      number = prefix + i;
      i--;
    }

    if ( number ) {
      n0 = atoi(number);
      *number = '\0';
    }
  }

  /* Alloc file name buffer */
  size = strlen(prefix) + 32 + strlen(suffix_str) + strlen(suffix_re);
  fname = (char *) malloc(dirlen + size);

  if ( dirname )
    sprintf(fname, "%s" G_DIR_SEPARATOR_S, dirname);
  basename = fname + dirlen;

  snprintf(basename, size, "%s(\\d+)?(\\.\\d+)?%s$", prefix, suffix_re);
  list = filelist(fname);

  if ( list ) {
    unsigned long n = number ? n0 : 0;
    int found;
    GList *l;

    /* Truncate suffixes */
    l = list;
    while ( l != NULL ) {
      char *p = suffix_fn((char *) l->data);

      if ( p != NULL )
	*p = '\0';

      l = l->next;
    }

    do {
      if ( n > 0 )
	snprintf(basename, size, "%s%04lu", prefix, n);
      else
	snprintf(basename, size, "%s", prefix);

      found = 0;
      l = list;
      while ( (l != NULL) && (!found) ) {
	found = (strcmp(basename, (char *) l->data) == 0);
	l = l->next;
      }

      n++;
    } while ( found && (n < 10000) );

    if ( found ) {
      eprintf("Cannot set a new image file name with format '%sNNNN%s'\n", prefix, suffix_str);
      free(fname);
      fname = NULL;
    }

    filelist_free(list);
  }
  else {
    if ( number )
      snprintf(basename, size, "%s%04lu", prefix, n0);
    else
      snprintf(basename, size, "%s", prefix);
  }

  if ( dirname )
    free(dirname);
  free(prefix);

  strcat(basename, suffix_str);

  return fname;
}


unsigned char *image_dup(unsigned char *rgb_buf, unsigned int rgb_width, unsigned int rgb_height)
{
  unsigned char *target_buf;
  unsigned long rgb_size;

  rgb_size = frame_rgb_bufsize(rgb_width, rgb_height);
  target_buf = malloc(rgb_size + FRAME_RGB_MMTAIL);

  if ( rgb_buf != NULL ) {
    memcpy(target_buf, rgb_buf, rgb_size);
    memset(target_buf+rgb_size, 0, FRAME_RGB_MMTAIL);
  }
  else {
    memset(target_buf, 0, rgb_size+FRAME_RGB_MMTAIL);
  }

  return target_buf;
}


int image_load(char *fname,
	       unsigned int *pwidth, unsigned int *pheight,
	       unsigned char **pbuf, unsigned char **palpha,
	       char **perr)
{
  char *ftype = NULL;
  int ret = -1;

  /* Try JPEG file */
  if ( jpeg_filename_suffix(fname) != NULL ) {
    ftype = "JPEG";
    ret = jpeg_load(fname, pwidth, pheight, pbuf);
  }
  /* Try PNG file */
  else if ( png_filename_suffix(fname) != NULL ) {
    ftype = "PNG";
    ret = png_load_full(fname, pwidth, pheight, pbuf, palpha);
  }
  /* Try PPM file */
  else if ( ppm_filename_suffix(fname) != NULL ) {
    ftype = "PPM";
    ret = ppm_load(fname, pwidth, pheight, pbuf);
  }

  if ( ret && (perr != NULL) ) {
    char str[256];

    if ( ftype != NULL ) {
      snprintf(str, sizeof(str), "%s: Failed to load %s image file", fname, ftype);
    }
    else {
      snprintf(str, sizeof(str), "%s: Unknown image format", fname);
    }

    *perr = strdup(str);
  }

  return ret;
}
