/****************************************************************************/
/* TestFarm RFB Interface                                                   */
/* RFB Input recording                                                      */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 30-MAR-2007                                                    */
/****************************************************************************/

/*
 * $Revision: 793 $
 * $Date: 2007-11-06 22:44:29 +0100 (mar., 06 nov. 2007) $
 */

#ifndef __RFB_DISPLAY_RECORD_H__
#define __RFB_DISPLAY_RECORD_H__

#include <gtk/gtk.h>

#include "capture_cap.h"

extern int display_record_init(GtkWindow *window);
extern void display_record_done(void);
extern void display_record_set_ctl(int sock, capture_cap_t cap);
extern void display_record_fkey(unsigned int keyval);
extern void display_record_stop(void);

extern void display_record_pointer_position(int x, int y, unsigned long time);
extern void display_record_pointer_button(unsigned int n, int pressed, unsigned long time);
extern void display_record_pointer_scroll(unsigned char direction);
extern void display_record_key(unsigned long keyval, int pressed);

#endif /* __RFB_DISPLAY_RECORD_H__ */
