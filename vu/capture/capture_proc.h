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
