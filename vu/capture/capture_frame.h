/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* Capture Device - Frame management                                        */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 26-APR-2007                                                    */
/****************************************************************************/

/*
 * $Revision: 1036 $
 * $Date: 2008-12-07 14:18:30 +0100 (dim., 07 déc. 2008) $
 */

#ifndef __TVU_CAPTURE_FRAME_H__
#define __TVU_CAPTURE_FRAME_H__

#include "frame_buf.h"
#include "capture_interface.h"
#include "capture_device.h"


#define CAPTURE_FRAME(_ptr_) ((capture_frame_t *)(_ptr_))

typedef struct {
  capture_t h;
  frame_buf_t *fb;             /* Pointer to RGB Frame Buffer */
  frame_geometry_t window;     /* Active screen window */

  capture_update_fn *update;
  void *data;

  capture_dev_t *dev;

  unsigned long proc_period;        /* Frame processing period in milliseconds */
  guint proc_timeout_tag;           /* Frame processing timeout tag */
  GThread *proc_thread;             /* Frame processing thread */
  int proc_in[2];
  int proc_out[2];
  GIOChannel *proc_out_channel;
  guint proc_out_tag;

  int rotate;
  unsigned int window_ofs;     /* Offset of active screen window in RGB buffer */

  char refresh_requested;
  char refresh_full;
  char processing;
} capture_frame_t;

#endif /* __TVU_CAPTURE_FRAME_H__ */
