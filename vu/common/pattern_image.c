/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* Image Pattern management                                                 */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 27-JUN-2007                                                    */
/****************************************************************************/

/*
 * $Revision: 1131 $
 * $Date: 2010-03-31 16:01:06 +0200 (mer., 31 mars 2010) $
 */

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <malloc.h>
#include <unistd.h>

#include "image_file.h"
#include "ppm.h"
#include "png_file.h"
#include "color.h"
#include "frame_ctl.h"
#include "pattern.h"
#include "pattern_image.h"


static void pattern_image_err(char **perr, char *fmt, ...)
{
  va_list ap;
  char str[256];

  if ( perr == NULL )
    return;

  va_start(ap, fmt);
  vsnprintf(str, sizeof(str), fmt, ap);
  va_end(ap);

  *perr = strdup(str);
}


int pattern_image_set(pattern_t *pattern, char *filename, char **perr)
{
  int len = strlen(filename);
  char *fname;
  unsigned int width, height;
  unsigned char *buf = NULL;
  int ret = 0;

  /* Clear error message ptr */
  if ( perr != NULL )
    *perr = NULL;

  /* Free image buffer */
  pattern_image_free(pattern);

  if ( filename[0] == '#' ) {
    fname = strdup(filename);

    ret = color_fill_from_spec(filename, &width, &height, &buf);
    if ( ret ) {
      pattern_image_err(perr, "Syntax error in color fill specification", "");
    }
  }
  else {
    fname = malloc(len+11);
    strcpy(fname, filename);

    /* Try PNG file first */
    if ( access(fname, F_OK) ) {
      strcpy(fname + len, PNG_SUFFIX);

      /* Then try PPM */
      if ( access(fname, F_OK) ) {
	strcpy(fname + len, PPM_SUFFIX);
	if ( access(fname, F_OK) ) {
	  fname[len] = '\0';
	}
      }
    }

    if ( access(fname, F_OK) ) {
      pattern_image_err(perr, "Cannot find PNG or PPM image file %s", fname);
      ret = -1;
    }

    /* Load image file */
    else {
      unsigned char *alpha = NULL;

      ret = image_load(fname, &width, &height, &buf, &alpha, perr);

      /* Set mask if an alpha channel is present */
      if ( alpha != NULL ) {
	pattern_mask_set_alpha(&pattern->d.image.mask, width, height, alpha);
      }
    }
  }

  if ( ret ) {
    if ( buf != NULL )
      free(buf);
    free(fname);
  }
  else {
    /* Set image source */
    if ( pattern->source != NULL )
      free(pattern->source);
    pattern->source = fname;

    /* Set image data */
    pattern->d.image.width = width;
    pattern->d.image.height = height;
    pattern->d.image.fuzz = fuzz_null;
    pattern->d.image.buf = buf;

    /* Set number of unmasked pixels */
    pattern->d.image.npixels = width * height;

    /* Clear match constraints */
    pattern->d.image.badpixels_rate = 0;
    pattern->d.image.badpixels_max = 0;

    pattern->d.image.potential_rate = 100;
    pattern->d.image.potential_scale = 0;
    pattern->d.image.potential_max = 0;
    pattern->d.image.potential = 0;
  }

  return ret;
}


