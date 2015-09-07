/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* RGB Color specification                                                  */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 29-AUG-2007                                                    */
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

#include "frame_rgb.h"
#include "color.h"


int color_parse(char *spec, unsigned long *pvalue)
{
  unsigned long value = 0;
  int ret = 0;

  /* #RRGGBB format */
  if ( spec[0] == '#' ) {
    char *s = spec+1;

    if ( strlen(s) == 6 )
      value = strtoul(s, NULL, 16);
    else
      ret = -1;
  }

  /* Interger format => grayscale */
  else {
    char *s = spec;
    int v;

    while ( *s != '\0' ) {
      if ( (*s < '0') || (*s > '9') )
	ret = -1;
      s++;
    }

    if ( ret == 0 ) {
      v = atoi(spec);
      if ( v < 0 )
	v = 0;
      else if ( v > 255 )
	v = 255;

      value = (v << 16) | (v << 8) | v;
    }
  }

  if ( (ret == 0) && (pvalue != NULL) )
    *pvalue = value;

  return ret;
}


void color_set_rgb(unsigned long value, color_rgb_t rgb)
{
  rgb[0] = (value >> 16) & 0xFF;
  rgb[1] = (value >> 8) & 0xFF;
  rgb[2] = value & 0xFF;
}


void color_fill(unsigned long color, unsigned int width, unsigned int height, unsigned char **pbuf)
{
  color_rgb_t rgb;
  unsigned char *buf;
  int size;
  int i;

  color_set_rgb(color, rgb);

  size = frame_rgb_bufsize(width, height);
  buf = malloc(size + FRAME_RGB_MMTAIL);
  for (i = 0; i < size; i++) {
    buf[i] = rgb[i%3];
  }

  for (i = 0; i < FRAME_RGB_MMTAIL; i++) {
    buf[size+i] = 0;
  }

  *pbuf = buf;
}


int color_fill_from_spec(char *spec, unsigned int *pwidth, unsigned int *pheight, unsigned char **pbuf)
{
  char *spec2;
  char *s_width;
  unsigned int width = 1;
  unsigned int height = 1;
  unsigned long color = 0;
  int ret = 0;

  spec2 = strdup(spec);

  s_width = strchr(spec2, ':');
  if ( s_width != NULL ) {
    char *s_height;

    *(s_width++) = '\0';
    s_height = strchr(s_width, 'x');

    if ( s_height != NULL ) {
      *(s_height++) = '\0';
      width = atoi(s_width);
      height = atoi(s_height);
    }
    else {
      width = height = atoi(s_width);
    }
  }

  if ( (width > 0) && (height > 0) ) {
    ret = color_parse(spec2, &color);
    if ( ret == 0 )
      color_fill(color, width, height, pbuf);
  }
  else {
    ret = -1;
  }

  free(spec2);

  if ( pwidth != NULL )
    *pwidth = width;
  if ( pheight != NULL )
    *pheight = height;

  return ret;
}
