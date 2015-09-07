/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* Generic Frame Descriptor Header                                          */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 14-NOV-2007                                                    */
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
