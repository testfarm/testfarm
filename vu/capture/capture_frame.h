/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* Capture Device - Frame management                                        */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 26-APR-2007                                                    */
/****************************************************************************/

/*
    This file is part of TestFarm,
    the Test Automation Tool for Embedded Software.
    Please visit http://www.testfarm.org.

    TestFarm is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    TestFarm is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with TestFarm.  If not, see <http://www.gnu.org/licenses/>.
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
