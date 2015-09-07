/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* Display control buffer                                                   */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 05-JAN-2004                                                    */
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

#ifndef __TVU_FRAME_BUF_H__
#define __TVU_FRAME_BUF_H__

#include "frame_rgb.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct {
  frame_rgb_t rgb;            /* RGB frame buffer */
} frame_buf_t;

extern frame_buf_t *frame_buf_alloc(unsigned int width, unsigned int height, int *_shmid);
extern frame_buf_t *frame_buf_map(int shmid, int readonly);
extern void frame_buf_free(frame_buf_t *fb);

#ifdef __cplusplus
}
#endif

#endif /* __TVU_FRAME_BUF_H__ */
