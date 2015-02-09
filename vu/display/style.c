/****************************************************************************/
/* TestFarm RFB Interface                                                   */
/* Remote Frame Buffer display - Colors, Fonts and Styles                   */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 06-JAN-2004                                                    */
/****************************************************************************/

/*
 * $Revision: 56 $
 * $Date: 2006-06-03 16:35:08 +0200 (sam., 03 juin 2006) $
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
