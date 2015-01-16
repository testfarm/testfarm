/****************************************************************************/
/* TestFarm                                                                 */
/* Graphical User Interface : Service Socket (TCP)                          */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 29-AUG-2007                                                    */
/****************************************************************************/

/*
 * $Revision: 730 $
 * $Date: 2007-09-22 10:20:59 +0200 (sam., 22 sept. 2007) $
 */

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <glib.h>

#include "tcp.h"
#include "tcpsrv.h"


#define NAME "Service Connection"


static void srv_sock_clear(srv_sock_t *srv_sock)
{
  srv_sock->fd = -1;
  srv_sock->channel = NULL;
  srv_sock->tag = 0;
}


static void srv_sock_shutdown(srv_sock_t *srv_sock)
{
  if ( srv_sock->tag > 0 ) {
    g_source_remove(srv_sock->tag);
    srv_sock->tag = 0;
  }

  if ( srv_sock->channel != NULL ) {
    g_io_channel_unref(srv_sock->channel);
    srv_sock->channel = NULL;
  }

  if ( srv_sock->fd >= 0 ) {
    shutdown(srv_sock->fd, 2);
    close(srv_sock->fd);
    srv_sock->fd = -1;
  }
}


static int srv_dsock_read(srv_t *srv)
{
  GIOStatus status;
  GError *error = NULL;
  gchar *buf = NULL;
  gsize size = 0;
  int ret = 0;

  status = g_io_channel_read_line(srv->dsock.channel, &buf, &size, NULL, &error);
  if ( status == G_IO_STATUS_NORMAL ) {
    if ( buf != NULL ) {
      g_strchomp(buf);

      if ( srv->func != NULL ) {
	srv->func(srv->user_data, SRV_IO_DATA, buf);
      }
      g_free(buf);
    }
  }
  else if ( status != G_IO_STATUS_AGAIN ) {
    if ( status == G_IO_STATUS_ERROR )
      fprintf(stderr, NAME ": *ERROR* read: %s\n", error->message);
     
    ret = -1;
  }

  return ret;
}


int srv_write(srv_t *srv, char *str)
{
  int len = strlen(str);
  int ret = 0;

  if ( srv->proto == SRV_PROTO_TCP ) {
    if ( srv->dsock.fd >= 0 )
      ret = write(srv->dsock.fd, str, len);
  }
  else {
    sendto(srv->csock.fd, str, len, 0,
	   (struct sockaddr *) &srv->iremote, sizeof(srv->iremote));
  }

  return ret;
}


static gboolean srv_dsock_event(GIOChannel *source, GIOCondition condition, srv_t *srv)
{
  if ( condition & G_IO_IN ) {
    if ( srv_dsock_read(srv) ) {
      condition |= G_IO_HUP;
    }
  }

  if ( condition & G_IO_HUP ) {
    srv_sock_shutdown(&srv->dsock);
    fprintf(stderr, NAME ": Connection closed\n");

    if ( srv->func != NULL ) {
      srv->func(srv->user_data, SRV_IO_HUP, NULL);
    }

    return FALSE;
  }

  return TRUE;
}


static void srv_csock_accept(srv_t *srv)
{
  int sock;
  socklen_t size;
  int flags;

  /* Accept client connection */
  size = sizeof(srv->iremote);
  sock = accept(srv->csock.fd, (struct sockaddr *) &srv->iremote, &size);
  if ( sock < 0 ) {
    fprintf(stderr, NAME ": *ERROR* accept: %s\n", strerror(errno));
    return;
  }

  if ( srv->dsock.fd > 0 ) {
    fprintf(stderr, NAME ": *WARNING* Rejecting redundant connection\n");
    close(sock);
    return;
  }

  if ( (flags = fcntl(sock, F_GETFL, 0)) == -1 ) {
    fprintf(stderr, NAME ": *ERROR* fcntl(F_GETFL): %s\n", strerror(errno));
    close(sock);
    return;
  }
  if ( fcntl(sock, F_SETFL, O_NONBLOCK | flags) == -1 ) {
    fprintf(stderr, NAME ": *ERROR* fcntl(F_SETFL): %s\n", strerror(errno));
    close(sock);
    return;
  }

  /* Prevent child processes from inheriting this socket */
  fcntl(sock, F_SETFD, FD_CLOEXEC);

  srv->dsock.fd = sock;
  srv->dsock.channel = g_io_channel_unix_new(srv->dsock.fd);
  srv->dsock.tag = g_io_add_watch(srv->dsock.channel, G_IO_IN | G_IO_HUP,
				  (GIOFunc) srv_dsock_event, srv);

  /* Signal connection */
  if ( srv->func != NULL ) {
    char *s_addr = tcpaddr(NULL, &srv->iremote);
    fprintf(stderr, NAME ": Connection established with %s\n", s_addr);
    srv->func(srv->user_data, SRV_IO_CONNECT, s_addr);
  }
}