int pattern_image_set_options(pattern_t *pattern, GList *options, char **perr)
{
  GList *l = options;
  int ret = 0;

  while ( (ret == 0) && (l != NULL) ) {
    char *args = l->data;
    char *eq = strchr(args, '=');

    if ( eq != NULL ) {
      char *str = eq + 1;

      *eq = '\0';

      if ( strcmp(args, "mask") == 0 ) {
	unsigned int width, height;
	unsigned char *buf = NULL;

	ret = image_load(str, &width, &height, &buf, NULL, perr);

	if ( ret == 0 ) {
	  pattern_mask_set_rgb(&pattern->d.image.mask, str, width, height, buf);
	  pattern_mask_clip(&pattern->d.image.mask, pattern->d.image.width, pattern->d.image.height);
	}
      }
      else if ( strcmp(args, "fuzz") == 0 ) {
	fuzz_t fuzz = FUZZ_NULL;

	if ( fuzz_parse(&fuzz, str) == 0 ) {
	  pattern->d.image.fuzz = fuzz;
	}
	else {
	  *perr = strdup("Syntax error in fuzz specification");
	  ret = -1;
	}
      }
      else if ( strcmp(args, "loss") == 0 ) {
	char *comma = strchr(str, ',');
	int i;

	if ( comma != NULL ) {
	  *(comma++) = '\0';

	  i = atoi(comma);
	  if ( i < 0 )
	    i = 0;
	  if ( i > 100 )
	    i = 100;
	  pattern->d.image.potential_rate = i;
	}

	i = atoi(str);
	if ( i < 0 )
	  i = 0;
	if ( i > 100 )
	  i = 100;
	pattern->d.image.badpixels_rate = i;
      }
      else {
	int size = strlen(args) + 40;
      
	*perr = (char *) malloc(size);
	snprintf(*perr, size, "Illegal option for image pattern: %s", args);
	ret = -1;
      }

      *eq = '=';
    }
    else {
      if ( strcmp(args, "map") == 0 ) {
	pattern->d.image.opt |= PATTERN_IMAGE_OPT_MAP;
      }
      else if ( strcmp(args, "nomask") == 0 ) {
	/* Kill the mask buffer that may have been created when loading
	   a PNG alpha channel */
	pattern_mask_clear(&pattern->d.image.mask);
      }
      else {
	int size = strlen(args) + 50;

	*perr = (char *) malloc(size);
	snprintf(*perr, size, "Syntax error in %s specification: missing value", args);
	ret = -1;
      }
    }

    l = g_list_next(l);
  }

  if ( ret == 0 ) {
    int i;

    /* Compute number of unmasked pixels */
    pattern->d.image.npixels = (pattern->d.image.mask.buf != NULL) ?
      pattern->d.image.mask.n :
      (pattern->d.image.width * pattern->d.image.height);

    /* Compute quality rejection threshold */
    pattern->d.image.badpixels_max = (pattern->d.image.badpixels_rate * pattern->d.image.npixels) / 100;

    /* Compute potential scale */
    pattern->d.image.potential_scale = 0;
    for (i = 0; i < 3; i++)
      pattern->d.image.potential_scale += (255 - pattern->d.image.fuzz.color[i]);
    pattern->d.image.potential_scale *= pattern->d.image.npixels;

    /* Compute maximum acceptable potential */
    pattern->d.image.potential_max = (pattern->d.image.potential_rate * pattern->d.image.potential_scale) / 100;
  }

  return ret;
}


void pattern_image_free(pattern_t *pattern)
{
  if ( pattern->d.image.buf != NULL )
    free(pattern->d.image.buf);
  pattern->d.image.buf = NULL;

  pattern_mask_clear(&pattern->d.image.mask);
}


void pattern_image_copy(pattern_t *pattern, pattern_t *pattern0)
{
  if ( pattern0->d.image.buf != NULL ) {
    pattern->d.image.buf = image_dup(pattern0->d.image.buf, pattern0->d.image.width, pattern0->d.image.height);
  }

  pattern_mask_copy(&pattern->d.image.mask, &pattern0->d.image.mask);

  pattern->d.image.fuzz = pattern0->d.image.fuzz;
  pattern->d.image.badpixels_rate = pattern0->d.image.badpixels_rate;
  pattern->d.image.potential_rate = pattern0->d.image.potential_rate;
}


int pattern_image_str(pattern_t *pattern, char *buf, int size)
{
	int len = 0;

	if (!fuzz_is_null(&(pattern->d.image.fuzz)))
		len += snprintf(buf+len, size-len, " fuzz=%s", fuzz_str(&(pattern->d.image.fuzz)));
	if ( pattern->d.image.mask.source != NULL )
		len += snprintf(buf+len, size-len, " mask=%s", pattern->d.image.mask.source);
	if ( pattern->d.image.badpixels_rate != 0 ) {
		len += snprintf(buf+len, size-len, " loss=%u", pattern->d.image.badpixels_rate);
		if ( pattern->d.image.potential_rate != 100 )
			len += snprintf(buf+len, size-len, ",%u", pattern->d.image.potential_rate);
	}

	return len;
}


int pattern_image_diff(pattern_t *p1, pattern_t *p2)
{
	  if (memcmp(&p1->d.image.fuzz, &p2->d.image.fuzz, sizeof(fuzz_t)) != 0)
		  return 1;
	  if (p1->d.image.badpixels_rate != p2->d.image.badpixels_rate)
		  return 1;
	  if (p1->d.image.potential_rate != p2->d.image.potential_rate)
		  return 1;
	  return 0;
}
