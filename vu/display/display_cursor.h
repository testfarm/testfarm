/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* Mouse cursor display                                                     */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 03-APR-2007                                                    */
/****************************************************************************/

/*
 * $Revision: 451 $
 * $Date: 2007-04-05 11:21:29 +0200 (jeu., 05 avril 2007) $
 */

#ifndef __VU_DISPLAY_CURSOR_H__
#define __VU_DISPLAY_CURSOR_H__

#include <gtk/gtk.h>

extern void display_cursor_init(GtkWindow *window, GtkWidget *widget);
extern void display_cursor_show(int visible);

#endif /* __VU_DISPLAY_CURSOR_H__ */
