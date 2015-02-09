/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* V4L2 Capture Device                                                      */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 04-MAY-2007                                                    */
/****************************************************************************/

/*
 * $Revision: 977 $
 * $Date: 2008-03-03 10:39:17 +0100 (lun., 03 mars 2008) $
 */

#ifndef __TVU_V4LDEV_H__
#define __TVU_V4LDEV_H__

#include <glib.h>

#include "fps.h"
#include "decoder.h"
#include "capture_device.h"


#define V4L_DEV(_ptr_) ((v4l_device_t *)(_ptr_))

typedef struct {
  capture_dev_t h;

  int fd;
  GIOChannel *channel;
  guint tag;

  capture_func callback_func;
  void *callback_ctx;

  char *hwinfo;
  char pixelformat[5];
  unsigned long imagesize;
  int input;
  int streaming;
  void *buf;
  decoder_t *decoder;
  fps_t fps;
} v4l_device_t;


#endif /* __TVU_V4LDEV_H__ */
