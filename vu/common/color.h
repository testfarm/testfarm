/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* RGB Color specification                                                  */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 29-AUG-2007                                                    */
/****************************************************************************/

/*
 * $Revision: 816 $
 * $Date: 2007-11-27 12:33:34 +0100 (mar., 27 nov. 2007) $
 */

#ifndef __TVU_COLOR_H__
#define __TVU_COLOR_H__

typedef unsigned char color_rgb_t[3];

extern int color_parse(char *spec, unsigned long *pvalue);
extern void color_set_rgb(unsigned long value, color_rgb_t rgb);

extern void color_fill(unsigned long color, unsigned int width, unsigned int height, unsigned char **pbuf);
extern int color_fill_from_spec(char *spec, unsigned int *pwidth, unsigned int *pheight, unsigned char **pbuf);

#endif /* __TVU_COLOR_H__ */
