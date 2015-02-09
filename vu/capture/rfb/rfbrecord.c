/****************************************************************************/
/* TestFarm VNC Interface                                                   */
/* RFB recorder and proxy server                                            */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 25-NOV-2003                                                    */
/****************************************************************************/

/*
 * $Revision: 489 $
 * $Date: 2007-04-26 13:48:42 +0200 (jeu., 26 avril 2007) $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "chklicense.h"
#include "tcp.h"
#include "child.h"
#include "rfbproto.h"
#include "rfblib.h"
#include "record.h"


#define NAME "RFBrecord"

#define DEFAULT_HOST   "127.0.0.1"
#define DEFAULT_PORT   RFB_PORT
#define DEFAULT_VIEWER "vncviewer"


/*******************************************************/
/* RFB primitives recording                            */
/*******************************************************/

typedef void record_handler_t(void *);

static unsigned char record_prev_buttons = 0;


static unsigned char record_buf[sizeof(rfbClientToServerMsg)];
static unsigned int record_idx = 0;


static void record_event_key(rfbKeyEventMsg *ke)
{
  record_key(CARD32_TO_ULONG(ke->key), CARD8_TO_UCHAR(ke->down));
}


static void record_event_pointer(rfbPointerEventMsg *pe)
{
  unsigned char buttons = CARD8_TO_UCHAR(pe->buttonMask);
  unsigned short x, y;

  x = CARD16_TO_USHORT(pe->x);
  y = CARD16_TO_USHORT(pe->y);

  if ( buttons != record_prev_buttons ) {
    record_delay();
    record_position(x, y);
    record_buttons(buttons);
    record_prev_buttons = buttons;
  }
}


static void record(char *buf, int size)
{
  record_date();

  while ( size > 0 ) {
    unsigned char msgtype;
    int msgsize = 0;
    record_handler_t *handler = NULL;

    if ( record_idx == 0 ) {
      record_buf[record_idx++] = buf[0];
      buf++;
      size--;
    }

    msgtype = CARD8_TO_UCHAR(((rfbClientToServerMsg *) record_buf)->type);

    switch ( msgtype ) {
    case rfbSetPixelFormat:
      msgsize = sz_rfbSetPixelFormatMsg;
      break;
    case rfbFixColourMapEntries:
      msgsize = sz_rfbFixColourMapEntriesMsg;
      break;
    case rfbSetEncodings:
      msgsize = sz_rfbSetEncodingsMsg;
      break;
    case rfbFramebufferUpdateRequest:
      msgsize = sz_rfbFramebufferUpdateRequestMsg;
      break;
    case rfbKeyEvent:
      msgsize = sz_rfbKeyEventMsg;
      handler = (record_handler_t *) record_event_key;
      break;
    case rfbPointerEvent:
      msgsize = sz_rfbPointerEventMsg;
      handler = (record_handler_t *) record_event_pointer;
      break;
    case rfbClientCutText:
      msgsize = sz_rfbClientCutTextMsg;
      break;
    default:
      /* Synchronization fault => flush buffer */
      msgsize = 0;
      size = 0;
      record_idx = 0;
    }

    if ( msgsize > 0 ) {
      /* Fill message buffer */
      int count = msgsize - record_idx;
      if ( count > size )
        count = size;
      memcpy(record_buf + record_idx, buf, count);
      record_idx += count;
      buf += count;
      size -= count;

      /* Process message if complete */
      if ( record_idx == msgsize ) {
        if ( handler != NULL )
          handler(record_buf);
        record_idx = 0;
      }
    }
  }
}


/*******************************************************/
/* RFB server connection                               */
/*******************************************************/

static struct sockaddr_in server_sin;
static int server_sock = -1;

