/****************************************************************************/
/* TestFarm VNC Interface                                                   */
/* PPM image file management                                                */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 23-DEC-2003                                                    */
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

#ifndef __TVU_PPM_H__
#define __TVU_PPM_H__

#include "frame_geometry.h"
#include "frame_rgb.h"

#define PPM_SUFFIX ".ppm.gz"

extern char *ppm_filename(char *filename);
extern char *ppm_filename_suffix(char *filename);

extern int ppm_save_buf(unsigned char *rgb_buf, unsigned int rgb_width,
                        frame_geometry_t *g, char *fname);

extern int ppm_save(frame_rgb_t *rgb, frame_geometry_t *g, char *filename);
extern int ppm_size(char *fname, unsigned int *pwidth, unsigned int *pheight);
extern int ppm_load(char *fname, unsigned int *pwidth, unsigned int *pheight, unsigned char **pbuf);

#endif /* __TVU_GRAB_H__ */
