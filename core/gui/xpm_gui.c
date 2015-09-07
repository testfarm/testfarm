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

#include <stdio.h>
#include <gtk/gtk.h>

#include "support.h"


/*===========================================*/
/*    Built-in pixmaps                       */
/*===========================================*/

static char *xpm_blank_data[] = {
  "16 16 1 1",
  "  c None",
  "                ",
  "                ",
  "                ",
  "                ",
  "                ",
  "                ",
  "                ",
  "                ",
  "                ",
  "                ",
  "                ",
  "                ",
  "                ",
  "                ",
  "                ",
  "                ",
};


/*===========================================*/
/*    Pixmap pointers                        */
/*===========================================*/

GdkPixbuf *pixbuf_logo = NULL;

GdkPixbuf *pixbuf_blank = NULL;
GdkPixbuf *pixbuf_info = NULL;
GdkPixbuf *pixbuf_warning = NULL;
GdkPixbuf *pixbuf_error = NULL;
GdkPixbuf *pixbuf_panic = NULL;

GdkPixbuf *pixbuf_tree = NULL;
GdkPixbuf *pixbuf_seq = NULL;
GdkPixbuf *pixbuf_case = NULL;
GdkPixbuf *pixbuf_break = NULL;
GdkPixbuf *pixbuf_abort = NULL;

GdkPixbuf *pixbuf_passed = NULL;
GdkPixbuf *pixbuf_failed = NULL;
GdkPixbuf *pixbuf_inconclusive = NULL;
GdkPixbuf *pixbuf_skip = NULL;

GdkPixbuf *pixbuf_breakpoint = NULL;

GdkPixbuf *pixbuf_up = NULL;
GdkPixbuf *pixbuf_down = NULL;

GdkPixbuf *pixbuf_modified = NULL;
GdkPixbuf *pixbuf_updated = NULL;


void xpm_gui_init(void)
{
  pixbuf_logo = create_pixbuf("testfarm-logo.png");

  pixbuf_blank = gdk_pixbuf_new_from_xpm_data((const char **) xpm_blank_data);

  pixbuf_info = create_pixbuf("info.png");
  pixbuf_warning = create_pixbuf("warning.png");
  pixbuf_error = create_pixbuf("error.png");
  pixbuf_panic = create_pixbuf("panic.png");

  pixbuf_tree = create_pixbuf("node_tree.png");
  pixbuf_seq = create_pixbuf("node_seq.png");
  pixbuf_case = create_pixbuf("node_case.png");
  pixbuf_break = create_pixbuf("break.png");
  pixbuf_abort = create_pixbuf("abort.png");

  pixbuf_up = create_pixbuf("start.png");
  pixbuf_down = create_pixbuf("stop.png");
  pixbuf_breakpoint = create_pixbuf("breakpoint.png");

  pixbuf_passed = create_pixbuf("passed.png");
  pixbuf_failed = create_pixbuf("failed.png");
  pixbuf_inconclusive = create_pixbuf("inconclusive.png");
  pixbuf_skip = create_pixbuf("skip.png");

  pixbuf_modified = create_pixbuf("modified.png");
  pixbuf_updated = create_pixbuf("updated.png");
}


void xpm_gui_done(void)
{
  /* Nothing */
}
