/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* Filter Agent : Pixel Down-sampling                                       */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 15-JAN-2008                                                    */
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


const char *filter_agent_name = "pixel-ds";
const char *filter_agent_desc = "Pixel down-sampling filter - (c) TestFarm.org 2008";

typedef enum {
  FILTER_METHOD_SKIP=0,
  FILTER_METHOD_FILL,
  FILTER_METHOD_MEAN,
  FILTER_METHOD_MAX,
} filter_method_t;

typedef struct {
  int pixsize;
  filter_method_t method;
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

  filter_agent_register("ds");
  filter_agent_register("");  // Terminate the list of filter classes

  filters = g_hash_table_new_full(NULL, NULL, NULL, (GDestroyNotify) free_filter);

  return 0;
}


void filter_agent_add(unsigned long num, char *class_id, GList *options)
{
  filter_t *filter;

  filter = malloc(sizeof(*filter));

  filter->pixsize = 2;
  filter->method = FILTER_METHOD_SKIP;

  while ( options ) {
    char *str = options->data;
    char *eq = strchr(str, '=');

    if ( eq != NULL ) {
      *(eq++) = '\0';
    }

    if ( strcmp(str, "pix") == 0 ) {
      if ( eq != NULL ) {
	filter->pixsize = atoi(eq);
	if ( filter->pixsize < 1 )
	  filter->pixsize = 1;
      }
      else {
	fprintf(stderr, "[ERROR] ADD %lu: Missing value for option '%s'\n", num, str);
      }
    }
    else if ( strcmp(str, "skip") == 0 ) {
      filter->method = FILTER_METHOD_SKIP;
    }
    else if ( strcmp(str, "fill") == 0 ) {
      filter->method = FILTER_METHOD_FILL;
    }
    else if ( strcmp(str, "mean") == 0 ) {
      filter->method = FILTER_METHOD_MEAN;
    }
    else if ( strcmp(str, "max") == 0 ) {
      filter->method = FILTER_METHOD_MAX;
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
    int len;

    len = snprintf(buf, sizeof(buf), "pix=%d", filter->pixsize);
    
    switch ( filter->method ) {
    case FILTER_METHOD_SKIP:
      len += snprintf(buf+len, sizeof(buf)-len, " skip");
      break;
    case FILTER_METHOD_FILL:
      len += snprintf(buf+len, sizeof(buf)-len, " fill");
      break;
    case FILTER_METHOD_MEAN:
      len += snprintf(buf+len, sizeof(buf)-len, " mean");
      break;
    case FILTER_METHOD_MAX:
      len += snprintf(buf+len, sizeof(buf)-len, " max");
      break;
    default:
      break;
    }

    filter_agent_shown(num, buf);
  }
  else {
    fprintf(stderr, "[ERROR] SHOW %lu: Unknown filter\n", num);
    filter_agent_shown(num, "");
  }
}


static void pixel_downsample(filter_t *filter, frame_buf_t *fb)
{
  unsigned int width = fb->rgb.width / filter->pixsize;
  unsigned int height = fb->rgb.height / filter->pixsize;
  int xstride = fb->rgb.bpp * filter->pixsize;
  int ystride = xstride * fb->rgb.width;
  unsigned char *ybuf;
  int xi, yi;

  /* Downsampling with 1x1 pixel is trivial... */
  if ( filter->pixsize < 2 )
    return;

  ybuf = fb->rgb.buf;
  for (yi = 0; yi < height; yi++) {
    unsigned char *xbuf = ybuf;

    for (xi = 0; xi < width; xi++) {
      unsigned char *yybuf;
      int xx, yy;
      int i;

      if ( filter->method == FILTER_METHOD_SKIP ) {
	yybuf = xbuf;
	for (yy = 0; yy < filter->pixsize; yy++) {
	  unsigned char *xxbuf = yybuf;

	  for (xx = 0; xx < filter->pixsize; xx++) {
	    if ( (xx != 0) || (yy != 0) ) {
	      for (i = 0; i < fb->rgb.bpp; i++) {
		xxbuf[i] = 0;
	      }
	    }

	    xxbuf += fb->rgb.bpp;
	  }

	  yybuf += fb->rgb.rowstride;
	}
      }
      else {
	unsigned long pix[fb->rgb.bpp];
	int pixsize2;

	for (i = 0; i < fb->rgb.bpp; i++) {
	  pix[i] = 0;
	}

	switch ( filter->method ) {
	case FILTER_METHOD_FILL:
	  for (i = 0; i < fb->rgb.bpp; i++) {
	    pix[i] = xbuf[i];
	  }
	  break;
	case FILTER_METHOD_MEAN:
	  yybuf = xbuf;
	  for (yy = 0; yy < filter->pixsize; yy++) {
	    unsigned char *xxbuf = yybuf;

	    for (xx = 0; xx < filter->pixsize; xx++) {
	      for (i = 0; i < fb->rgb.bpp; i++) {
		pix[i] += xxbuf[i];
	      }

	      xxbuf += fb->rgb.bpp;
	    }

	    yybuf += fb->rgb.rowstride;
	  }

	  pixsize2 = filter->pixsize * filter->pixsize;
	  for (i = 0; i < fb->rgb.bpp; i++) {
	    pix[i] /= pixsize2;
	  }
	  break;

	case FILTER_METHOD_MAX:
	  yybuf = xbuf;
	  for (yy = 0; yy < filter->pixsize; yy++) {
	    unsigned char *xxbuf = yybuf;

	    for (xx = 0; xx < filter->pixsize; xx++) {
	      for (i = 0; i < fb->rgb.bpp; i++) {
		if ( xxbuf[i] > pix[i] )
		  break;
	      }

	      if ( i < fb->rgb.bpp ) {
		for (i = 0; i < fb->rgb.bpp; i++) {
		  pix[i] = xxbuf[i];
		}
	      }

	      xxbuf += fb->rgb.bpp;
	    }

	    yybuf += fb->rgb.rowstride;
	  }
	  break;

	default:
	  break;
	}

	yybuf = xbuf;
	for (yy = 0; yy < filter->pixsize; yy++) {
	  unsigned char *xxbuf = yybuf;

	  for (xx = 0; xx < filter->pixsize; xx++) {
	    for (i = 0; i < fb->rgb.bpp; i++) {
	      xxbuf[i] = pix[i];
	    }

	    xxbuf += fb->rgb.bpp;
	  }

	  yybuf += fb->rgb.rowstride;
	}
      }

      xbuf += xstride;
    }

    ybuf += ystride;
  }
}


void filter_agent_apply(unsigned long num, frame_buf_t *fb)
{
  filter_t *filter = g_hash_table_lookup(filters, GINT_TO_POINTER(num));

  if ( filter != NULL ) {
    pixel_downsample(filter, fb);
  }
  else {
    fprintf(stderr, "[ERROR] APPLY %lu: Unknown filter\n", num);
  }

  filter_agent_applied(num);
}
