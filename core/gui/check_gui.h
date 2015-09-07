/****************************************************************************/
/* Basil Dev - TestFarm                                                     */
/* Test Suite generator : Files modification checker                        */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 13-MAY-2004                                                    */
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

#ifndef __TESTFARM_CHECK_GUI_H__
#define __TESTFARM_CHECK_GUI_H__

#include <glib.h>

typedef void check_gui_spot_t(void *data, unsigned long key);
typedef void check_gui_done_t(void *data);

extern void check_gui_files(GList *list,
			    check_gui_spot_t *spot, void *spot_data,
			    check_gui_done_t *done, void *done_data);

#endif /* __TESTFARM_CHECK_GUI_H__ */
