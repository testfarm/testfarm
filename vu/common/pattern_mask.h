/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* Image pattern Mask management                                            */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 23-JUL-2007                                                    */
/****************************************************************************/

/*
 * $Revision: 979 $
 * $Date: 2008-03-04 11:18:50 +0100 (mar., 04 mars 2008) $
 */

#ifndef __TVU_PATTERN_MASK_H__
#define __TVU_PATTERN_MASK_H__

typedef struct {
  char *source;                 /* Mask source specification */
  unsigned int width;           /* Mask width */
  unsigned int height;          /* Mask height */
  unsigned char *buf;           /* Mask RGB buffer */
  unsigned int n;               /* Number of unmasked pixels */
} pattern_mask_t;

extern void pattern_mask_set_rgb(pattern_mask_t *mask,
				 char *source,
				 unsigned int width, unsigned int height,
				 unsigned char *rgb);
extern void pattern_mask_set_alpha(pattern_mask_t *mask,
				   unsigned int width, unsigned int height,
				   unsigned char *alpha);

extern void pattern_mask_clip(pattern_mask_t *mask, unsigned int width, unsigned int height);
extern void pattern_mask_clear(pattern_mask_t *mask);
extern void pattern_mask_copy(pattern_mask_t *mask, pattern_mask_t *mask0);

#endif /* __TVU_PATTERN_MASK_H__ */
