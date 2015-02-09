/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* V4L1 Capture Device                                                      */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 28-FEB-2008                                                    */
/****************************************************************************/

/*
 * $Revision: 974 $
 * $Date: 2008-02-29 22:35:59 +0100 (ven., 29 févr. 2008) $
 */

#ifndef __TVU_CAPTURE_V4L1DEV_H__
#define __TVU_CAPTURE_V4L1DEV_H__

#include <glib.h>

#include "fps.h"
#include "decoder.h"
#include "capture_device.h"


#define V4L_DEV(_ptr_) ((v4l_device_t *)(_ptr_))

typedef struct {
  capture_dev_t h;

  int fd;        /* Video device file descriptor */

  capture_func callback_func;
  void *callback_ctx;

  char *hwinfo;
  char pixelformat_str[5];
  int pixelformat;
  unsigned long imagesize;
  int input;
  char *input_str;
  int streaming;
  struct video_mbuf mbuf;
  void *buf;
  decoder_t *decoder;
  fps_t fps;

  GThread *read_thread;             /* Read loop thread */  
  int read_in[2];
  int read_out[2];
  GIOChannel *read_out_channel;
  guint read_out_tag;
} v4l_device_t;

#endif /* __TVU_CAPTURE_V4LDEV_H__ */
