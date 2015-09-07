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

#ifndef __TVU_CAPTURE_PROC_H__
#define __TVU_CAPTURE_PROC_H__

#include "frame_geometry.h"
#include "capture_cap.h"

typedef enum {
  CAPTURE_PROC_CB_OPEN = 0,
  CAPTURE_PROC_CB_WINDOW,
  CAPTURE_PROC_CB_PERIOD,
  CAPTURE_PROC_CB_ATTR,
  CAPTURE_PROC_CMD_N
} capture_proc_cb_type_t;

typedef struct {
  unsigned short type;
  unsigned short len;
} capture_proc_cb_hdr_t;

typedef struct {
  capture_proc_cb_hdr_t hdr;
  int shmid;                 /* Frame buffer shmid */
  capture_cap_t cap;         /* Capture device capabilities */
  char name[1];              /* Source device or connection name */
} capture_proc_cb_open_t;

typedef struct {
  capture_proc_cb_hdr_t hdr;
  frame_geometry_t g;
} capture_proc_cb_window_t;

typedef struct {
  capture_proc_cb_hdr_t hdr;
  long delay;
} capture_proc_cb_period_t;

typedef struct {
  capture_proc_cb_hdr_t hdr;
  char key_value[1];
} capture_proc_cb_attr_t;

#endif /* __TVU_CAPTURE_PROC_H__ */
