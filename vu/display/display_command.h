/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* Display Tool - command entry and history                                 */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 08-AUG-2010                                                    */
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

#ifndef __TVU_DISPLAY_COMMAND_H__
#define __TVU_DISPLAY_COMMAND_H__

#include <gtk/gtk.h>

extern void display_command_init(GtkWindow *window);
extern void display_command_done(void);

extern void display_command_connect(int sock);
extern void display_command_disconnect(void);

extern void display_command_history(int id, char *cmd);

#endif /* __TVU_DISPLAY_COMMAND_H__ */