static void srv_csock_recv(srv_t *srv)
{
  socklen_t iremote_len = sizeof(srv->iremote);
  char buf[1024];
  ssize_t size;

  size = recvfrom(srv->csock.fd, buf, sizeof(buf)-1, 0,
		  (struct sockaddr *) &srv->iremote, &iremote_len);
  if ( size > 0 ) {
    buf[size] = 0;
    if ( srv->func != NULL ) {
      srv->func(srv->user_data, SRV_IO_DATA, buf);
    }
  }
}


static gboolean srv_csock_event(GIOChannel *source, GIOCondition condition, srv_t *srv)
{
  if ( condition & G_IO_HUP ) {
    fprintf(stderr, NAME ": *WARNING* Service connection socket shut down\n");
    fprintf(stderr, NAME ": *WARNING* Remote clients won't be able to connect any more\n");
    srv_sock_shutdown(&srv->csock);
    return FALSE;
  }

  if ( condition & G_IO_IN ) {
    if ( srv->proto == SRV_PROTO_TCP )
      srv_csock_accept(srv);
    else
      srv_csock_recv(srv);
  }

  return TRUE;
}


void srv_init(srv_t *srv)
{
  srv->proto = SRV_PROTO_TCP;
  srv->func = NULL;
  srv->user_data = NULL;
  srv_sock_clear(&srv->csock);
  srv_sock_clear(&srv->dsock);
}


int srv_listen(srv_t *srv, int port, srv_proto_t proto,
	       srv_func_t func, gpointer user_data)
{
  int sock_proto;
  struct sockaddr_in ilocal;

  srv_init(srv);

  srv->proto = proto;
  sock_proto = (proto == SRV_PROTO_TCP) ? SOCK_STREAM : SOCK_DGRAM;

  /* Create network socket */
  if ( (srv->csock.fd = socket(PF_INET, sock_proto, 0)) == -1 ) {
    fprintf(stderr, NAME ": *ERROR* socket(PF_INET, SOCK_%s): %s\n",
	    (proto == SRV_PROTO_TCP) ? "STREAM":"DGRAM", strerror(errno));
    return -1;
  }

  /* Bind network socket */
  ilocal.sin_family = AF_INET;
  ilocal.sin_addr.s_addr = htonl(INADDR_ANY);
  ilocal.sin_port = htons(port);

  if ( bind(srv->csock.fd, (struct sockaddr *) &ilocal, sizeof(ilocal)) == -1 ) { 
    fprintf(stderr, NAME ": *ERROR* bind(%d): %s\n", port, strerror(errno));
    close(srv->csock.fd);
    srv->csock.fd = -1;
    return -1;
  }

  if ( proto == SRV_PROTO_TCP ) {
    /* Listen to network connection */
    if ( listen(srv->csock.fd, 0) == -1 ) {
      fprintf(stderr, NAME ": *ERROR* listen: %s\n", strerror(errno));
      close(srv->csock.fd);
      srv->csock.fd = -1;
      return -1;
    }
    fprintf(stderr, NAME ": Listening to TCP connections from port %d\n", port);
  }
  else {
    fprintf(stderr, NAME ": Listening to UDP datagrams from port %d\n", port);
  }

  /* Prevent child processes from inheriting this socket */
  fcntl(srv->csock.fd, F_SETFD, FD_CLOEXEC);

  srv->csock.channel = g_io_channel_unix_new(srv->csock.fd);
  srv->csock.tag = g_io_add_watch(srv->csock.channel, G_IO_IN | G_IO_HUP,
				  (GIOFunc) srv_csock_event, srv);


  srv->func = func;
  srv->user_data = user_data;

  return 0;
}


void srv_shutdown(srv_t *srv)
{
  srv->func = NULL;
  srv->user_data = NULL;
  srv_sock_shutdown(&srv->dsock);
  srv_sock_shutdown(&srv->csock);
}
