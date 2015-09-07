/****************************************************************************/
/* TestFarm                                                                 */
/* Graphical User Interface : status bar                                    */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 10-MAY-2000                                                    */
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

#ifndef __TESTFARM_STATUS_GUI_H
#define __TESTFARM_STATUS_GUI_H

#include <gtk/gtk.h>

#include "status.h"

extern void status_gui_init(GtkWidget *window);
extern void status_gui_done(void);

#endif /* __TESTFARM_STATUS_GUI_H */
