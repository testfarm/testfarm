/****************************************************************************/
/* TestFarm                                                                 */
/* Data Logger Interface : subprocesses management                          */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 28-MAR-2000                                                    */
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

#ifndef __INTERFACE_SUB_H__
#define __INTERFACE_SUB_H__

extern int sub_init(void);
extern void sub_done(void);

#ifdef ENABLE_GLIB
extern void sub_use_glib(void);
#endif

#endif /* __INTERFACE_SUB_H__ */
