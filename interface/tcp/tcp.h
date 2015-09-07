/****************************************************************************/
/* TestFarm                                                                 */
/* TCP/IP connection tools                                                  */
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

#ifndef __TCP_H
#define __TCP_H

#include <string.h>
#include <netinet/in.h>

/* tcpaddr(): builds a string containing a human-readable INET address
 *   INPUT:  str = target buffer, or NULL for using an internal static buffer.
 *           addr = INET address.
 *   OUTPUT: pointer to the target buffer.
 */
extern char *tcpaddr(char *str, struct sockaddr_in *addr);


/* tcpconnect(): create a socket and connect to a server through TCP/IP.
 *   INPUT:  host = server host name.
 *           port = server port number.
 *           debug = boolean value telling to display a connection
 *                   information message
 *   OUTPUT: file descriptor of created+connected socket.
 */
extern int tcpconnect(char *host, int port, int debug);

#endif /* __TCP_H */
