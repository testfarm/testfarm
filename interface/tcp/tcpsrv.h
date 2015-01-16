/****************************************************************************/
/* TestFarm                                                                 */
/* Graphical User Interface : Service Socket                                */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 29-AUG-2007                                                    */
/****************************************************************************/

/*
 * $Revision: 730 $
 * $Date: 2007-09-22 10:20:59 +0200 (sam., 22 sept. 2007) $
 */

#ifndef __TCPSRV_H__
#define __TCPSRV_H__

#include <netinet/in.h>
#include <glib.h>


typedef enum {
  SRV_PROTO_TCP=0,
  SRV_PROTO_UDP,
  SRV_PROTO_N
} srv_proto_t;

typedef enum {
  SRV_IO_CONNECT=0,
  SRV_IO_DATA,
  SRV_IO_HUP
} srv_io_t;

typedef void srv_func_t(gpointer user_data, srv_io_t io, char *str);

typedef struct {
  int fd;
  GIOChannel *channel;
  guint tag;
} srv_sock_t;

typedef struct {
  srv_proto_t proto;
  srv_sock_t csock;
  srv_sock_t dsock;
  struct sockaddr_in iremote;
  srv_func_t *func;
  gpointer user_data;
} srv_t;

extern void srv_init(srv_t *srv);
extern int srv_listen(srv_t *srv, int port, srv_proto_t proto,
		      srv_func_t func, gpointer user_data);
extern int srv_write(srv_t *srv, char *str);
extern void srv_shutdown(srv_t *srv);

#endif /* __TCPSRV_H__ */
