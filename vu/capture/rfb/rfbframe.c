/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* RFB Frame Device                                                         */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 25-APR-2007                                                    */
/****************************************************************************/

/*
 * $Revision: 1105 $
 * $Date: 2010-01-12 12:21:25 +0100 (mar., 12 janv. 2010) $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <glib.h>

#include "useful.h"
#include "log.h"

#include "rfbproto.h"
#include "rfblib.h"
#include "rfbframe.h"


//#define __DEBUG__

#ifdef __DEBUG__
#define DEBUG(args...) fprintf(stderr, "RFB DEBUG: " args)
#else
#define DEBUG(args...)
#endif

#define eprintf(args...) fprintf(stderr, "testfarm-vu (rfb): " args)

#define FRAME_DELAY_DATA 10000

#define FRAME_STATE_IDLE      0   /* No update request in progress */
#define FRAME_STATE_WAIT      1   /* Wait for frame update message */
#define FRAME_STATE_RECTANGLE 2   /* Wait for rectangle header */
#define FRAME_STATE_PIXEL     3   /* Wait for rectangle pixels */
#define FRAME_STATE_CUTTEXT   4   /* Wait for server cut text */

static void frame_set_state(rfb_capture_t *capture, int state, int watchdog);


/*=========================================================*/
/* RFB i/o                                                 */
/*=========================================================*/

static int frame_read(rfb_capture_t *capture);


static void *frame_read_alloc(rfb_capture_t *capture, int size)
{
  if ( size <= 0 ) {
    if ( capture->read_buf != NULL )
      free(capture->read_buf);
    capture->read_buf = NULL;
    capture->read_size = 0;
    capture->read_ofs = 0;
  }
  else if ( size > capture->read_size ) {
    if ( capture->read_buf != NULL )
      free(capture->read_buf);
    capture->read_buf = malloc(size);
    capture->read_size = size;
    capture->read_ofs = 0;
  }

  return capture->read_buf;
}


static gboolean frame_read_event(GIOChannel *source, GIOCondition condition, rfb_capture_t *capture)
{
  if ( condition & G_IO_IN ) {
    if ( frame_read(capture) < 0 )
      condition |= G_IO_HUP;
  }

  if ( condition & G_IO_HUP ) {
    eprintf("Connection closed by VNC server\n");
    rfb_Shutdown(capture->rfb);
    exit(0);
    return FALSE;
  }

  return TRUE;
}


static gboolean frame_read_timeout(rfb_capture_t *capture)
{
  /* Refresh timeout */
  if ( (capture->state != FRAME_STATE_IDLE) && (capture->state != FRAME_STATE_WAIT) ) {
    fprintf(stderr, "RFB: frame data receive timeout in state %d\n", capture->state);
    frame_set_state(capture, FRAME_STATE_IDLE, 0);
  }

  return FALSE;
}


/*=================================================================*/
/* Frame update                                                    */
/*=================================================================*/

#define ENCODINGS_N 2
static const unsigned long encodings[ENCODINGS_N] = { rfbEncodingCopyRect, rfbEncodingRaw };


static void frame_set_state(rfb_capture_t *capture, int state, int watchdog)
{
  capture->state = state;
  capture->read_ofs = 0;

  if ( capture->watchdog_tag > 0 )
    g_source_remove(capture->watchdog_tag);
  capture->watchdog_tag = 0;

  if ( watchdog )
    capture->watchdog_tag = g_timeout_add(FRAME_DELAY_DATA, (GSourceFunc) frame_read_timeout, capture);
}


static void frame_idle(rfb_capture_t *capture)
{
  capture->nrects = 0;
  capture->xi = 0;
  capture->yi = 0;

  frame_set_state(capture, FRAME_STATE_IDLE, 0);
}


static rfb_t *frame_connect(char *device, capture_opt_t opt)
{
  char *password = NULL;
  char *host = NULL;
  int port = RFB_PORT;
  char *s;
  rfb_t *rfb;

  /* Parse VNC server description */
  host = strchr(device, '@');
  if ( host != NULL ) {
    *(host++) = '\0';
    password = device;
  }
  else {
    host = device;
  }

  if ( (s = strchr(host, ':')) != NULL ) {
    *(s++) = NUL;
    port = strtol(s, (char **) NULL, 0);
    if ( port < 10 ) {
      port += RFB_PORT;
    }
    else if ( port > 65535 ) {
      eprintf("Illegal TCP/IP port number\n");
      return NULL;
    }
  }

  /* Open VNC server connection */
  rfb_Debug(opt & CAPTURE_OPT_DEBUG);
  rfb = rfb_New(host, port, password, opt & CAPTURE_OPT_SHARED);
  if ( rfb == NULL ) {
    eprintf("Connection to VNC server %s:%d failed\n", host, port);
    return NULL;
  }

  return rfb;
}


