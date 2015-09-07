/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* Pattern management                                                       */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 23-DEC-2003                                                    */
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
#include "pattern_image.h"
#include "pattern_text.h"
#include "pattern.h"


#define eprintf(args...) fprintf(stderr, "testfarm-vu (pattern): " args)


pattern_t *pattern_alloc(char *id, frame_hdr_t *frame, frame_geometry_t *g, pattern_mode_t mode)
{
  pattern_t *pattern;

  pattern = (pattern_t *) malloc(sizeof(pattern_t));
  memset(pattern, 0, sizeof(pattern_t));

  /* Set pattern id */
  pattern->id = strdup(id);

  /* Set pointer to source frame buffer */
  //fprintf(stderr, "-- pattern_alloc id=%s frame=%s\n", id, frame ? frame->id:"(null)");
  pattern->frame = frame;

  /* Clear pattern source definition */
  pattern->source = strdup("");
  pattern->type = PATTERN_TYPE_NONE;
  memset(&pattern->d, 0, sizeof(pattern->d));

  /* Set pattern window geometry */
  if ( g != NULL ) {
    pattern->window = *g;
  }
  else {
    pattern->window.x = 0;
    pattern->window.y = 0;
    pattern->window.width = 0;
    pattern->window.height = 0;
  }

  /* Clear pattern matching conditions */
  pattern->matched = pattern->matched0 = frame_geometry_null;

  pattern->flag = 0;
  pattern->state = 0;
  pattern->mode = mode;

  return pattern;
}


int pattern_set_source(pattern_t *pattern, pattern_type_t type, char *source, char **errptr)
{
  int ret;

  switch ( type ) {
  case PATTERN_TYPE_IMAGE:
    ret = pattern_image_set(pattern, source, errptr);
    break;
  case PATTERN_TYPE_TEXT:
    ret = pattern_text_set(pattern, source, errptr);
    break;
  default:
    if ( errptr != NULL )
      *errptr = strdup("*PANIC* Illegal pattern type");
    ret = -1;
    break;
  }

  if ( ret == 0 ) {
    pattern->type = type;
  }

  return ret;
}


int pattern_set_options(pattern_t *pattern, GList *options, char **errptr)
{
  int ret;

  switch ( pattern->type ) {
  case PATTERN_TYPE_IMAGE:
    ret = pattern_image_set_options(pattern, options, errptr);
    break;
  case PATTERN_TYPE_TEXT:
    ret = pattern_text_set_options(pattern, options, errptr);
    break;
  default:
    if ( errptr != NULL )
      *errptr = strdup("*PANIC* Illegal pattern type");
    ret = -1;
    break;
  }

  return ret;
}


static void pattern_free_guts(pattern_t *pattern)
{
  switch ( pattern->type ) {
  case PATTERN_TYPE_IMAGE: pattern_image_free(pattern); break;
  case PATTERN_TYPE_TEXT:  pattern_text_free(pattern); break;
  default:                 break;
  }
  pattern->type = PATTERN_TYPE_NONE;

  if ( pattern->source != NULL )
    free(pattern->source);
  pattern->source = NULL;

  if ( pattern->id != NULL )
    free(pattern->id);
  pattern->id = NULL;
}


void pattern_copy(pattern_t *pattern, pattern_t *pattern0)
{
  /* Free allocatable elements */
  pattern_free_guts(pattern);

  memcpy(pattern, pattern0, sizeof(pattern_t));

  pattern->id = strdup(pattern0->id);

  if ( pattern0->source != NULL )
    pattern->source = strdup(pattern0->source);

  memset(&pattern->d, 0, sizeof(pattern->d));

  switch ( pattern->type ) {
  case PATTERN_TYPE_IMAGE: pattern_image_copy(pattern, pattern0); break;
  case PATTERN_TYPE_TEXT:  pattern_text_copy(pattern, pattern0); break;
  default:                 break;
  }
}


pattern_t *pattern_clone(pattern_t *pattern0)
{
  pattern_t *pattern = (pattern_t *) malloc(sizeof(pattern_t));
  memset(pattern, 0, sizeof(pattern_t));

  pattern_copy(pattern, pattern0);

  return pattern;
}


