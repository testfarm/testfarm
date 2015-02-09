/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* Remote Frame Buffer device primitives                                    */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 25-APR-2007                                                    */
/****************************************************************************/

/*
 * $Revision: 1143 $
 * $Date: 2010-05-05 11:30:20 +0200 (mer., 05 mai 2010) $
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <byteswap.h>

#include "tcp.h"
#include "d3des.h"
#include "scroll.h"
#include "rfbproto.h"
#include "rfblib.h"


/*==========================================================================*/
/* Status & Config reporting                                                */
/*==========================================================================*/

static int rfb_DebugFlag = 0;

int rfb_Debug(int state)
{
  int flag = rfb_DebugFlag;

  if ( state >= 0 )
    rfb_DebugFlag = state;

  return flag;
}


static int rfb_Report_ProtocolVersion(rfb_t *rfb, FILE *f, char *hdr)
{
  int ret = 0;

  ret += fprintf(f, "%sRFB Protocol Version:\n", hdr);
  ret += fprintf(f, "%s  Proposed by Server: %d.%d\n", hdr, rfb->major, rfb->minor);
  ret += fprintf(f, "%s  Required by Viewer: %d.%d\n", hdr, rfbProtocolMajorVersion, rfbProtocolMinorVersion);

  return ret;
}


static int rfb_Report_ServerInfo(rfb_t *rfb, FILE *f, char *hdr)
{
  int ret = 0;

  ret += fprintf(f, "%sRFB Server:\n", hdr);
  ret += fprintf(f, "%s  TCP/IP address: %s\n", hdr, tcpaddr(NULL, &rfb->iremote));
  ret += fprintf(f, "%s  Desktop name: '%s'\n", hdr, rfb->name);
  ret += fprintf(f, "%s  Display size: width=%u height=%u\n", hdr, rfb->width, rfb->height);

  ret += fprintf(f, "%s  Pixel format:\n", hdr);
  ret += fprintf(f, "%s    Bits per Pixel: %d\n", hdr, rfb->bits_per_pixel);
  ret += fprintf(f, "%s    Depth: %d\n", hdr, rfb->depth);
  ret += fprintf(f, "%s    Big-endian: %s\n", hdr, rfb->big_endian ? "yes":"no");
  ret += fprintf(f, "%s    True color: %s\n", hdr, rfb->true_color ? "yes":"no");

  if ( rfb->true_color ) {
    ret += fprintf(f, "%s      RGB max = %u,%u,%u\n", hdr, rfb->red_max, rfb->green_max, rfb->blue_max);
    ret += fprintf(f, "%s      RGB shift = %u,%u,%u\n", hdr, rfb->red_shift, rfb->green_shift, rfb->blue_shift);
  }

  return ret;
}


int rfb_Report(rfb_t *rfb, FILE *f, char *hdr)
{
  int ret = 0;

  ret += rfb_Report_ProtocolVersion(rfb, f, hdr);
  ret += rfb_Report_ServerInfo(rfb, f, hdr);

  return ret;
}


/*==========================================================================*/
/* Initial handshaking                                                      */
/*==========================================================================*/

static rfb_t *rfb_Alloc(void)
{
  rfb_t *rfb = (rfb_t *) malloc(sizeof(rfb_t));

  rfb->host = NULL;
  rfb->port = 0;
  rfb->sock = -1;
  rfb->major = 0;
  rfb->minor = 0;

  rfb->width = 0;
  rfb->height = 0;
  rfb->name = NULL;

  rfb->bits_per_pixel = 0;
  rfb->depth = 0;
  rfb->big_endian = 0;
  rfb->true_color = 0;
  rfb->red_max = rfb->green_max = rfb->blue_max = 0;
  rfb->red_shift = rfb->green_shift = rfb->blue_shift = 0;

  rfb->pointer_x = 0;
  rfb->pointer_x = 0;
  rfb->pointer_buttons = 0;

  return rfb;
}


void rfb_Free(rfb_t *rfb)
{
  if ( rfb == NULL )
    return;

  if ( rfb->sock != -1 )
    shutdown(rfb->sock, 2);

  if ( rfb->host != NULL )
    free(rfb->host);

  if ( rfb->name != NULL )
    free(rfb->name);

  free(rfb);
}


static int rfb_Connect(rfb_t *rfb, char *host, int port)
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

  /* Set connection settings within RFB descriptor */
  rfb->host = strdup(host);
  rfb->port = port;
  memcpy(&rfb->iremote, &iremote, sizeof(struct sockaddr_in));
  rfb->sock = sock;

  return sock;
}


