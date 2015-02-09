/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* Image Pattern management                                                 */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 27-JUN-2007                                                    */
/****************************************************************************/

/*
 * $Revision: 1131 $
 * $Date: 2010-03-31 16:01:06 +0200 (mer., 31 mars 2010) $
 */

#ifndef __TVU_PATTERN_IMAGE_H__
#define __TVU_PATTERN_IMAGE_H__

#include <glib.h>
#include "pattern.h"

extern int pattern_image_set(pattern_t *pattern, char *filename, char **err);
extern int pattern_image_set_options(pattern_t *pattern, GList *options, char **err);
extern void pattern_image_free(pattern_t *pattern);
extern void pattern_image_copy(pattern_t *pattern, pattern_t *pattern0);
extern int pattern_image_str(pattern_t *pattern, char *buf, int size);
extern int pattern_image_diff(pattern_t *p1, pattern_t *p2);

#endif /* __TVU_PATTERN_IMAGE_H__ */
