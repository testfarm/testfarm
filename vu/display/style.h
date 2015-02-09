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

#ifndef __RFB_DISPLAY_STYLE_H__
#define __RFB_DISPLAY_STYLE_H__

#include <gtk/gtk.h>

extern GtkStyle *style_normal;
extern GtkStyle *style_slanted;

extern GdkColor black;
extern GdkColor gray;
extern GdkColor white;
extern GdkColor red;
extern GdkColor green;
extern GdkColor blue;
extern GdkColor yellow;

extern void style_init(void);
extern void style_done(void);

#endif /* __RFB_DISPLAY_STYLE_H__ */
