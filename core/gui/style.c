/****************************************************************************/
/* TestFarm                                                                 */
/* Test Suite User Interface: style setup utilities                         */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 07-MAR-2005                                                    */
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
#include <gtk/gtk.h>

#include "install.h"
#include "color.h"
#include "style.h"


GtkStyle *style_white_fixed = NULL;
GtkStyle *style_blue_fixed = NULL;
GtkStyle *style_red_fixed = NULL;


static GtkStyle *style_alloc(GdkColor *fg, GdkColor *bg, char *font)
{
  GtkStyle *style = gtk_style_new();
  style->fg[GTK_STATE_NORMAL] = *fg;
  style->base[GTK_STATE_NORMAL] = *bg;

  if ( font != NULL ) {
    pango_font_description_free(style->font_desc);
    style->font_desc = pango_font_description_from_string(font);
  }

  return style;
}


void style_init(void)
{
  char *fixed = get_font_fixed();

  style_white_fixed = style_alloc(&color_black, &color_white, fixed);
  style_blue_fixed = style_alloc(&color_blue, &color_white, fixed);
  style_red_fixed = style_alloc(&color_red, &color_white, fixed);
}


void style_done(void)
{
}
