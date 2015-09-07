/****************************************************************************/
/* TestFarm                                                                 */
/* TCP/IP server connection                                                 */
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
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include "tcp.h"

int tcpconnect(char *host, int port, int debug)
{
  struct hostent *hp;
  int sock;
  struct sockaddr_in iremote;

  /* Retrieve server address */
  if ( (hp = gethostbyname(host)) == NULL ) {
    fprintf(stderr, "Unknown host name: %s\n", host);
    return -1;
  }

  /* Create network socket */
  if ( (sock = socket(AF_INET, SOCK_STREAM, 0)) == -1 ) {
    perror("socket");
    return -1;
  }

  /* Connect to server */
  iremote.sin_family = AF_INET;
  memcpy(&iremote.sin_addr.s_addr, hp->h_addr, hp->h_length);
  iremote.sin_port = htons(port);

  if ( connect(sock, (struct sockaddr *) &iremote, sizeof(iremote)) == -1 ) {
    perror("connect");
    close(sock);
    return -1;
  }

  if ( debug )
    fprintf(stderr, "Connected to %s\n", tcpaddr(NULL, &iremote));

  return sock;
}
