/****************************************************************************/
/* TestFarm                                                                 */
/* Test Suite User Interface : Pixmaps database                             */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 12-APR-2000                                                    */
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
