/****************************************************************************/
/* TestFarm RFB Interface                                                   */
/* Remote Frame Buffer display - Colors, Fonts and Styles                   */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 06-JAN-2004                                                    */
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

#include "style.h"

static GdkFont *font_normal = NULL;
static GdkFont *font_slanted = NULL;

GdkColor black;
GdkColor gray;
GdkColor white;
GdkColor red;
GdkColor green;
GdkColor blue;
GdkColor yellow;

GtkStyle *style_normal = NULL;
GtkStyle *style_slanted = NULL;


static void color_init(GdkColor *color, int red, int green, int blue)
{
  GdkColormap *colormap = gdk_colormap_get_system();

  color->red = red;
  color->green = green;
  color->blue = blue;

  if ( ! gdk_colormap_alloc_color(colormap, color, FALSE, TRUE) )
    fprintf(stderr, "RFB display: Cannot allocate color (R=%d, G=%d, B=%d)\n", red, green, blue);
}


static GtkStyle *style_alloc(GdkColor *fg, GdkColor *bg, GdkFont *font)
{
  GtkStyle *style = gtk_style_new();
  style->fg[GTK_STATE_NORMAL] = *fg;
  style->base[GTK_STATE_NORMAL] = *bg;
#if ! GTK_CHECK_VERSION(2,0,0)
  gdk_font_unref(style->font);
  style->font = font;
#endif

  return style;
}


void style_init(void)
{
#if ! GTK_CHECK_VERSION(2,0,0)
  if ( font_normal == NULL )
    font_normal = gdk_font_load("-*-lucida-medium-r-*-*-*-120-*-*-*-*-*-*");
  if ( font_slanted == NULL )
    font_slanted = gdk_font_load("-*-lucida-medium-i-*-*-*-120-*-*-*-*-*-*");
#endif

  color_init(&black, 0, 0, 0);
  color_init(&gray, 0xD600, 0xD600, 0xD600);
  color_init(&white, 65535, 65535, 65535);
  color_init(&yellow, 65535, 65535, 0);
  color_init(&red, 65535, 0, 0);
  color_init(&green, 0, 65535, 0);
  color_init(&blue, 0, 0, 65535);

  if ( style_normal == NULL )
    style_normal = style_alloc(&black, &white, font_normal);
  if ( style_slanted == NULL )
    style_slanted = style_alloc(&black, &white, font_slanted);

#if GTK_CHECK_VERSION(2,0,0)
  pango_font_description_set_style(style_slanted->font_desc, PANGO_STYLE_ITALIC);
#endif
}


void style_done(void)
{
}