void rfb_Shutdown(rfb_t *rfb)
{
  if ( rfb == NULL )
    return;

  if ( rfb->sock != -1 ) {
    shutdown(rfb->sock, 2);
    rfb->sock = -1;
  }
}


static int rfb_Read(rfb_t *rfb, void *buf, int size)
{
  int count = 0;
  int ret = 0;
  int disconnected = 0;

  while ( (! disconnected) && (ret >= 0) && (size > 0) ) {
    ret = read(rfb->sock, buf+count, size);

    if ( ret == -1 ) {
      if ( errno == EAGAIN )
        ret = 0;
    }
    else if ( ret == 0 ) {
      disconnected = 1;
    }
    else if ( ret > 0 ) {
      count += ret;
      size -= ret;
    }
  }

  if ( disconnected ) {
    fprintf(stderr, "Connection closed by RFB server\n");
    count = -1;
  }
  else if ( ret == -1 ) {
    perror("RFB read");
    count = -1;
  }

  return count;
}


static int rfb_ProtocolVersion(rfb_t *rfb)
{
  rfbProtocolVersionMsg ProtocolVersion;

  /* Get server Protocol Version */
  if ( rfb_Read(rfb, ProtocolVersion, sz_rfbProtocolVersionMsg) != sz_rfbProtocolVersionMsg ) {
    fprintf(stderr, "Invalid RFB server (bad protocol version size)\n");
    return -1;
  }

  ProtocolVersion[sz_rfbProtocolVersionMsg] = '\0';
  if ( sscanf(ProtocolVersion, rfbProtocolVersionFormat, &rfb->major, &rfb->minor) != 2 ) {
    fprintf(stderr, "Invalid RFB server (bad protocol version format)\n");
    return -1;
  }

  if ( rfb_DebugFlag )
    rfb_Report_ProtocolVersion(rfb, stderr, "");

  /* Send viewer Protocol Version */
  sprintf(ProtocolVersion, rfbProtocolVersionFormat, rfbProtocolMajorVersion, rfbProtocolMinorVersion);

  write(rfb->sock, ProtocolVersion, sz_rfbProtocolVersionMsg);

  return 0;
}


static void rfb_Encrypt(unsigned char *bytes, char *passwd)
{
  unsigned char key[8];
  int i;

  /* key is simply password padded with nulls */
  for (i = 0; i < 8; i++) {
    if (i < strlen(passwd)) {
      key[i] = passwd[i];
    } else {
      key[i] = 0;
    }
  }

  ___deskey(key, EN0);

  for (i = 0; i < sz_rfbAuthenticationChallenge; i += 8) {
    ___des(bytes+i, bytes+i);
  }
}


