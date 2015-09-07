/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* Display control buffer - RGB frame buffer                                */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 28-JUN-2007                                                    */
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

#ifndef __TVU_FRAME_RGB_H__
#define __TVU_FRAME_RGB_H__

#include "frame_geometry.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define FRAME_RGB_BPP 3


/* Allocate this amount of unused spare bytes to avoid memory faults
   when accessing buffer using 64-bit MMX instructions. */
#define FRAME_RGB_MMTAIL (8-FRAME_RGB_BPP)


typedef struct {
  unsigned int width, height;          /* RGB frame size */
  unsigned int bpp;                    /* Numbre of bytes per pixel */
  unsigned int rowstride;              /* Number of bytes per row */
  unsigned char buf[1];                /* RGB frame buffer */
} frame_rgb_t;


extern unsigned int frame_rgb_bufsize(unsigned int width, unsigned int height);
extern void frame_rgb_init(frame_rgb_t *rgb, unsigned int width, unsigned int height);

extern void frame_rgb_clip_geometry(frame_rgb_t *rgb, frame_geometry_t *g);
extern int frame_rgb_parse_geometry(frame_rgb_t *rgb, char *str, frame_geometry_t *g);

#ifdef __cplusplus
}
#endif

#endif /* __TVU_FRAME_RGB_H__ */
