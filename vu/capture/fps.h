/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* Video Frame Rate statistics                                              */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 04-MAY-2007                                                    */
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

#ifndef __TVU_FPS_H__
#define __TVU_FPS_H__

#define FPS_NFRAMES 100

typedef struct {
  unsigned long long t0;
  unsigned long nframes[FPS_NFRAMES];
} fps_t;

extern void fps_init(fps_t *fps);
extern void fps_update(fps_t *fps, unsigned long long tstamp);
extern unsigned long fps_compute(fps_t *fps);

#endif /* __TVU_FPS_H__ */
