/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* Graphical Front-end - Display area selection                             */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 07-AUG-2007                                                    */
/****************************************************************************/

/*
 * $Revision: 677 $
 * $Date: 2007-08-07 13:14:35 +0200 (mar., 07 août 2007) $
 */

#ifndef __TVU_DISPLAY_SEL_H__
#define __TVU_DISPLAY_SEL_H__

#include <gtk/gtk.h>
#include "frame_geometry.h"
#include "display.h"

#define DISPLAY_SEL_BORDER 2

typedef enum {
  DISPLAY_SEL_COLOR_RED=0,
  DISPLAY_SEL_COLOR_GREEN,
  DISPLAY_SEL_COLOR_BLUE,
  DISPLAY_SEL_COLOR_MAX
} display_sel_color_t;


extern void display_sel_init(GtkDrawingArea *darea);
extern void display_sel_done(void);
extern void display_sel_clip(display_t *d);

extern void display_sel_draw(display_sel_color_t color, frame_geometry_t *sel_g);
extern void display_sel_undraw(display_t *d, frame_geometry_t *sel_g);

#endif /* __TVU_DISPLAY_SEL_H__ */