static void _frame_close(capture_t *_capture)
{
  rfb_capture_t *capture = RFB_CAPTURE(_capture);

  if ( capture == NULL )
    return;

  /* Stop read watchdog */
  frame_idle(capture);

  /* Stop RFB refresh timeout */
  if ( capture->rfb_timeout > 0 ) {
    g_source_remove(capture->rfb_timeout);
    capture->rfb_timeout = 0;
  }

  /* Stop RFB read events handling */
  if ( capture->rfb_tag > 0 ) {
    g_source_remove(capture->rfb_tag);
    capture->rfb_tag = 0;
  }

  if ( capture->rfb_channel != NULL ) {
    g_io_channel_unref(capture->rfb_channel);
    capture->rfb_channel = NULL;
  }

  /* Shut RFB connection down */
  if ( capture->rfb != NULL ) {
    rfb_Destroy(capture->rfb);
    capture->rfb = NULL;
  }

  /* Free RFB read buffer */
  frame_read_alloc(capture, 0);

  /* Free RFB processing stuffs */
  if ( capture->cutbuf != NULL ) {
    free(capture->cutbuf);
    capture->cutbuf = NULL;
  }

  if ( capture->rowbuf != NULL ) {
    free(capture->rowbuf);
    capture->rowbuf = NULL;
  }

  free(capture->rfb_buf);
  free(capture);
}


static capture_t *_frame_open(char *device, capture_opt_t opt,
			      capture_update_fn *update, void *data)
{
  rfb_t *rfb;
  rfb_capture_t *capture;
  int rfbsock;
  int flags;

  /* Connect to VNC server */
  rfb = frame_connect(device, opt);
  if ( rfb == NULL )
    return NULL;

  capture = (rfb_capture_t *) malloc(sizeof(rfb_capture_t));
  memset(capture, 0, sizeof(rfb_capture_t));

  /* Init frame buffer */
  capture->rfb = rfb;
  capture->rfb_buf = (unsigned char *) malloc(3 * rfb->width * rfb->height);
  capture->rfb_g = frame_geometry_null;

  /* Set active window */
  capture->window.x = 0;
  capture->window.y = 0;
  capture->window.width = rfb->width;
  capture->window.height = rfb->height;

  /* Init RFB protocol management */
  capture->delay = 0;
  capture->state = FRAME_STATE_IDLE;
  capture->watchdog_tag = 0;
  capture->rowbuf = NULL;
  capture->rowsize = 0;

  capture->cutlen = 0;
  capture->cutidx = 0;
  capture->cutbuf = NULL;
  capture->cutstate = FRAME_STATE_IDLE;

  capture->read_buf = NULL;
  capture->read_size = 0;
  capture->read_ofs = 0;

  /* Enable non-blocking read operations from RFB socket */
  rfbsock = capture->rfb->sock;
  if ( (flags = fcntl(capture->rfb->sock, F_GETFL, 0)) == -1 ) {
    eprintf("RFB: fcntl(F_GETFL): %s\n", strerror(errno));
  }
  else if ( fcntl(capture->rfb->sock, F_SETFL, O_NONBLOCK | flags) == -1 ) {
    eprintf("RFB: fcntl(F_SETFL): %s\n", strerror(errno));
  }

  /* Setup RFB read events handling */
  capture->rfb_channel = g_io_channel_unix_new(rfbsock);
  capture->rfb_tag = g_io_add_watch(capture->rfb_channel, G_IO_IN | G_IO_HUP,
				    (GIOFunc) frame_read_event, capture);

  /* Declare supported frame buffer encodings */
  rfb_SetEncodings(rfb, ENCODINGS_N, encodings);

  /* Declare RGB pixel format */
  rfb_SetRGBPixelFormat(rfb, CHAR_BIT);

  /* Allocate target frame buffer */
  capture->fb = frame_buf_alloc(capture->window.width, capture->window.height, &(capture->h.shmid));
  if ( capture->fb == NULL ) {
    eprintf("Failed to allocate frame buffer\n");
    goto failed;
  }

  /* Set name and capabilities */
  capture->h.cap |= CAPTURE_CAP_VIDEO | CAPTURE_CAP_KEY | CAPTURE_CAP_POINTER | CAPTURE_CAP_SCROLL;
  capture->h.name = capture->rfb->name;

  /* Hook frame update handler */
  capture->update = update;
  capture->data = data;

  return (capture_t *) capture;

 failed:
  _frame_close((capture_t *) capture);
  return NULL;
}


