/****************************************************************************/
/* TestFarm                                                                 */
/* Data Logger interface : TCP/IP tools                                     */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 12-AVR-2000                                                    */
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


#ifndef __INTERFACE_TCPIP_H__
#define __INTERFACE_TCPIP_H__

extern int tcpip_init(void);
extern void tcpip_done(void);

#endif /* __INTERFACE_TCPIP_H__ */
