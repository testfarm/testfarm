/****************************************************************************/
/* TestFarm                                                                 */
/* TCP/IP address display in a human-readable form                          */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 17-AUG-1999                                                    */
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

#include <stdio.h>
#include <netinet/in.h>

char *tcpaddr(char *str, struct sockaddr_in *addr)
{
  static char buf[32];

  if ( str == NULL )
    str = buf;

  sprintf(str, "%d.%d.%d.%d:%d",
	  (int) (addr->sin_addr.s_addr >>  0) & 0xFF,
	  (int) (addr->sin_addr.s_addr >>  8) & 0xFF,
	  (int) (addr->sin_addr.s_addr >> 16) & 0xFF,
	  (int) (addr->sin_addr.s_addr >> 24) & 0xFF,
	  ntohs(addr->sin_port) & 0xFFFF);

  return str;
}
