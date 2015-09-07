/****************************************************************************/
/* TestFarm                                                                 */
/* Test Suite User Interface - Color management                             */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 07-DEC-1999                                                    */
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
#include <gdk/gdk.h>

#include "color.h"

GdkColor color_black;
GdkColor color_white;
GdkColor color_gray;
GdkColor color_red;
GdkColor color_yellow;
GdkColor color_green;
GdkColor color_blue;


static void color_alloc(GdkColor *color, int red, int green, int blue)
{
  GdkColormap *colormap;

  /* Get drawingarea colormap */
  colormap = gdk_colormap_get_system();

  /* The color */
  color->red = red;
  color->green = green;
  color->blue = blue;

  /* Allocate color */
  if ( ! gdk_colormap_alloc_color(colormap, color, FALSE, TRUE) )
    fprintf(stderr, "Cannot allocate color (R=%d, G=%d, B=%d)\n", red, green, blue);
}


void color_init(void)
{
  color_alloc(&color_black, 0, 0, 0); 
  color_alloc(&color_white, 65535, 65535, 65535); 
  color_alloc(&color_gray, 50000, 50000, 50000);
  color_alloc(&color_red, 45000, 0, 0); 
  color_alloc(&color_yellow, 63000, 52000, 27000); 
  color_alloc(&color_green, 0, 40000, 0); 
  color_alloc(&color_blue, 0, 0, 0xD000); 
}


void color_done(void)
{
  /* Nothing */
}
