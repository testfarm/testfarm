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


const char *filter_agent_name = "pixel-dilate";
const char *filter_agent_desc = "Pixel dilate filter - (c) TestFarm.org 2008";

typedef struct {
  unsigned char bg[3];
  unsigned int map_size;
  unsigned char *map_buf;
} filter_t;


static GHashTable *filters = NULL;


static void free_filter(filter_t *filter)
{
  if ( filter->map_buf != NULL )
    free(filter->map_buf);
  memset(filter, 0, sizeof(*filter)); /* For Security & Paranoia reasons */
  free(filter);
}


int filter_agent_init(int argc, char *argv[])
{
  if ( argc > 1 ) {
    fprintf(stderr, "Usage: %s\n", filter_agent_name);
    return -1;
  }

  filter_agent_register("dilate");
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

    snprintf(buf, sizeof(buf), "bg=#%02X%02X%02X",
	     filter->bg[0], filter->bg[1], filter->bg[2]);
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

static inline void pixel_acc(unsigned char *p,
			     unsigned short acc[4])
{
  acc[0] += p[0];
  acc[1] += p[1];
  acc[2] += p[2];
  acc[3]++;
}


#define BIT(n) (1<<(n))
enum {
  M21=0,
  M20,
  M10,
  M00,
  M01,
  M02,
  M12,
  M22,
};

static void pixel_dilate(filter_t *filter, frame_buf_t *fb)
{
  int map_size = fb->rgb.width * fb->rgb.height;
  int xmax = fb->rgb.width - 1;
  int ymax = fb->rgb.height - 1;
  unsigned char *p11, *m11;
  int xi, yi;

  /* (Re)Alloc pixel map buffer */
  if ( map_size > filter->map_size ) {
    filter->map_size = map_size;

    if ( filter->map_buf != NULL )
      free(filter->map_buf);
    filter->map_buf = malloc(map_size);
  }

  memset(filter->map_buf, 0, map_size);

  /* Construct pixel propagation map */
  m11 = filter->map_buf;
  p11 = fb->rgb.buf;

  for (yi = 0; yi <= ymax; yi++) {
    for (xi = 0; xi <= xmax; xi++) {
      if ( ! pixel_equal(p11, filter->bg) ) {
	unsigned char *m00, *m10, *m20;
	unsigned char *m01,       *m21;
	unsigned char *m02, *m12, *m22;

	if ( xi > 0 ) {
	  m01 = m11 - 1;
	  *m01 |= BIT(M21);
	}

	if ( xi < xmax ) {
	  m21 = m11 + 1;
	  *m21 |= BIT(M01);
	}

	if ( yi > 0 ) {
	  /* YY */
	  m10 = m11 - fb->rgb.width;
	  *m10 |= BIT(M12);

	  if ( xi > 0 ) {
	    /* XY */
	    m00 = m10 - 1;
	    *m00 |= BIT(M22);
	  }

	  if ( xi < xmax ) {
	    m20 = m10 + 1;
	    *m20 |= BIT(M02);
	  }
	}

	if ( yi < ymax ) {
	  m12 = m11 + fb->rgb.width;
	  *m12 |= BIT(M10);

	  if ( xi > 0 ) {
	    m02 = m12 - 1;
	    *m02 |= BIT(M20);
	  }

	  if ( xi < xmax ) {
	    m22 = m12 + 1;
	    *m22 |= BIT(M00);
	  }
	}
      }

      p11 += fb->rgb.bpp;
      m11++;
    }
  }

  /* Propagate pixels */
  m11 = filter->map_buf;
  p11 = fb->rgb.buf;

  for (yi = 0; yi <= ymax; yi++) {
    for (xi = 0; xi <= xmax; xi++) {
      if ( *m11 ) {
	unsigned char *p00, *p10, *p20;
	unsigned char *p01,       *p21;
	unsigned char *p02, *p12, *p22;
	unsigned short acc[4] = {0,0,0,0};

	if ( *m11 & BIT(M01) ) {
	  p01 = p11 - fb->rgb.bpp;
	  pixel_acc(p01, acc);
	}

	if ( *m11 & BIT(M21) ) {
	  p21 = p11 + fb->rgb.bpp;
	  pixel_acc(p21, acc);
	}

	if ( *m11 & BIT(M10) ) {
	  p10 = p11 - fb->rgb.rowstride;
	  pixel_acc(p10, acc);
	}

	if ( *m11 & BIT(M12) ) {
	  p12 = p11 + fb->rgb.rowstride;
	  pixel_acc(p12, acc);
	}

	if ( *m11 & BIT(M00) ) {
	  p00 = p11 - fb->rgb.rowstride - fb->rgb.bpp;
	  pixel_acc(p00, acc);
	}

	if ( *m11 & BIT(M20) ) {
	  p20 = p11 - fb->rgb.rowstride + fb->rgb.bpp;
	  pixel_acc(p20, acc);
	}

	if ( *m11 & BIT(M02) ) {
	  p02 = p11 + fb->rgb.rowstride - fb->rgb.bpp;
	  pixel_acc(p02, acc);
	}

	if ( *m11 & BIT(M22) ) {
	  p22 = p11 + fb->rgb.rowstride + fb->rgb.bpp;
	  pixel_acc(p22, acc);
	}

	if ( acc[3] ) {
	  p11[0] = acc[0] / acc[3];
	  p11[1] = acc[1] / acc[3];
	  p11[2] = acc[2] / acc[3];
	}
      }

      p11 += fb->rgb.bpp;
      m11++;
    }
  }
}


void filter_agent_apply(unsigned long num, frame_buf_t *fb)
{
  filter_t *filter = g_hash_table_lookup(filters, GINT_TO_POINTER(num));

  if ( filter != NULL ) {
    pixel_dilate(filter, fb);
  }
  else {
    fprintf(stderr, "[ERROR] APPLY %lu: Unknown filter\n", num);
  }

  filter_agent_applied(num);
}