void pattern_free(pattern_t *pattern)
{
  pattern_free_guts(pattern);
  memset(pattern, 0, sizeof(pattern_t));
  free(pattern);
}


int pattern_compare(pattern_t *pattern, char *id)
{
  return strcmp(pattern->id, id);
}


int pattern_diff(pattern_t *p1, pattern_t *p2)
{
  if ( p1 == p2 )
    return 0;
  if ( p1 == NULL )
    return 1;
  if ( p2 == NULL )
    return 1;

  if (p1->type != p2->type)
    return 1;
  if ( strcmp(p1->id, p2->id) != 0 )
    return 1;
  if ( memcmp(&(p1->window), &(p2->window), sizeof(frame_geometry_t)) != 0 )
    return 1;
  if ( p1->mode != p2->mode )
    return 1;
  if ( p1->type != p2->type )
    return 1;
  if ( strcmp(p1->source, p2->source) != 0 )
    return 1;

  switch ( p1->type ) {
  case PATTERN_TYPE_IMAGE:
	  if (pattern_image_diff(p1, p2))
		  return 1;
    break;
  case PATTERN_TYPE_TEXT:
	  if (pattern_text_diff(p1, p2))
		  return 1;
    break;
  default:
    break;
  }

  return 0;
}


int pattern_set_window(pattern_t *pattern, frame_geometry_t *g)
{
  int rgb_width;
  int rgb_height;
  int x = g->x;
  int y = g->y;
  int width = g->width;
  int height = g->height;

  if ( (pattern->frame == NULL) || (pattern->frame->fb == NULL) ) {
    eprintf("Pattern %s has no frame\n", pattern->id);
    return -1;
  }

  rgb_width = pattern->frame->g0.width;
  rgb_height = pattern->frame->g0.height;

  /* Convert window position reference */
  if ( x < 0 )
    x += rgb_width;
  if ( y < 0 )
    y += rgb_height;

  /* Check window fits into display area */
  if ( (x < 0) || (x >= rgb_width) ||
       ((x + width) > rgb_width) ) {
    eprintf("Window %ux%u+%u+%u exceeds display width (%d)\n", width, height, x, y, rgb_width);
    return -1;
  }

  if ( (y < 0) || (y >= rgb_height) ||
       ((y + height) > rgb_height) ) {
    eprintf("Window %ux%u+%u+%u exceeds display height (%d)\n", width, height, x, y, rgb_height);
    return -2;
  }

  /* Set window geometry */
  pattern->window.x = x;
  pattern->window.y = y;
  pattern->window.width = width;
  pattern->window.height = height;

  return 0;
}


int pattern_str(pattern_t *pattern, char *buf, int size)
{
	int len;

	len = snprintf(buf, size, "%s", pattern->id);
	if ((pattern->frame != NULL) && (pattern->frame->id != NULL))
		len += snprintf(buf+len, size-len, " frame=%s", pattern->frame->id);
	len += snprintf(buf+len, size-len, " window=%s", frame_geometry_str(&(pattern->window)));
	len += snprintf(buf+len, size-len, " appear=%s", pattern->mode & PATTERN_MODE_APPEAR ? "yes":"no");
	len += snprintf(buf+len, size-len, " disappear=%s", pattern->mode & PATTERN_MODE_DISAPPEAR ? "yes":"no");
	len += snprintf(buf+len, size-len, " retrigger=%s", pattern->mode & PATTERN_MODE_RETRIGGER ? "yes":"no");

	switch (pattern->type) {
	case PATTERN_TYPE_IMAGE:
		len += snprintf(buf+len, size-len, " image=%s", pattern->source);
		len += pattern_image_str(pattern, buf+len, size-len);
		break;
	case PATTERN_TYPE_TEXT:
		len += snprintf(buf+len, size-len, " text=%s", pattern->source);
		len += pattern_text_str(pattern, buf+len, size-len);
		break;
	default:
		break;
	}

	return len;
}


void pattern_show(pattern_t *pattern)
{
	int size = strlen(pattern->id) + strlen(pattern->source) + 256;
	char buf[size];
	pattern_str(pattern, buf, size);
	printf("%s", buf);
}
