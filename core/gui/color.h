/****************************************************************************/
/* TestFarm                                                                 */
/* Test Suite User Interface: some standard colors                          */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 07-DEC-1999                                                    */
/****************************************************************************/

/*
 * $Revision: 42 $
 * $Date: 2006-06-03 15:30:01 +0200 (sam., 03 juin 2006) $
 */

#ifndef __COLOR_H__
#define __COLOR_H__

#include <gdk/gdk.h>

extern GdkColor color_black;
extern GdkColor color_white;
extern GdkColor color_gray;
extern GdkColor color_red;
extern GdkColor color_yellow;
extern GdkColor color_green;
extern GdkColor color_blue;

extern void color_init(void);
extern void color_done(void);

#endif /* __COLOR_H__ */
