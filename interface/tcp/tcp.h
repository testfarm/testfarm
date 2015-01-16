/****************************************************************************/
/* TestFarm                                                                 */
/* TCP/IP connection tools                                                  */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 17-AUG-1999                                                    */
/****************************************************************************/

/*
 * $Revision: 29 $
 * $Date: 2006-06-03 10:39:29 +0200 (sam., 03 juin 2006) $
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
