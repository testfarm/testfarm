/****************************************************************************/
/* TestFarm                                                                 */
/* Test Suite User Interface: style setup utilities                         */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 07-MAR-2005                                                    */
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

#ifndef __STYLE_H__
#define __STYLE_H__

#include <gtk/gtk.h>

extern GtkStyle *style_white_fixed;
extern GtkStyle *style_blue_fixed;
extern GtkStyle *style_red_fixed;

extern void style_init(void);
extern void style_done(void);

#endif /* __STYLE_H__ */
