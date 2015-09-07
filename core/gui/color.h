/****************************************************************************/
/* TestFarm                                                                 */
/* Test Suite User Interface: some standard colors                          */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 07-DEC-1999                                                    */
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

#ifndef __TESTFARM_COLOR_H__
#define __TESTFARM_COLOR_H__

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

#endif /* __TESTFARM_COLOR_H__ */