static int _frame_set_window(capture_t *capture, frame_geometry_t *g)
{
  RFB_CAPTURE(capture)->window = *g;
  return 0;
}


static int _frame_get_window(capture_t *capture, frame_geometry_t *g)
{
  *g = RFB_CAPTURE(capture)->window;
  return 0;
}


static int frame_refresh_request(rfb_capture_t *capture, int incremental)
{
  if ( (capture->state != FRAME_STATE_IDLE) && (capture->state != FRAME_STATE_WAIT) )
    return -1;

  DEBUG("Update request (incremental=%d)\n", incremental);

  rfb_UpdateRequest(capture->rfb, incremental,
		    capture->window.x, capture->window.y,
		    capture->window.width, capture->window.height);

  capture->nrects = 0;

  frame_set_state(capture, FRAME_STATE_WAIT, 0);

  return 0;
}


static void frame_refresh_process(rfb_capture_t *capture)
{
  frame_geometry_t g = capture->rfb_g;
  unsigned long rowstride = 3 * capture->rfb->width;
  unsigned long offset = (rowstride * g.y) + (3 * g.x);
  unsigned char *source = capture->rfb_buf + offset;
  unsigned char *target = capture->fb->rgb.buf + offset;
  unsigned long rowsize = 3 * g.width; 
  int yi;

  for (yi = 0; yi < g.height; yi++) {
    memcpy(target, source, rowsize);
    source += rowstride;
    target += rowstride;
  }

  DEBUG("frame_refresh_process %s\n", frame_geometry_str(&g));
  if ( capture->update )
    capture->update(&g, capture->data);

  capture->refresh_requested = 0;
  capture->rfb_g = frame_geometry_null;
}


static int frame_refresh_timeout(rfb_capture_t *capture)
{
  if ( capture->rfb_g.width > 0 ) {
    frame_refresh_process(capture);
  }
  else {
    capture->refresh_requested = 1;
  }

  /* Ensure an update request has been sent */
  if ( capture->state == FRAME_STATE_IDLE )
    frame_refresh_request(capture, 1);

  return TRUE;
}


static int _frame_refresh(capture_t *capture)
{
  RFB_CAPTURE(capture)->refresh_requested = 1;
  return frame_refresh_request(RFB_CAPTURE(capture), 0);
}


static long _frame_set_period(capture_t *_capture, long delay)
{
  rfb_capture_t *capture = RFB_CAPTURE(_capture);

  if ( delay <= 0 ) {
    capture->delay = 0;
  }
  else {
    if ( delay < CAPTURE_PERIOD_MIN )
      return -1;
    if ( delay > CAPTURE_PERIOD_MAX )
      return -1;

    capture->delay = delay;
  }

  if ( capture->rfb_timeout > 0 )
    g_source_remove(capture->rfb_timeout);
  capture->rfb_timeout = 0;

  if ( capture->delay > 0 )
    capture->rfb_timeout = g_timeout_add(capture->delay, (GSourceFunc) frame_refresh_timeout, capture);

  return capture->delay;
}


static long _frame_get_period(capture_t *capture)
{
  return RFB_CAPTURE(capture)->delay;
}


static void frame_read_update(rfb_capture_t *capture, frame_geometry_t *g)
{
  /* Update refresh area geometry */
  DEBUG("frame_read_update %s\n", frame_geometry_str(g));
  if ( capture->rfb_g.width > 0 ) {
    int x1, y1;
    int x2, y2;
    int v;

    x1 = capture->rfb_g.x;
    if ( x1 > g->x )
      x1 = g->x;

    y1 = capture->rfb_g.y;
    if ( y1 > g->y )
      y1 = g->y;

    x2 = capture->rfb_g.x + capture->rfb_g.width;
    v = g->x + g->width;
    if ( x2 < v )
      x2 = v;

    y2 = capture->rfb_g.y + capture->rfb_g.height;
    v = g->y + g->height;
    if ( y2 < v )
      y2 = v;

    capture->rfb_g.x = x1;
    capture->rfb_g.y = y1;
    capture->rfb_g.width = x2 - x1;
    capture->rfb_g.height = y2 - y1;
  }
  else {
    capture->rfb_g = *g;
  }
  DEBUG("   => %s\n", frame_geometry_str(&capture->rfb_g));

  /* Update frame now if refresh request pending */
  if ( capture->refresh_requested )
    frame_refresh_process(capture);

  /* Request for another frame update */
  frame_refresh_request(capture, 1);
}


