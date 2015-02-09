/****************************************************************************/
/* TestFarm VNC Interface                                                   */
/* PPM image file management                                                */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 23-DEC-2003                                                    */
/****************************************************************************/

/*
 * $Revision: 980 $
 * $Date: 2008-03-04 17:54:24 +0100 (mar., 04 mars 2008) $
 */

#ifndef __RFB_PPM_H__
#define __RFB_PPM_H__

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

#endif /* __RFB_GRAB_H__ */
