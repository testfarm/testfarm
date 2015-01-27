/****************************************************************************/
/* TestFarm                                                                 */
/* Test Suite User Interface : Pixmaps database                             */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 12-APR-2000                                                    */
/****************************************************************************/

/*
 * $Revision: 770 $
 * $Date: 2007-10-09 14:34:18 +0200 (mar., 09 oct. 2007) $
 */

#ifndef __XPM_GUI_H__
#define __XPM_GUI_H__

#include <gtk/gtk.h>

extern GdkPixbuf *pixbuf_logo;

extern GdkPixbuf *pixbuf_blank;
extern GdkPixbuf *pixbuf_info;
extern GdkPixbuf *pixbuf_warning;
extern GdkPixbuf *pixbuf_error;
extern GdkPixbuf *pixbuf_panic;

extern GdkPixbuf *pixbuf_tree;
extern GdkPixbuf *pixbuf_seq;
extern GdkPixbuf *pixbuf_case;
extern GdkPixbuf *pixbuf_break;
extern GdkPixbuf *pixbuf_abort;

extern GdkPixbuf *pixbuf_passed;
extern GdkPixbuf *pixbuf_failed;
extern GdkPixbuf *pixbuf_inconclusive;
extern GdkPixbuf *pixbuf_skip;

extern GdkPixbuf *pixbuf_breakpoint;

extern GdkPixbuf *pixbuf_up;
extern GdkPixbuf *pixbuf_down;

extern GdkPixbuf *pixbuf_modified;
extern GdkPixbuf *pixbuf_updated;

extern void xpm_gui_init(void);
extern void xpm_gui_done(void);

#endif /* __XPM_GUI_H__ */
