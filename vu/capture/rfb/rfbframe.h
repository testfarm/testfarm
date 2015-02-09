/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* RFB Frame Device                                                         */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 25-APR-2007                                                    */
/****************************************************************************/

/*
 * $Revision: 1036 $
 * $Date: 2008-12-07 14:18:30 +0100 (dim., 07 déc. 2008) $
 */

#ifndef __TVU_RFBFRAME_H__
#define __TVU_RFBFRAME_H__

#include "frame_buf.h"
#include "rfbproto.h"
#include "capture_interface.h"


#define RFB_CAPTURE(_ptr_) ((rfb_capture_t *)(_ptr_))

typedef struct {
  capture_t h;
  frame_buf_t *fb;             /* Pointer to RGB Frame Buffer */
  frame_geometry_t window;     /* Active screen window */

  capture_update_fn *update;
  void *data;

  rfb_t *rfb;
  GIOChannel *rfb_channel;
  guint rfb_tag;
  guint rfb_timeout;

  unsigned char *rfb_buf;              /* Incoming frame buffer */
  frame_geometry_t rfb_g;              /* Recently refreshed area */
  int refresh_requested;               /* Refresh requested but nothing new from RFB server */

  unsigned long delay;                 /* Frame refresh delay in milliseconds */
  unsigned int state;                  /* State of frame buffer update */
  guint watchdog_tag;                  /* Read watchdog timeout tag */
  unsigned int nrects;
  rfbFramebufferUpdateRectHeader rect;
  unsigned char *rowbuf;
  unsigned long rowsize;
  unsigned long xi, yi;

  unsigned long cutlen;                /* Number of cut text characters */
  unsigned long cutidx;                /* Cut text receive index */
  unsigned char *cutbuf;
  unsigned long cutstate;              /* Cut text previous state backup */

  void *read_buf;
  int read_size;
  int read_ofs;
} rfb_capture_t;

#endif /* __TVU_RFBFRAME_H__ */
