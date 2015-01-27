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

#ifndef __STYLE_H__
#define __STYLE_H__

#include <gtk/gtk.h>

extern GtkStyle *style_white_fixed;
extern GtkStyle *style_blue_fixed;
extern GtkStyle *style_red_fixed;

extern void style_init(void);
extern void style_done(void);

#endif /* __STYLE_H__ */
