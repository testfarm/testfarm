/****************************************************************************/
/* TestFarm                                                                 */
/* TCP/IP server connection                                                 */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 17-AUG-1999                                                    */
/****************************************************************************/

/*
 * $Revision: 29 $
 * $Date: 2006-06-03 10:39:29 +0200 (sam., 03 juin 2006) $
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
