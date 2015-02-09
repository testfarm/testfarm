/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* Image file name constructor                                              */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 04-MAR-2008                                                    */
/****************************************************************************/

/*
 * $Revision: 980 $
 * $Date: 2008-03-04 17:54:24 +0100 (mar., 04 mars 2008) $
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
