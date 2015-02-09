/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* PNG image file management                                                */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 03-MAR-2008                                                    */
/****************************************************************************/

/*
 * $Revision: 980 $
 * $Date: 2008-03-04 17:54:24 +0100 (mar., 04 mars 2008) $
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
