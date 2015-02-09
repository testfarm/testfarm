/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* JPEG image file management                                               */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 27-NOV-2007                                                    */
/****************************************************************************/

/*
 * $Revision: 825 $
 * $Date: 2007-11-30 16:32:01 +0100 (ven., 30 nov. 2007) $
 */

#ifndef __TVU_JPEG_H__
#define __TVU_JPEG_H__

extern char *jpeg_filename_suffix(char *filename);

extern int jpeg_size(char *fname, unsigned int *pwidth, unsigned int *pheight);
extern int jpeg_load(char *fname, unsigned int *pwidth, unsigned int *pheight, unsigned char **pbuf);

#endif /* __TVU_JPEG_H__ */