static int rfb_Authentication(rfb_t *rfb, char *passwd)
{
  CARD32 AuthenticationScheme;
  int scheme;
  int ret = 0;

  /* Get authentication scheme from server */
  if ( rfb_Read(rfb, &AuthenticationScheme, sizeof(AuthenticationScheme)) <= 0 )
    return -1;

  scheme =  CARD32_TO_ULONG(AuthenticationScheme);

  switch ( scheme ) {
  case rfbConnFailed:
    {
      CARD32 v;
      unsigned long reason_length;
      char *reason;
      int len;

      if ( rfb_DebugFlag )
        fprintf(stderr, "Authentication Scheme: CONNECTION FAILED\n");

      if ( rfb_Read(rfb, &v, 4) == -1 )
        return -1;

      reason_length = CARD32_TO_ULONG(v);
      reason = (char *) malloc(reason_length+1);
      len = rfb_Read(rfb, reason, reason_length);

      if ( (len > 0 ) && (len <= reason_length) ) {
        reason[len] = '\0';
        if ( rfb_DebugFlag )
          fprintf(stderr, "  Reason: %s\n", reason);
      }

      free(reason);
      ret = -1;
    }
  break;

  case rfbNoAuth:
    if ( rfb_DebugFlag )
      fprintf(stderr, "Authentication Scheme: NO AUTHENTICATION\n");
    break;

  case rfbVncAuth:
    {
      rfbAuthenticationChallenge challenge;
      CARD32 v;
      unsigned long response;

      if ( rfb_DebugFlag )
        fprintf(stderr, "Authentication Scheme: VNC AUTHENTICATION\n");

      /* Check a password is provided */
      if ( passwd == NULL ) {
        fprintf(stderr, "Password required by RFB server\n");
        return -1;
      }

      /* Get server challenge */
      if ( rfb_Read(rfb, &challenge, sz_rfbAuthenticationChallenge) == -1 )
        return -1;

      if ( rfb_DebugFlag > 1 ) {
        int i;

        fprintf(stderr, "  Challenge: ");
        for (i = 0; i < sz_rfbAuthenticationChallenge; i++)
          fprintf(stderr, "%02X", challenge[i]);
        fprintf(stderr, "\n");
      }

      /* Encrypt challenge with DES using password */
      rfb_Encrypt(challenge, passwd);

      /* Send encrypted challenge to server */
      write(rfb->sock, challenge, sz_rfbAuthenticationChallenge);

      /* Get server response */
      if ( rfb_Read(rfb, &v, 4) == -1 )
        return -1;
      response = CARD32_TO_ULONG(v);

      switch ( response ) {
      case rfbVncAuthOK:
        if ( rfb_DebugFlag )
          fprintf(stderr, "  Authentication OK\n");
        break;
      case rfbVncAuthFailed:
        if ( rfb_DebugFlag )
          fprintf(stderr, "  Authentication failed\n");
        ret = -1;
        break;
      case rfbVncAuthTooMany:
        if ( rfb_DebugFlag )
          fprintf(stderr, "  Too many authentication failures occured: please wait before reconnecting\n");
        ret = -1;
        break;
      default :
        fprintf(stderr, "Unknown Authentication Response: %lu\n", response);
        ret = -1;
      }
    }
    break;

  default :
    fprintf(stderr, "Unknown Authentication Scheme: %d\n", scheme);
    ret = -1;
  }

  return ret;
}


static int rfb_ClientInit(rfb_t *rfb, int share)
{
  rfbClientInitMsg ClientInit = { share ? 1:0 };

  /* Send client init parameter */
  write(rfb->sock, &ClientInit, sz_rfbClientInitMsg);

  if ( rfb_DebugFlag )
    fprintf(stderr, "Client Init: %sshared\n", share ? "":"not ");

  return 0;
}


static int rfb_ServerInit(rfb_t *rfb)
{
  rfbServerInitMsg ServerInit;
  unsigned long namelength;
  unsigned long len;
  int ret = 0;

  /* Get server init parameters (numeric values) */
  if ( rfb_Read(rfb, &ServerInit, sz_rfbServerInitMsg) == -1 )
    return -1;

  /* Get server init parameters (desktop name string) */
  namelength = CARD32_TO_ULONG(ServerInit.nameLength);
  rfb->name = (char *) malloc(namelength+1);
  len = rfb_Read(rfb, rfb->name, namelength);
  if ( len < 0 ) {
    len = 0;
    ret = -1;
  }
  rfb->name[len] = '\0';

  /* Retrieve frame buffer dimensions */
  rfb->width = CARD16_TO_USHORT(ServerInit.framebufferWidth);
  rfb->height = CARD16_TO_USHORT(ServerInit.framebufferHeight);

  /* Retrieve pixel format settings */
  rfb->bits_per_pixel = (unsigned int) CARD8_TO_UCHAR(ServerInit.format.bitsPerPixel);
  rfb->depth = (unsigned int) CARD8_TO_UCHAR(ServerInit.format.depth);
  rfb->big_endian = CARD8_TO_UCHAR(ServerInit.format.bigEndian) ? 1:0;
  rfb->true_color = CARD8_TO_UCHAR(ServerInit.format.trueColour) ? 1:0;

  if ( rfb->true_color ) {
    rfb->red_max = CARD16_TO_USHORT(ServerInit.format.redMax);
    rfb->green_max = CARD16_TO_USHORT(ServerInit.format.greenMax);
    rfb->blue_max = CARD16_TO_USHORT(ServerInit.format.blueMax);
    rfb->red_shift = CARD8_TO_UCHAR(ServerInit.format.redShift);
    rfb->green_shift = CARD8_TO_UCHAR(ServerInit.format.greenShift);
    rfb->blue_shift = CARD8_TO_UCHAR(ServerInit.format.blueShift);
  }

  return ret;
}


