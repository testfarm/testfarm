/****************************************************************************/
/* TestFarm                                                                 */
/* Test Suite User Interface: command options                               */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 25-SEP-2002                                                    */
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

#ifndef __TESTFARM_GUI_OPTIONS_H__
#define __TESTFARM_GUI_OPTIONS_H__

extern int opt_noflags;
extern int opt_go;
extern int opt_quit;
extern char *opt_target;
extern unsigned long opt_depth;
extern char *opt_operator;
extern char *opt_release;
extern int opt_command;
extern int opt_service;
extern int opt_nogui;

extern int opt_get(int argc, char *argv[]);
extern void opt_usage(int summary);

#endif /* __TESTFARM_GUI_OPTIONS_H__ */
