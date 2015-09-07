/****************************************************************************/
/* TestFarm                                                                 */
/* TCP/IP server wrapper - Test client                                      */
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

#include "useful.h"
#include "tcp.h"


int main(int argc, char *argv[])
{
  struct hostent *hp;
  int port;
  int sock;
  struct sockaddr_in iremote;
  char buf[BUFSIZ];

  /* Check arguments */
  if ( argc != 3 ) {
    fprintf(stderr, "Usage: tcpclient host port\n");
    exit(EXIT_FAILURE);
  }

  /* Retrieve server address */
  if ( (hp = gethostbyname(argv[1])) == NULL ) {
    fprintf(stderr, "Unknown host name: %s\n", argv[1]);
    exit(EXIT_FAILURE);
  }

  /* Retrieve port number */
  if ( (port = atoi(argv[2])) <= 0 ) {
    fprintf(stderr, "Illegal port number: %s\n", argv[2]);
    exit(EXIT_FAILURE);
  }

  /* Create network socket */
  if ( (sock = socket(AF_INET, SOCK_STREAM, 0)) == -1 ) {
    perror("socket");
    exit(EXIT_FAILURE);
  }

  /* Connect to server */
  iremote.sin_family = AF_INET;
  memcpy(&iremote.sin_addr.s_addr, hp->h_addr, hp->h_length);
  iremote.sin_port = htons(port);

  if ( connect(sock, (struct sockaddr *) &iremote, sizeof(iremote)) == -1 ) {
    perror("connect");
    close(sock);
    exit(EXIT_FAILURE);
  }

  fprintf(stderr, "Connected to %s\n", tcpaddr(NULL, &iremote));

  /* Talk with server */
  *buf = '\0';
  while ( *buf != '\n' ) {
    printf("REQUEST: ");
    fflush(stdout);
    fgets(buf, sizeof(buf)-1, stdin);

    write(sock, buf, strlen(buf));
    if ( strread(sock, buf, sizeof(buf)) > 0 )
      printf("REPLY: %s", buf);
    else
      *buf = '\n';
  }

  /* Shut connection down */
  shutdown(sock, 2);

  return 0;
}