rfb_t *rfb_New(char *host, int port, char *passwd, int shared)
{
  rfb_t *rfb;

  if ( rfb_DebugFlag )
	  fprintf(stderr, "RFB types size: CARD8(%zu) CARD16(%zu) CARD32(%zu)\n",
		  sizeof(CARD8), sizeof(CARD16), sizeof(CARD32));

  /* Alloc RFB descriptor */
  rfb = rfb_Alloc();
  if ( rfb == NULL )
    return NULL;

  /* Connect to RFB server */
  if ( rfb_Connect(rfb, host, port > 0 ? port : RFB_PORT) == -1 ) {
    rfb_Destroy(rfb);
    return NULL;
  }

  /* Negociate Protocol Version */
  if ( rfb_ProtocolVersion(rfb) == -1 ) {
    rfb_Destroy(rfb);
    return NULL;
  }

  /* Retrieve Authentication Scheme */
  if ( rfb_Authentication(rfb, passwd) == -1 ) {
    rfb_Destroy(rfb);
    return NULL;
  }

  /* Perform client init in non-shared mode */
  if ( rfb_ClientInit(rfb, shared) == -1 ) {
    rfb_Destroy(rfb);
    return NULL;
  }

  /* Perform server init */
  if ( rfb_ServerInit(rfb) == -1 ) {
    rfb_Destroy(rfb);
    return NULL;
  }

  if ( rfb_DebugFlag )
    rfb_Report_ServerInfo(rfb, stderr, "");

  return rfb;
}


void rfb_Destroy(rfb_t *rfb)
{
  rfb_Shutdown(rfb);
  rfb_Free(rfb);
}


/*==========================================================================*/
/* Key event                                                                */
/*==========================================================================*/

int rfb_KeyEvent(rfb_t *rfb, int down, unsigned long key)
{
  rfbKeyEventMsg KeyEvent;

  KeyEvent.type = UCHAR_TO_CARD8(rfbKeyEvent);
  KeyEvent.down = UCHAR_TO_CARD8(down ? 1:0);
  KeyEvent.pad = 0;
  KeyEvent.key = ULONG_TO_CARD32(key);

  if ( write(rfb->sock, &KeyEvent, sz_rfbKeyEventMsg) != sz_rfbKeyEventMsg )
    return -1;

  return 0;
}


/*==========================================================================*/
/* Pointer events                                                            */
/*==========================================================================*/

static int rfb_PointerEvent_send(rfb_t *rfb)
{
  rfbPointerEventMsg PointerEvent;

  PointerEvent.type = UCHAR_TO_CARD8(rfbPointerEvent);
  PointerEvent.buttonMask = UCHAR_TO_CARD8(rfb->pointer_buttons);
  PointerEvent.x = USHORT_TO_CARD16((unsigned short) rfb->pointer_x);
  PointerEvent.y = USHORT_TO_CARD16((unsigned short) rfb->pointer_y);

  if ( write(rfb->sock, &PointerEvent, sz_rfbPointerEventMsg) != sz_rfbPointerEventMsg )
    return -1;

  return 0;
}


int rfb_PointerEvent(rfb_t *rfb, unsigned char buttons, unsigned int x, unsigned int y)
{
  rfb->pointer_buttons = buttons;
  rfb->pointer_x = x;
  rfb->pointer_y = y;

  return rfb_PointerEvent_send(rfb);
}


int rfb_PointerPosition(rfb_t *rfb, unsigned int x, unsigned int y)
{
  rfb->pointer_x = x;
  rfb->pointer_y = y;

  return rfb_PointerEvent_send(rfb);
}


int rfb_PointerButtons(rfb_t *rfb, unsigned char buttons)
{
  rfb->pointer_buttons = buttons;

  return rfb_PointerEvent_send(rfb);
}


int rfb_PointerScroll(rfb_t *rfb, unsigned char direction)
{
  unsigned char mask = 0;

  switch ( direction ) {
  case SCROLL_UP:
    mask = RFB_BUTTON_UP;
    break;
  case SCROLL_DOWN:
    mask = RFB_BUTTON_DOWN;
    break;
  case SCROLL_LEFT:
    mask = RFB_BUTTON_LEFT;
    break;
  case SCROLL_RIGHT:
    mask = RFB_BUTTON_RIGHT;
    break;
  default:
    break;
  }

  if ( mask == 0 )
    return 0;

  rfb->pointer_buttons |= mask;
  if ( rfb_PointerEvent_send(rfb) )
    return -1;

  rfb->pointer_buttons &= ~mask;
  return rfb_PointerEvent_send(rfb);
}


/*==========================================================================*/
/* Pixel format                                                             */
/*==========================================================================*/

