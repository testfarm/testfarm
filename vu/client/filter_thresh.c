/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* RGB Threshold Filter                                                     */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 28-DEC-2007                                                    */
/****************************************************************************/

/*
 * $Revision: 1017 $
 * $Date: 2008-08-13 17:00:11 +0200 (mer., 13 août 2008) $
 */

#include <stdio.h>
#include <string.h>

#include "error.h"
#include "fuzz.h"
#include "filter.h"
#include "filter_thresh.h"


#define FILTER_COLOR_MASK_IGNORE 0x80000000
#define FILTER_COLOR_MASK_VALUE  0x00FFFFFF

#define FILTER_COLOR_SET(color,value) ((color) = ((value) & FILTER_COLOR_MASK_VALUE))
#define FILTER_COLOR_CLEAR(color) ((color) = FILTER_COLOR_MASK_IGNORE)

#define FILTER_COLOR_IGNORE(color) ((color) & FILTER_COLOR_MASK_IGNORE)
#define FILTER_COLOR_VALUE(color)  ((color) & FILTER_COLOR_MASK_VALUE)


#define FILTER_THRESH(_ptr_) ((filter_thresh_t *)(_ptr_))

typedef unsigned long filter_thresh_action_t;

typedef struct {
  filter_t hdr;
  unsigned long color_value;
  fuzz_t fuzz;
  filter_thresh_action_t accept;
  filter_thresh_action_t reject;
} filter_thresh_t;


static int filter_thresh_parse_action(char *action, char *eq, filter_thresh_action_t *ptr)
{
  if ( eq[0] == '*' ) {
    FILTER_COLOR_CLEAR(*ptr);
  }
  else {
    unsigned long value;

    if ( color_parse(eq, &value) == 0 ) {
      FILTER_COLOR_SET(*ptr, value);
    }
    else {
      error(NULL, "Syntax error in %s specification", action);
      return -1;
    }
  }

  return 0;
}


static int filter_thresh_parse(filter_thresh_t *filter, GList *options)
{
  /* Set default parameter values */
  filter->color_value = 0;
  filter->fuzz = fuzz_null;
  FILTER_COLOR_CLEAR(filter->accept);  /* Leave accepted colors unchanged */
  FILTER_COLOR_SET(filter->reject, 0); /* Paint rejected colors in black */

  /* Parse command arguments */
  while ( options ) {
    char *str = options->data;
    char *eq = strchr(str, '=');

    if ( eq != NULL ) {
      *(eq++) = '\0';

      if ( strcmp(str, "color") == 0 ) {
	if ( color_parse(eq, &(filter->color_value)) ) {
	  error(NULL, "Syntax error in filter color specification", str);
	  return -1;
	}
      }
      else if ( strcmp(str, "fuzz") == 0 ) {
	if ( fuzz_parse(&(filter->fuzz), eq) ) {
	  error(NULL, "Syntax error in filter fuzz specification");
	  return -1;
	}
      }
      else if ( strcmp(str, "accept") == 0 ) {
	if ( filter_thresh_parse_action(str, eq, &(filter->accept)) )
	  return -1;
      }
      else if ( strcmp(str, "reject") == 0 ) {
	if ( filter_thresh_parse_action(str, eq, &(filter->reject)) )
	  return -1;
      }
      else {
	error(NULL, "Unknown filter option '%s'", str);
	return -1;
      }
    }
    else {
      error(NULL, "Illegal filter argument '%s'", str);
      return -1;
    }

    options = g_list_next(options);
  }

  return 0;
}


static filter_t *filter_thresh_alloc(char *class_id, GList *options)
{
  filter_thresh_t *filter;

  filter = (filter_thresh_t *) malloc(sizeof(filter_thresh_t));
  memset(filter, 0, sizeof(filter_thresh_t));

  if ( filter_thresh_parse(filter, options) ) {
    free(filter);
    return NULL;
  }

  return FILTER(filter);
}


static char *rgb_str(unsigned long value)
{
  color_rgb_t rgb;
  static char str[16];

  color_set_rgb(value, rgb);
  snprintf(str, sizeof(str), "#%02X%02X%02X", rgb[0], rgb[1], rgb[2]);

  return str;
}


static void filter_thresh_show(filter_t *filter_, char *header, char *trailer)
{
  filter_thresh_t *filter = FILTER_THRESH(filter_);

  fputs(header, stdout);
  printf(" color=%s", rgb_str(filter->color_value));
  printf(" fuzz=%s", fuzz_str(&filter->fuzz));
  printf(" accept=%s", FILTER_COLOR_IGNORE(filter->accept) ? "*" : rgb_str(FILTER_COLOR_VALUE(filter->accept)));
  printf(" reject=%s", FILTER_COLOR_IGNORE(filter->reject) ? "*" : rgb_str(FILTER_COLOR_VALUE(filter->reject)));
  fputs(trailer, stdout);
}


static void filter_thresh_apply(filter_t *filter_, frame_hdr_t *frame)
{
  filter_thresh_t *filter = FILTER_THRESH(filter_);
  color_rgb_t color, accept, reject;
  frame_rgb_t *rgb = &(frame->fb->rgb);
#if 0
  unsigned char *py = rgb->buf;
  int xi, yi;
#else
  unsigned int size = rgb->width * rgb->height;
  unsigned char *p = rgb->buf;
  unsigned int i;
#endif

  /* Prepare RGB color values */
  color_set_rgb(filter->color_value, color);
  color_set_rgb(FILTER_COLOR_VALUE(filter->accept), accept);
  color_set_rgb(FILTER_COLOR_VALUE(filter->reject), reject);

#if 0
  for (yi = 0; yi < rgb->height; yi++) {
    unsigned char *px = py;

    for (xi = 0; xi < rgb->width; xi++) {
      /* If pixel color is selected, process it according to the reject flag */
      if ( fuzz_check(&filter->fuzz, px, color) ) {
	if ( ! FILTER_COLOR_IGNORE(filter->accept) ) {
	  px[0] = accept[0];
	  px[1] = accept[1];
	  px[2] = accept[2];
	}
      }
      else {
	if ( ! FILTER_COLOR_IGNORE(filter->reject) ) {
	  px[0] = reject[0];
	  px[1] = reject[1];
	  px[2] = reject[2];
	}
      }

      px += rgb->bpp;
    }

    py += rgb->rowstride;
  }
#else
  for (i = 0; i < size; i++) {
    /* If pixel color is selected, process it according to the reject flag */
    if ( fuzz_check(&filter->fuzz, p, color) ) {
      if ( ! FILTER_COLOR_IGNORE(filter->accept) ) {
	p[0] = accept[0];
	p[1] = accept[1];
	p[2] = accept[2];
      }
    }
    else {
      if ( ! FILTER_COLOR_IGNORE(filter->reject) ) {
	p[0] = reject[0];
	p[1] = reject[1];
	p[2] = reject[2];
      }
    }

    p += rgb->bpp;
  }
#endif

  filter_applied(FILTER(filter));
}


static filter_class_t filter_thresh_class = {
  id: "thresh",
  alloc: filter_thresh_alloc,
  destroy: NULL,
  show: filter_thresh_show,
  apply: filter_thresh_apply,
};


void filter_thresh_init(void)
{
  filter_register(&filter_thresh_class);
}


void filter_thresh_done(void)
{
}
