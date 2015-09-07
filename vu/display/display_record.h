/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* RFB Input recording                                                      */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 30-MAR-2007                                                    */
/****************************************************************************/

/*
    This file is part of TestFarm,
    the Test Automation Tool for Embedded Software.
    Please visit http://www.testfarm.org.

    TestFarm is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    TestFarm is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with TestFarm.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __TVU_DISPLAY_RECORD_H__
#define __TVU_DISPLAY_RECORD_H__

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

#endif /* __TVU_DISPLAY_RECORD_H__ */
