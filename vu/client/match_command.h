/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* Pattern Matching commands                                                 */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 12-NOV-2007                                                    */
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

#ifndef __TVU_MATCH_COMMAND_H__
#define __TVU_MATCH_COMMAND_H__

#include "shell.h"
#include "frame_display.h"

extern int match_command_init(shell_t *shell, frame_display_t *display);
extern void match_command_done(shell_t *shell);

#endif /* __TVU_MATCH_COMMAND_H__ */
