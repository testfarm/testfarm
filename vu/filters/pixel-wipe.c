/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* Pixel consolidation Filter Agent                                         */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 02-JAN-2008                                                    */
/****************************************************************************/

/*
 * $Revision: 1147 $
 * $Date: 2010-06-04 16:54:08 +0200 (ven., 04 juin 2010) $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <glib.h>

#include "filter_agent.h"


const char *filter_agent_name = "pixel-wipe";
const char *filter_agent_desc = "Pixel wipe filter - (c) Basil Dev 2008 - $Revision: 1147 $";

typedef struct {
  unsigned char bg[3];
} filter_t;


static GHashTable *filters = NULL;


static void free_filter(filter_t *filter)
{
  free(filter);
}


int filter_agent_init(int argc, char *argv[])
{
  if ( argc > 1 ) {
    fprintf(stderr, "Usage: %s\n", filter_agent_name);
    return -1;
  }

  filter_agent_register("wipe");
  filter_agent_register("");  // Terminate the list of filter classes

  filters = g_hash_table_new_full(NULL, NULL, NULL, (GDestroyNotify) free_filter);

  return 0;
}


void filter_agent_add(unsigned long num, char *class_id, GList *options)
{
  filter_t *filter;

  filter = malloc(sizeof(*filter));
  memset(filter, 0, sizeof(*filter));

  while ( options ) {
    char *str = options->data;
    char *eq = strchr(str, '=');

    if ( eq != NULL ) {
      *(eq++) = '\0';
    }

    if ( strcmp(str, "bg") == 0 ) {
      if ( eq != NULL ) {
	unsigned long value = strtoul(eq, NULL, 16);
	filter->bg[0] = (value >> 16) & 0xFF;
	filter->bg[1] = (value >> 8) & 0xFF;
	filter->bg[2] = (value >> 0) & 0xFF;
      }
      else {
	fprintf(stderr, "[ERROR] ADD %lu: Missing value for option '%s'\n", num, str);
      }
    }
    else {
      fprintf(stderr, "[ERROR] ADD %lu: Unknown option '%s'\n", num, str);
    }

    options = g_list_next(options);
  }

  g_hash_table_insert(filters, GINT_TO_POINTER(num), filter);

  filter_agent_added(num);
}


void filter_agent_remove(unsigned long num)
{
  g_hash_table_remove(filters, GINT_TO_POINTER(num));
  filter_agent_removed(num);
}


void filter_agent_show(unsigned long num)
{
  filter_t *filter = g_hash_table_lookup(filters, GINT_TO_POINTER(num));

  if ( filter != NULL ) {
    char buf[80];

    snprintf(buf, sizeof(buf), "bg=#%02X%02X%02X", filter->bg[0], filter->bg[1], filter->bg[2]);
    filter_agent_shown(num, buf);
  }
  else {
    fprintf(stderr, "[ERROR] SHOW %lu: Unknown filter\n", num);
    filter_agent_shown(num, "");
  }
}


static inline int pixel_equal(unsigned char *p, unsigned char *bg)
{
  return (p[0] == bg[0]) && (p[1] == bg[1]) && (p[2] == bg[2]);
}


static void pixel_wipe(filter_t *filter, frame_buf_t *fb)
{
  int xmax = fb->rgb.width - 1;
  int ymax = fb->rgb.height - 1;
  unsigned char *p11;
  int xi, yi;

  p11 = fb->rgb.buf;

  for (yi = 0; yi <= ymax; yi++) {
    for (xi = 0; xi <= xmax; xi++) {
      if ( ! pixel_equal(p11, filter->bg) ) {
	unsigned char *p00, *p10, *p20;
	unsigned char *p01,       *p21;
	unsigned char *p02, *p12, *p22;

	if ( xi > 0 ) {
	  p01 = p11 - fb->rgb.bpp;
	  if ( ! pixel_equal(p01, filter->bg) )
	    goto bailout;
	}

	if ( xi < xmax ) {
	  p21 = p11 + fb->rgb.bpp;
	  if ( ! pixel_equal(p21, filter->bg) )
	    goto bailout;
	}

	if ( yi > 0 ) {
	  /* YY */
	  p10 = p11 - fb->rgb.rowstride;
	  if ( ! pixel_equal(p10, filter->bg) )
	    goto bailout;

	  if ( xi > 0 ) {
	    /* XY */
	    p00 = p10 - fb->rgb.bpp;
	    if ( ! pixel_equal(p00, filter->bg) )
	      goto bailout;
	  }

	  if ( xi < xmax ) {
	    p20 = p10 + fb->rgb.bpp;
	    if ( ! pixel_equal(p20, filter->bg) )
	      goto bailout;
	  }
	}

	if ( yi < ymax ) {
	  p12 = p11 + fb->rgb.rowstride;
	  if ( ! pixel_equal(p12, filter->bg) )
	    goto bailout;

	  if ( xi > 0 ) {
	    p02 = p12 - fb->rgb.bpp;
	    if ( ! pixel_equal(p02, filter->bg) )
	      goto bailout;
	  }

	  if ( xi < xmax ) {
	    p22 = p12 + fb->rgb.bpp;
	    if ( ! pixel_equal(p22, filter->bg) )
	      goto bailout;
	  }
	}

	/* Pixel is alone. Wipe it out */
	p11[0] = filter->bg[0];
	p11[1] = filter->bg[1];
	p11[2] = filter->bg[2];
      }

    bailout:
      p11 += fb->rgb.bpp;
    }
  }
}


void filter_agent_apply(unsigned long num, frame_buf_t *fb)
{
  filter_t *filter = g_hash_table_lookup(filters, GINT_TO_POINTER(num));

  if ( filter != NULL ) {
    pixel_wipe(filter, fb);
  }
  else {
    fprintf(stderr, "[ERROR] APPLY %lu: Unknown filter\n", num);
  }

  filter_agent_applied(num);
}