static int server_init(char *host, int port)
{
  struct hostent *hp;

  /* Retrieve server address */
  if ( (hp = gethostbyname(host)) == NULL ) {
    fprintf(stderr, NAME ": Unknown server host name: %s\n", host);
    return -1;
  }

  /* Setup server connection settings */
  server_sin.sin_family = AF_INET;
  memcpy(&server_sin.sin_addr.s_addr, hp->h_addr, hp->h_length);
  server_sin.sin_port = htons(port);

  server_sock = -1;

  return 0;
}


static void server_terminate(void)
{
  if ( server_sock > 0 ) {
    close(server_sock);
    server_sock = -1;
  }
}


static int server_connect(void)
{
  if ( server_sock > 0 ) {
    fprintf(stderr, NAME ": *PANIC*: Server already connected\n");
    return -1;
  }

  /* Create server connection socket */
  server_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
  if ( server_sock < 0 ) {
    fprintf(stderr, NAME ": Unable to create server socket: %s\n", strerror(errno));
    return -1;
  }

  if ( connect(server_sock, (struct sockaddr *) &server_sin, sizeof(server_sin)) == -1 ) {
    fprintf(stderr, NAME ": Unable to connect to server: %s\n", strerror(errno));
    return -1;
  }

  fprintf(stderr, NAME ": Connected to RFB server %s\n", tcpaddr(NULL, &server_sin));

  return 0;
}


/*******************************************************/
/* Proxy server connection                             */
/*******************************************************/

static int proxy_port = -1;
static int proxy_sockbind = -1;
static int proxy_sock = -1;
static unsigned long proxy_tx = 0;
static unsigned long proxy_tx_k = 0;
static unsigned long proxy_rx = 0;
static unsigned long proxy_rx_k = 0;

static int proxy_init(int port)
{
  struct sockaddr_in sin;
  int sockbind = -1;
  int on = 1;
  socklen_t len;

  if ( port < 0 )
    return -1;

  proxy_port = -1;
  proxy_sock = -1;
  proxy_sockbind = -1;

  sockbind = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
  if ( sockbind < 0 ) {
    fprintf(stderr, NAME ": Unable to create proxy-server connection socket: %s\n", strerror(errno));
    return -1;
  }

  if ( setsockopt(sockbind, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) ) {
    fprintf(stderr, NAME ": Unable to setup proxy server connection socket: %s\n", strerror(errno));
    close(sockbind);
    return -1;
  }

  sin.sin_family = AF_INET;
  sin.sin_addr.s_addr = INADDR_ANY;
  sin.sin_port = port;

  if ( bind(sockbind, (struct sockaddr *) &sin, sizeof(struct sockaddr_in)) ) {
    fprintf(stderr, NAME ": Unable to bind proxy server connection socket: %s\n", strerror(errno));
    close(sockbind);
    return -1;
  }

  len = sizeof(struct sockaddr_in);
  if ( getsockname(sockbind, (struct sockaddr *) &sin, &len) ) {
    fprintf(stderr, NAME ": Unable to get proxy server connection socket settings: %s\n", strerror(errno));
    close(sockbind);
    return -1;
  }

  proxy_port = ntohs(sin.sin_port);
  proxy_sockbind = sockbind;

  return proxy_port;
}


static void proxy_terminate(void)
{
  if ( proxy_sockbind > 0 ) {
    close(proxy_sockbind);
    proxy_sockbind = -1;
  }

  if ( proxy_sock > 0 ) {
    close(proxy_sock);
    proxy_sock = -1;
  }
}


static int proxy_listen(int backlog)
{
  if ( listen(proxy_sockbind, backlog) ) {
    fprintf(stderr, NAME ": Unable to listen to proxy server connection socket: %s\n", strerror(errno));
    close(proxy_sockbind);
    return -1;
  }

  return 0;
}