int rfb_SetPixelFormat(rfb_t *rfb)
{
  rfbSetPixelFormatMsg SetPixelFormat;

  memset(&SetPixelFormat, 0, sizeof(SetPixelFormat));
  SetPixelFormat.type = UCHAR_TO_CARD8(rfbSetPixelFormat);
  SetPixelFormat.format.bitsPerPixel = UCHAR_TO_CARD8(rfb->bits_per_pixel);
  SetPixelFormat.format.depth = UCHAR_TO_CARD8(rfb->depth);
  SetPixelFormat.format.bigEndian = UCHAR_TO_CARD8(rfb->big_endian);
  SetPixelFormat.format.trueColour = UCHAR_TO_CARD8(rfb->true_color);
  SetPixelFormat.format.redMax = USHORT_TO_CARD16(rfb->red_max);
  SetPixelFormat.format.greenMax = USHORT_TO_CARD16(rfb->green_max);
  SetPixelFormat.format.blueMax = USHORT_TO_CARD16(rfb->blue_max);
  SetPixelFormat.format.redShift = UCHAR_TO_CARD8(rfb->red_shift);
  SetPixelFormat.format.greenShift = UCHAR_TO_CARD8(rfb->green_shift);
  SetPixelFormat.format.blueShift = UCHAR_TO_CARD8(rfb->blue_shift);

  if ( write(rfb->sock, &SetPixelFormat, sz_rfbSetPixelFormatMsg) != sz_rfbSetPixelFormatMsg )
    return -1;

  return 0;
}


int rfb_SetRGBPixelFormat(rfb_t *rfb, int bits_per_color)
{
  /* Clamp saturation value to a 8-bits value */
  if ( bits_per_color > 8 )
    bits_per_color = 8;

  rfb->depth = 3 * bits_per_color;
  if ( rfb->depth > 16 )
    rfb->bits_per_pixel = 32;
  else if ( rfb->depth > 8 )
    rfb->bits_per_pixel = 16;
  else
    rfb->bits_per_pixel = 8;

#if __BYTE_ORDER == __BIG_ENDIAN
  rfb->big_endian = 1;
#else
  rfb->big_endian = 0;
#endif

  rfb->true_color = 1;
  rfb->red_max = rfb->green_max = rfb->blue_max = (1 << bits_per_color) - 1;
  rfb->red_shift = 0;
  rfb->green_shift = bits_per_color;
  rfb->blue_shift = bits_per_color * 2;

  return rfb_SetPixelFormat(rfb);
}


/*==========================================================================*/
/* Frame buffer encoding                                                    */
/*==========================================================================*/

int rfb_SetEncodings(rfb_t *rfb, int tabc, const unsigned long *tabv)
{
  int sz = sz_rfbSetEncodingsMsg + (4*tabc);
  unsigned char buf[sz];
  rfbSetEncodingsMsg *SetEncodings = (rfbSetEncodingsMsg *) (buf + 0);
  CARD32 *Encodings = (CARD32 *) (buf + sz_rfbSetEncodingsMsg);
  int i;

  SetEncodings->type = UCHAR_TO_CARD8(rfbSetEncodings);
  SetEncodings->nEncodings = USHORT_TO_CARD16(tabc);
  for (i = 0; i < tabc; i++)
    Encodings[i] = ULONG_TO_CARD32(tabv[i]);

  if ( write(rfb->sock, buf, sz) != sz )
    return -1;

  return 0;
}


/*==========================================================================*/
/* Frame buffer update request                                              */
/*==========================================================================*/

int rfb_UpdateRequest(rfb_t *rfb, unsigned char incremental,
                      unsigned int x, unsigned int y, unsigned int w, unsigned int h)
{
  rfbFramebufferUpdateRequestMsg UpdateRequest;

  UpdateRequest.type = UCHAR_TO_CARD8(rfbFramebufferUpdateRequest);
  UpdateRequest.incremental = UCHAR_TO_CARD8(incremental);
  UpdateRequest.x = USHORT_TO_CARD16((unsigned short) x);
  UpdateRequest.y = USHORT_TO_CARD16((unsigned short) y);
  UpdateRequest.w = USHORT_TO_CARD16((unsigned short) w);
  UpdateRequest.h = USHORT_TO_CARD16((unsigned short) h);

  if ( write(rfb->sock, &UpdateRequest, sz_rfbFramebufferUpdateRequestMsg) != sz_rfbFramebufferUpdateRequestMsg )
    return -1;

  return 0;
}
