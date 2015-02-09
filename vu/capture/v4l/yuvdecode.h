/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* YUV frame decoder                                                       */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 28-FEB-2008                                                    */
/****************************************************************************/

/*
 * $Revision: 974 $
 * $Date: 2008-02-29 22:35:59 +0100 (ven., 29 févr. 2008) $
 */

#ifndef __TVU_YUVDECODE_H__
#define __TVU_YUVDECODE_H__

#include "decoder.h"

extern decoder_t *yuv_create(unsigned long fmt, int width, int height);

#endif /* __TVU_YUVDECODE_H__ */
