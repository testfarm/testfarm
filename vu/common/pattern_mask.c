/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* Image pattern Mask management                                            */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 23-JUL-2007                                                    */
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
#include <malloc.h>

#include "frame_rgb.h"
#include "pattern_mask.h"


static void pattern_mask_set_n(pattern_mask_t *mask)
{
  unsigned int size = mask->width * mask->height;
  unsigned char *pmask = mask->buf;
  int i;

  mask->n = 0;

  for (i = 0; i < size; i++) {
    if ( *(pmask++) )
      (mask->n)++;
  }
}


void pattern_mask_set_rgb(pattern_mask_t *mask,
			  char *source,
			  unsigned int width, unsigned int height,
			  unsigned char *rgb)
{
  unsigned int size = width * height;
  unsigned char *p, *pmask;
  int i;

  mask->source = strdup(source);
  mask->width = width;
  mask->height = height;
  mask->buf = pmask = (unsigned char *) malloc(size);

  p = rgb;

  for (i = 0; i < size; i++) {
    *(pmask++) = (p[0] | p[1] | p[2]) ? 255:0;
    p += FRAME_RGB_BPP;
  }

  free(rgb);

  /* Update number of unmasked pixels */
  pattern_mask_set_n(mask);
}


void pattern_mask_set_alpha(pattern_mask_t *mask,
			    unsigned int width, unsigned int height,
			    unsigned char *alpha)
{
  mask->source = strdup("(alpha)");
  mask->width = width;
  mask->height = height;
  mask->buf = alpha;

  /* Update number of unmasked pixels */
  pattern_mask_set_n(mask);
}


void pattern_mask_clip(pattern_mask_t *mask, unsigned int width, unsigned int height)
{
  unsigned char *old_buf = mask->buf;
  int old_width = mask->width;
  int new_bufsize = width * height;
  int min_rowsize = (old_width < width) ? old_width : width;
  int min_height = (mask->height < height) ? mask->height : height;
  unsigned char *old_p, *new_p;
  int y;

  mask->buf = malloc(new_bufsize);
  memset(mask->buf, 0, new_bufsize);

  old_p = old_buf;
  new_p = mask->buf;
  for (y = 0; y < min_height; y++) {
    memcpy(new_p, old_p, min_rowsize);
    old_p += old_width;
    new_p += width;
  }

  free(old_buf);

  /* Update number of unmasked pixels */
  pattern_mask_set_n(mask);
}


void pattern_mask_clear(pattern_mask_t *mask)
{
  mask->width = 0;
  mask->height = 0;

  if ( mask->source != NULL ) {
    free(mask->source);
    mask->source = NULL;
  }

  if ( mask->buf != NULL ) {
    free(mask->buf);
    mask->buf = NULL;
  }

  mask->n = 0;
}


void pattern_mask_copy(pattern_mask_t *mask, pattern_mask_t *mask0)
{
  pattern_mask_clear(mask);

  mask->width = mask0->width;
  mask->height = mask0->height;
  
  if ( mask0->source != NULL )
    mask->source = strdup(mask0->source);

  if ( mask0->buf != NULL ) {
    int size = mask->width * mask->height;
    mask->buf = (unsigned char *) malloc(size);
    memcpy(mask->buf, mask0->buf, size);
  }

  mask->n = mask0->n;
}
