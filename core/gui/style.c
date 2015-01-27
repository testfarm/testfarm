/****************************************************************************/
/* TestFarm                                                                 */
/* Test Suite User Interface: style setup utilities                         */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 07-MAR-2005                                                    */
/****************************************************************************/

/*
 * $Revision: 42 $
 * $Date: 2006-06-03 15:30:01 +0200 (sam., 03 juin 2006) $
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
