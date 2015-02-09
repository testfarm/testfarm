/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* Generic Frame Descriptor Header                                          */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 14-NOV-2007                                                    */
/****************************************************************************/

/*
 * $Revision: 850 $
 * $Date: 2007-12-10 17:20:50 +0100 (lun., 10 déc. 2007) $
 */

#ifndef __TVU_FRAME_HDR_H__
#define __TVU_FRAME_HDR_H__

#include "frame_buf.h"

#define FRAME_HDR(_ptr_) ((frame_hdr_t *)(_ptr_))

typedef struct {
  char *id;              /* Frame identifier */
  int shmid;             /* Frame buffer shmid */
  frame_buf_t *fb;       /* Frame buffer pointer */
  frame_geometry_t g0;   /* Frame geometry in root frame */
} frame_hdr_t;

#endif /* __TVU_FRAME_HDR_H__ */
