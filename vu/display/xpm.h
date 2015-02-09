/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* Display Tool - Pixbufs and Icons                                         */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 08-JAN-2004                                                    */
/****************************************************************************/

/*
 * $Revision: 814 $
 * $Date: 2007-11-27 11:33:11 +0100 (mar., 27 nov. 2007) $
 */

#ifndef __TVU_DISPLAY_XPM_H__
#define __TVU_DISPLAY_XPM_H__

#include <gtk/gtk.h>

extern GdkPixbuf *pixbuf_blank;
extern GdkPixbuf *pixbuf_connect_creating;
extern GdkPixbuf *pixbuf_connect_established;
extern GdkPixbuf *pixbuf_connect_no;

extern GdkPixbuf *pixbuf_frame;

extern void xpm_init(GtkWidget *window);
extern void xpm_done(void);

#endif /* __TVU_DISPLAY_XPM_H__ */
