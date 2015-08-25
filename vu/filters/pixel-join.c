/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* Pixel consolidation Filter Agent                                         */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 02-JAN-2008                                                    */
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
#include <glib.h>

#include "filter_agent.h"


const char *filter_agent_name = "pixel-join";
const char *filter_agent_desc = "Pixel join filter - (c) TestFarm.org 2008";

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

  filter_agent_register("join");
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


static inline void pixel_join_axis(unsigned char *p1, unsigned char *p2,
				   unsigned char *bg,
				   unsigned short acc[4])
{
  if ( pixel_equal(p1, bg) )
    return;
  if ( pixel_equal(p2, bg) )
    return;

  acc[0] += ((unsigned short) p1[0]) + ((unsigned short) p2[0]);
  acc[1] += ((unsigned short) p1[1]) + ((unsigned short) p2[1]);
  acc[2] += ((unsigned short) p1[2]) + ((unsigned short) p2[2]);
  acc[3]++;
}


static void pixel_join(filter_t *filter, frame_buf_t *fb)
{
  int xmax = fb->rgb.width - 1;
  int ymax = fb->rgb.height - 1;
  unsigned long count;

  do {
    unsigned char *p11 = fb->rgb.buf;
    int xi, yi;

    count = 0;

    for (yi = 0; yi <= ymax; yi++) {
      for (xi = 0; xi <= xmax; xi++) {
	if ( pixel_equal(p11, filter->bg) ) {
	  unsigned char *p00, *p10, *p20;
	  unsigned char *p01,       *p21;
	  unsigned char *p02, *p12, *p22;
	  unsigned short acc[4] = {0,0,0,0};

	  if ( (xi > 0) && (xi < xmax) ) {
	    /* XX */
	    p01 = p11 - fb->rgb.bpp;
	    p21 = p11 + fb->rgb.bpp;
	    pixel_join_axis(p01, p21, filter->bg, acc);
	  }

	  if ( (yi > 0) && (yi < ymax) ) {
	    /* YY */
	    p10 = p11 - fb->rgb.rowstride;
	    p12 = p11 + fb->rgb.rowstride;
	    pixel_join_axis(p10, p12, filter->bg, acc);

	    if ( (xi > 0) && (xi < xmax) ) {
	      /* XY */
	      p00 = p10 - fb->rgb.bpp;
	      p22 = p12 + fb->rgb.bpp;
	      pixel_join_axis(p00, p22, filter->bg, acc);

	      /* YX */
	      p02 = p12 - fb->rgb.bpp;
	      p20 = p10 + fb->rgb.bpp;
	      pixel_join_axis(p02, p20, filter->bg, acc);
	    }
	  }

	  if ( acc[3] ) {
	    count++;
	    acc[3] *= 2;
	    p11[0] = acc[0] / acc[3];
	    p11[1] = acc[1] / acc[3];
	    p11[2] = acc[2] / acc[3];
	  }
	}

	p11 += fb->rgb.bpp;
      }
    }
  } while ( count );
}


void filter_agent_apply(unsigned long num, frame_buf_t *fb)
{
  filter_t *filter = g_hash_table_lookup(filters, GINT_TO_POINTER(num));

  if ( filter != NULL ) {
    pixel_join(filter, fb);
  }
  else {
    fprintf(stderr, "[ERROR] APPLY %lu: Unknown filter\n", num);
  }

  filter_agent_applied(num);
}
