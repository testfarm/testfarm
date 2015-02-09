/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* Text Pattern matching                                                    */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 27-JUN-2007                                                    */
/****************************************************************************/

/*
 * $Revision: 879 $
 * $Date: 2007-12-27 21:58:00 +0100 (jeu., 27 déc. 2007) $
 */

#ifndef __TVU_MATCH_TEXT_H__
#define __TVU_MATCH_TEXT_H__

#include "pattern.h"
#include "frame.h"

extern int match_text_dump;

extern int match_text_request(pattern_t *pattern);
extern int match_text_info(pattern_t *pattern, char *str, int len);

#endif /* __TVU_MATCH_TEXT_H__ */
