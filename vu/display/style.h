/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* Remote Frame Buffer display - Colors, Fonts and Styles                   */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 06-JAN-2004                                                    */
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

#ifndef __TVU_DISPLAY_STYLE_H__
#define __TVU_DISPLAY_STYLE_H__

#include <gtk/gtk.h>

extern GtkStyle *style_normal;
extern GtkStyle *style_slanted;

extern GdkColor black;
extern GdkColor gray;
extern GdkColor white;
extern GdkColor red;
extern GdkColor green;
extern GdkColor blue;
extern GdkColor yellow;

extern void style_init(void);
extern void style_done(void);

#endif /* __TVU_DISPLAY_STYLE_H__ */