static int proxy_accept(void)
{
  struct sockaddr_in sin;
  int sock;
  socklen_t size;

  /* Accept client connection */
  size = sizeof(sin);
  sock = accept(proxy_sockbind, (struct sockaddr *) &sin, &size);
  if ( sock < 0 ) {
    fprintf(stderr, NAME ": Error connecting to RFB client: %s\n", strerror(errno));
    return -1;
  }

  if ( proxy_sock > 0 ) {
    fprintf(stderr, NAME ": Rejecting redundant proxy-server connection from %s\n", tcpaddr(NULL, &sin));
    close(sock);
    return 0;
  }

  fprintf(stderr, NAME ": Accepting proxy-server connection from %s\n", tcpaddr(NULL, &sin));

  proxy_sock = sock;
  proxy_tx = proxy_rx = 0;
  proxy_tx_k = proxy_rx_k = 0;

  return server_connect();
}


static void proxy_close(void)
{
  fprintf(stderr, NAME ":   Received:%luK Transmitted:%luK\n", proxy_rx_k, proxy_tx_k);

  if ( proxy_sock > 0 )
    close(proxy_sock);
  proxy_sock = -1;
  if ( server_sock > 0 )
    close(server_sock);
  server_sock = -1;
}


static int proxy_loop(void)
{
  int ret = 0;

  while ( ret == 0 ) {
    fd_set rd;
    int max;
    int count;

    /* Setup i/o events listener */
    FD_ZERO(&rd);

    FD_SET(proxy_sockbind, &rd);
    max = proxy_sockbind;

    if ( proxy_sock > 0 ) {
      FD_SET(proxy_sock, &rd);
      if ( proxy_sock > max ) max = proxy_sock;
    }

    if ( server_sock > 0 ) {
      FD_SET(server_sock, &rd);
      if ( server_sock > max ) max = server_sock;
    }

    /* Wait for i/o event */
    count = select(max+1, &rd, NULL, NULL, NULL);

    if ( count < 0 ) {
      if ( (errno != EAGAIN) && (errno != EINTR) ) {
        perror(NAME ": select");
        ret = -1;
      }
    }
    else if ( count > 0 ) {
      /* Proxy-server connection request */
      if ( FD_ISSET(proxy_sockbind, &rd) ) {
        ret = proxy_accept();
      }
      if ( (proxy_sock > 0) && FD_ISSET(proxy_sock, &rd) ) {
        char buf[BUFSIZ];
        int size = read(proxy_sock, buf, sizeof(buf));

        if ( size > 0 ) {
          write(server_sock, buf, size);
          record(buf, size);

          proxy_rx += size;
          if ( proxy_rx >= 1024 ) {
            proxy_rx_k += (proxy_rx / 1024);
            proxy_rx %= 1024;
          }
        }
        else {
          fprintf(stderr, NAME ": Proxy-server connection shut down by client\n");

          shutdown(server_sock, 2);
          fprintf(stderr, NAME ": RFB server disconnected\n");

          proxy_close();
        }
      }
      if ( (server_sock > 0) && FD_ISSET(server_sock, &rd) ) {
        char buf[BUFSIZ];
        int size = read(server_sock, buf, sizeof(buf));

        if ( size > 0 ) {
          write(proxy_sock, buf, size);

          proxy_tx += size;
          if ( proxy_tx >= 1024 ) {
            proxy_tx_k += (proxy_tx / 1024);
            proxy_tx %= 1024;
          }
        }
        else {
          fprintf(stderr, NAME ": Server connection shut down\n");

          shutdown(proxy_sock, 2);
          fprintf(stderr, NAME ": Proxy-server client disconnected\n");

          proxy_close();
        }
      }
    }
    else {
      /* Timeout */
    }
  }

  return ret;
}


/*******************************************************/
/* Viewer                                              */
/*******************************************************/

static child_t *viewer_child = NULL;

static void viewer_terminated(int status, void *arg)
{
  exit(0);
}


static int viewer_init(char *viewer, int port)
{
  char port_s[10];
  char *argv[] = { viewer, port_s, NULL };

  snprintf(port_s, sizeof(port_s), ":%d", port);

  viewer_child = child_spawn(argv, -1, -1, -1, viewer_terminated, NULL);

  return 0;
}