static int frame_read_chunk(rfb_capture_t *capture, void *buf, int size)
{
  int ret;

  if ( capture->read_ofs > 0 ) {
  }

  ret = read(capture->rfb->sock, buf, size);

  if ( ret < 0 ) {
    if ( (errno == ECONNRESET) || (errno == ENETRESET) || (errno == ECONNABORTED) )
      return -2;

    eprintf("RFB: read: %s\n", strerror(errno));
    return -1;
  }

  if ( ret == 0 )
    return -2;

  if ( ret < size ) {
    frame_read_alloc(capture, size);
  }

  return ret;
}


static int frame_read_header(rfb_capture_t *capture)
{
  rfbServerToClientMsg msg;
  unsigned char msgtype;
  unsigned int len;
  int size;

  size = frame_read_chunk(capture, &msg, 1);
  if ( size < 0 )
    return size;

  msgtype = CARD8_TO_UCHAR(msg.type);

  switch ( msgtype ) {
  case rfbFramebufferUpdate:
    len = sz_rfbFramebufferUpdateMsg - 1;

    size = frame_read_chunk(capture, ((char *) &msg)+1, len);
    if ( size < 0 )
      return size;

    if ( size != len ) {
      fprintf(stderr, "RFB: received Update header with wrong length (%d/%d)\n", size, len);
      return -1;
    }

    capture->nrects = (unsigned int) CARD16_TO_USHORT(msg.fu.nRects);
    if ( capture->nrects == 0 ) {
      fprintf(stderr, "RFB: received Update header with no rectangle\n");
      return -1;
    }

    DEBUG("Update header: %u rectangles\n", capture->nrects);
    frame_set_state(capture, FRAME_STATE_RECTANGLE, 1);
    break;

  case rfbBell:
    printf("%s\n", log_hdr_("BELL"));
    frame_idle(capture);
    break;

  case rfbServerCutText:
    len = sz_rfbServerCutTextMsg - 1;
    size = frame_read_chunk(capture, ((char *) &msg)+1, len);
    if ( size < 0 )
      return size;

    if ( size != len ) {
      fprintf(stderr, "RFB: received CutText header with wrong length (%d/%d)\n", size, len);
      return -1;
    }

    capture->cutlen = CARD32_TO_ULONG(msg.sct.length);
    capture->cutidx = 0;
    capture->cutbuf = (unsigned char *) realloc(capture->cutbuf,  capture->cutlen);
    capture->cutstate = capture->state;

    DEBUG("CutText header: %lu characters\n", capture->cutlen);
    frame_set_state(capture, FRAME_STATE_CUTTEXT, 1);
    break;

  default:
    fprintf(stderr, "RFB: received unsupported server message type (%d)\n", msgtype);
    return -1;
  }

  return 0;
}


static int frame_read_rectangle(rfb_capture_t *capture)
{
  int size, len;
  char *ptr;
  unsigned long encoding;
  int not_supported;
  unsigned int w, h;
  int i;

  size = sz_rfbFramebufferUpdateRectHeader - capture->read_ofs;
  if ( size <= 0 ) {
    fprintf(stderr, "RFB: received Update rectangle in illegal buffering state (%d)\n", size);
    return -1;
  }

  ptr = ((char *) &(capture->rect)) + capture->read_ofs;
  len = frame_read_chunk(capture, ptr, size);
  if ( len < 0 )
    return len;

  /* Keep reading if RFB message is not complete */
  capture->read_ofs += len;
  if ( capture->read_ofs < sz_rfbFramebufferUpdateRectHeader )
    return 0;
  capture->read_ofs = 0;

  /* Compute pixel buffer size */
  encoding = CARD32_TO_ULONG(capture->rect.encoding);
  not_supported = 1;
  for (i = 0; i < ENCODINGS_N; i++) {
    if ( encoding == encodings[i] ) {
      not_supported = 0;
      break;
    }
  }

  if ( not_supported ) {
    fprintf(stderr, "RFB: received Update rectangle with unsupported encoding (%lu)\n", encoding);
    return -1;
  }

  /* Validate rectangle size */
  w = CARD16_TO_USHORT(capture->rect.r.w);
  h = CARD16_TO_USHORT(capture->rect.r.h);

  DEBUG("Update rectangle: %ux%u+%u+%u encoding=%lu\n",
        w, h, CARD16_TO_USHORT(capture->rect.r.x), CARD16_TO_USHORT(capture->rect.r.y), encoding);

  /* Reject rectangle if illegal size */
  if ( (w > capture->rfb->width) || (h > capture->rfb->height) ) {
    fprintf(stderr, "RFB: Received Update rectangle with illegal size %ux%u\n", w, h);
    return -1;
  }

  /* Init data buffering index */
  capture->xi = 0;
  capture->yi = 0;

  frame_set_state(capture, FRAME_STATE_PIXEL, 1);

  return 0;
}


