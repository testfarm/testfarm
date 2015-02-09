/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* File Pseudo-Capture device                                               */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 13-DEC-2007                                                    */
/****************************************************************************/

/*
 * $Revision: 979 $
 * $Date: 2008-03-04 11:18:50 +0100 (mar., 04 mars 2008) $
 */

#ifndef __TVU_FILEDEV_H__
#define __TVU_FILEDEV_H__

#include <glib.h>
#include <sys/types.h>
#include "capture_device.h"


#define FILE_DEV(_ptr_) ((file_device_t *)(_ptr_))

typedef struct {
  char *fname;
  unsigned long tstamp;
} file_device_frame_t;

typedef struct {
  capture_dev_t h;
  int nfiles;
  file_device_frame_t *files;
  int fmt;
  int current;
  guint timeout_tag;
  ino_t ino;
  time_t mtime;
} file_device_t;

#endif /* __TVU_FILEDEV_H__ */
