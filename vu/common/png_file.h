/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* PNG image file management                                                */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 03-MAR-2008                                                    */
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

#ifndef __TVU_PNG_FILE_H__
#define __TVU_PNG_FILE_H__

#include "frame_geometry.h"
#include "frame_rgb.h"

#define PNG_SUFFIX ".png"

extern char *png_filename_suffix(char *filename);
extern char *png_filename(char *filename);

extern int png_size(char *fname, unsigned int *pwidth, unsigned int *pheight);
extern int png_load(char *fname, unsigned int *pwidth, unsigned int *pheight,
		    unsigned char **pbuf);
extern int png_load_full(char *fname, unsigned int *pwidth, unsigned int *pheight,
			 unsigned char **pbuf, unsigned char **palpha);

extern int png_save_buf(unsigned char *rgb_buf, unsigned int rgb_width,
			frame_geometry_t *g, char *fname);
extern int png_save(frame_rgb_t *rgb, frame_geometry_t *g, char *filename);

#endif /* __TVU_PNG_FILE_H__ */