static int frame_read_raw(rfb_capture_t *capture)
{
  frame_geometry_t g = {
         x : CARD16_TO_USHORT(capture->rect.r.x),
         y : CARD16_TO_USHORT(capture->rect.r.y),
     width : CARD16_TO_USHORT(capture->rect.r.w),
    height : CARD16_TO_USHORT(capture->rect.r.h),
  };
  rfb_t *rfb = capture->rfb;
  unsigned long bpp = CEIL_DIV(rfb->bits_per_pixel, CHAR_BIT);
  unsigned long bpw = g.width * bpp;
  unsigned long len = bpw - capture->xi;
  int size;
  int completed;

  /* Grow row buffer as needed */
  if ( capture->rowsize < len ) {
    capture->rowsize = bpw;
    capture->rowbuf = realloc(capture->rowbuf, capture->rowsize);
  }

  /* Read row fragment */
  size = frame_read_chunk(capture, capture->rowbuf + capture->xi, len);
  if ( size < 0 )
    return size;

  //DEBUG("Update raw: xi=%lu yi=%lu len=%lu read=%d\n", capture->xi, capture->yi, len, size);

  capture->xi += size;
  if ( capture->xi >= bpw ) {
    unsigned char *source = capture->rowbuf;
    unsigned char *target = capture->rfb_buf + (3 * ((rfb->width * (g.y + capture->yi)) + g.x));
    int i;

    for (i = 0; i < g.width; i++) {
      target[0] = source[0];
      target[1] = source[1];
      target[2] = source[2];
      source += bpp;
      target += 3;
    }

    capture->xi = 0;
    capture->yi++;
  }

  completed = (capture->yi >= g.height);

  /* Call frame update handler when completed */
  if ( completed )
    frame_read_update(capture, &g);

  /* Return rectangle completion result */
  return completed;
}


static int frame_read_copyrect(rfb_capture_t *capture)
{
  frame_geometry_t g = {
         x : CARD16_TO_USHORT(capture->rect.r.x),
         y : CARD16_TO_USHORT(capture->rect.r.y),
     width : CARD16_TO_USHORT(capture->rect.r.w),
    height : CARD16_TO_USHORT(capture->rect.r.h),
  };
  rfb_t *rfb = capture->rfb;
  unsigned long bpw = 3 * g.width;
  unsigned long rowstride = 3 * rfb->width;
  unsigned long srcX, srcY;
  unsigned char *source, *tmp, *target;
  rfbCopyRect msg;
  int size;
  int i;

  /* Read source rectangle position */
  size = frame_read_chunk(capture, &msg, sz_rfbCopyRect);
  if ( size < 0 )
    return size;

  if ( size != sz_rfbCopyRect ) {
    fprintf(stderr, "RFB: Received Update CopyRect with wrong length (%d/%d)\n", size, sz_rfbCopyRect);
    return -1;
  }

  srcX = CARD16_TO_USHORT(msg.srcX);
  srcY = CARD16_TO_USHORT(msg.srcY);

  DEBUG("Update copyrect: srcX=%lu srcY=%lu => %s\n",
	srcX, srcY, frame_geometry_str(&g));

  /* Alloc tmp buffer */
  tmp = malloc(bpw * g.height);

  /* Copy rectangle pixels */
  source = capture->rfb_buf + (3 * ((rfb->width * srcY) + srcX));
  target = tmp;

  for (i = 0; i < g.height; i++) {
    memcpy(target, source, bpw);
    source += rowstride;
    target += bpw;
  }

  /* Paste rectangle pixels */
  source = tmp;
  target = capture->rfb_buf + (3 * ((rfb->width * g.y ) + g.x));

  for (i = 0; i < g.height; i++) {
    memcpy(target, source, bpw);
    source += bpw;
    target += rowstride;
  }

  /* Free tmp buffer */
  free(tmp);

  /* Call frame update handler */
  frame_read_update(capture, &g);

  return 1;
}


