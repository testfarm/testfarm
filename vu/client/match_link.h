/****************************************************************************/
/* TestFarm                                                                 */
/* Virtual User subprocess link management                                  */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 11-APR-2007                                                    */
/****************************************************************************/

/*
 * $Revision: 1232 $
 * $Date: 2012-04-08 16:17:42 +0200 (dim., 08 avril 2012) $
 */

#ifndef __RFB_MATCH_LINK_H__
#define __RFB_MATCH_LINK_H__

#ifndef ENABLE_GLIB
#define ENABLE_GLIB
#endif

#include "tstamp.h"

extern int match_link_dump(char *id, tstamp_t tstamp, char *str);

#endif /* __RFB_MATCH_LINK_H__ */