static void viewer_terminate(void)
{
  if ( viewer_child != NULL ) {
    child_handler(viewer_child, NULL, NULL);
    child_terminate(viewer_child);
    viewer_child = NULL;
  }
}


/*******************************************************/
/* Program body                                        */
/*******************************************************/

static void usage(void)
{
  fprintf(stderr, "Usage: rfbrecord [-async] [-rfbport <proxy-port>] [-noviewer] [-viewer <viewer>] [<host>[:<port>]]\n");
  fprintf(stderr, "  -async: Do not record measure timing between events\n");
  fprintf(stderr, "  -rfbport <proxy-port>: TCP connection port for RFB proxy-server\n");
  fprintf(stderr, "  -noviewer: Do not launch VNC viewer\n");
  fprintf(stderr, "  -viewer <viewer>: Launch this viewer instead of default %s\n", DEFAULT_VIEWER);
  fprintf(stderr, "  <host>: Host name of RFB server (default is %s)\n", DEFAULT_HOST);
  fprintf(stderr, "  <port>: TCP port of RFB server (default is %d)\n", DEFAULT_PORT);
  exit(2);
}


static void terminate(void)
{
  viewer_terminate();
  proxy_terminate();
  server_terminate();
  child_done();
}


int main(int argc, char *argv[])
{
  int async = 0;
  char *host = DEFAULT_HOST;
  int port = DEFAULT_PORT;
  char *viewer = DEFAULT_VIEWER;
  int proxyport = 0;
  char *host_args = NULL;
  int i;
  char *s;

  /* Check license */
  if ( chklicense() ) {
    fprintf(stderr, "rfbrecord: INVALID LICENSE KEY FILE\n");
    exit(EXIT_FAILURE);
  }

  for (i = 1; i < argc; i++) {
    char *args = argv[i];

    if ( args[0] == '-' ) {
      if ( strcmp(args, "-async") == 0 ) {
        async = 1;
      }
      else if ( strcmp(args, "-rfbport") == 0 ) {
        i++;
        if ( i >= argc )
          usage();

        proxyport = atoi(argv[i]);
        if ( (proxyport <= 0) || (proxyport > 65535) ) {
          fprintf(stderr, "RFB: Illegal TCP port number for RFB proxy-server\n");
          exit(2);
        }
      }
      else if ( strcmp(args, "-noviewer") == 0 ) {
        viewer = NULL;
      }
      else if ( strcmp(args, "-viewer") == 0 ) {
        i++;
        if ( i >= argc )
          usage();

        viewer = argv[i];
        if ( access(viewer, X_OK) ) {
          fprintf(stderr, "RFB: Cannot launch VNC viewer: %s\n", strerror(errno));
          exit(1);
        }
      }
      else {
        usage();
      }
    }
    else {
      if ( host_args != NULL )
        usage();
      host_args = args;
    }
  }

  /* Retrieve host:port argument */
  if ( host_args != NULL ) {
    host = host_args;
    if ( (s = strchr(host, ':')) != NULL ) {
      *(s++) = '\0';
      port = atoi(s);
      if ( (port <= 0) || (port > 65535) ) {
        fprintf(stderr, "RFB: Illegal TCP/IP port number for RFB server\n");
        exit(2);
      }
    }
  }

  atexit(terminate);

  /* Init child process management */
  child_init();

  /* Setup server connection */
  if ( server_init(host, port) )
    exit(1);

  /* Setup proxy-server connection */
  proxyport = proxy_init(proxyport);
  if ( proxyport < 0 )
    exit(1);

  if ( proxy_listen(0) )
    exit(1);

  fprintf(stderr, NAME ": Listening for VNC connections on TCP port %d\n", proxyport);

  /* Init RFB recording stuffs */
  record_idx = 0;
  record_init(stdout);
  record_set_async(async);

  /* Launch VNC viewer */
  if ( viewer != NULL ) {
    if ( viewer_init(viewer, proxyport) )
      exit(1);
  }

  proxy_loop();

  terminate();

  return 0;
}