static int frame_read_pixel(rfb_capture_t *capture)
{
  int ret = 0;

  switch ( CARD32_TO_ULONG(capture->rect.encoding) ) {
  case rfbEncodingRaw:
    ret = frame_read_raw(capture);
    break;

  case rfbEncodingCopyRect:
    ret = frame_read_copyrect(capture);
    break;

  default:
    /* Should never occur */
    fprintf(stderr, "RFB: received Update pixels with unsupported encoding (%lu)\n", CARD32_TO_ULONG(capture->rect.encoding));
    ret = -1;
    break;
  }

  /* Abort if error */
  if ( ret < 0 ) {
    frame_idle(capture);
    return ret;
  }

  /* Keep receiving if rectangle is not complete */
  if ( ret == 0 )
    return 0;

  /* Prepare to receive next rectangle (if any) */
  capture->nrects--;
  if ( capture->nrects > 0 ) {
    frame_set_state(capture, FRAME_STATE_RECTANGLE, 1);
    return 0;
  }

  /* Update completed: wake-up frame processing */
  DEBUG("Update completed\n");
  frame_idle(capture);

  return 0;
}


static int frame_read_cuttext(rfb_capture_t *capture)
{
  if ( (capture->cutlen > 0) && (capture->cutidx < capture->cutlen) ) {
    int size;

    size = frame_read_chunk(capture, capture->cutbuf + capture->cutidx, capture->cutlen - capture->cutidx);
    if ( size < 0 )
      return size;

    capture->cutidx += size;
  }

  if ( capture->cutidx >= capture->cutlen ) {
    int i;

    printf("\r%s[%lu] ", log_hdr_("CUTTEXT"), capture->cutlen);
    for (i = 0; i < capture->cutlen; i++) {
      unsigned char c = capture->cutbuf[i];
      if ( (c >= ' ') && (c < 128) )
        printf("%c", c);
      else
        printf("\\%u", c);
    }
    printf("\n");

    frame_set_state(capture, capture->cutstate, 0);
  }

  return 0;
}


static int frame_read_trash(rfb_capture_t *capture)
{
  char buf[32768];
  int size;

  size = frame_read_chunk(capture, buf, sizeof(buf));
  if ( size < 0 )
    return size;

  fprintf(stderr, "RFB: received %d unexpected data bytes\n", size);

  frame_idle(capture);

  return 0;
}


static int frame_read(rfb_capture_t *capture)
{
  int ret = 0;

  switch ( capture->state ) {
  case FRAME_STATE_IDLE:
  case FRAME_STATE_WAIT:
    ret = frame_read_header(capture);
    break;

  case FRAME_STATE_RECTANGLE:
    ret = frame_read_rectangle(capture);
    break;

  case FRAME_STATE_PIXEL:
    ret = frame_read_pixel(capture);
    break;

  case FRAME_STATE_CUTTEXT:
    ret = frame_read_cuttext(capture);
    break;

  default:
    ret = frame_read_trash(capture);
    break;
  }

  if ( ret < 0 )
    frame_idle(capture);

  return ret;
}


static void _frame_action_key(capture_t *capture, int down, unsigned long key)
{
  rfb_KeyEvent(RFB_CAPTURE(capture)->rfb, down, key);
}


static void _frame_action_pointer(capture_t *capture, unsigned char buttons, unsigned int x, unsigned int y)
{
  rfb_PointerEvent(RFB_CAPTURE(capture)->rfb, buttons, x, y);
}


static void _frame_action_scroll(capture_t *capture, unsigned char direction)
{
  rfb_PointerScroll(RFB_CAPTURE(capture)->rfb, direction);
}


static void _frame_show_status(capture_t *capture, char *hdr)
{
  rfb_Report(RFB_CAPTURE(capture)->rfb, stdout, hdr);
}


capture_interface_t _capture_interface = {
  open :           _frame_open,
  close :          _frame_close,
  set_window :     _frame_set_window,
  get_window :     _frame_get_window,
  set_period :     _frame_set_period,
  get_period :     _frame_get_period,

  refresh :        _frame_refresh,

  action_key :     _frame_action_key,
  action_pointer : _frame_action_pointer,
  action_scroll :  _frame_action_scroll,

  attr_set:        NULL,
  attr_get:        NULL,

  show_status :    _frame_show_status,
};
