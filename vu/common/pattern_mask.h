/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* Image pattern Mask management                                            */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 23-JUL-2007                                                    */
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
