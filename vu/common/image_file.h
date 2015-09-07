/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* Image file name constructor                                              */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 04-MAR-2008                                                    */
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

#ifndef __TVU_IMAGE_FILE_H__
#define __TVU_IMAGE_FILE_H__

extern char *image_filename(char *filename,
			    char *suffix_str,
			    char * (*suffix_fn)(char *filename),
			    char *suffix_re);

extern unsigned char *image_dup(unsigned char *rgb_buf,
				unsigned int rgb_width, unsigned int rgb_height);

extern int image_load(char *fname,
		      unsigned int *pwidth, unsigned int *pheight,
		      unsigned char **pbuf, unsigned char **palpha,
		      char **perr);

#endif /* __TVU_IMAGE_FILE_H__ */
