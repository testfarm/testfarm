/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* Image Pattern matching                                                   */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 27-JUN-2007                                                    */
/****************************************************************************/

/*
 * $Revision: 879 $
 * $Date: 2007-12-27 21:58:00 +0100 (jeu., 27 déc. 2007) $
 */

#ifndef __TVU_MATCH_IMAGE_H__
#define __TVU_MATCH_IMAGE_H__

#include "pattern.h"

extern int match_image_init(void);
extern void match_image_done(void);

extern int match_image_request(pattern_t *pattern);
extern int match_image_info(pattern_t *pattern, char *str, int len);

#endif /* __TVU_MATCH_IMAGE_H__ */
