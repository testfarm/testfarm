/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* Error reporting                                                          */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 22-OCT-2001                                                    */
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

#ifndef __TVU_ERROR_H__
#define __TVU_ERROR_H__

#include "shell.h"

#define NAME    "testfarm-vu"

extern void error(char *tag, char *fmt, ...);
extern void error_default_tag(char *tag);

extern void eprintf(char *fmt, ...);

extern int check_argc(shell_t *shell, shell_argv_t *cmd_argv, char *tag,
		      int min, int max);

#endif /* __TVU_ERROR_H__ */
