/****************************************************************************/
/* TestFarm                                                                 */
/* TCP/IP address display in a human-readable form                          */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 17-AUG-1999                                                    */
/****************************************************************************/

/*
 * $Revision: 29 $
 * $Date: 2006-06-03 10:39:29 +0200 (sam., 03 juin 2006) $
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
