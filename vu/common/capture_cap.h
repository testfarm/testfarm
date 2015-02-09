/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* Capture Device Capabilities                                              */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 06-NOV-2007                                                    */
/****************************************************************************/

/*
 * $Revision: 793 $
 * $Date: 2007-11-06 22:44:29 +0100 (mar., 06 nov. 2007) $
 */

#ifndef __TVU_CAPTURE_CAP_H__
#define __TVU_CAPTURE_CAP_H__

#define CAPTURE_CAP_BIT(n) (1<<(n))
#define CAPTURE_CAP_VIDEO   CAPTURE_CAP_BIT(0)
#define CAPTURE_CAP_KEY     CAPTURE_CAP_BIT(1)
#define CAPTURE_CAP_POINTER CAPTURE_CAP_BIT(2)
#define CAPTURE_CAP_SCROLL  CAPTURE_CAP_BIT(3)
#define CAPTURE_CAP_INPUT   (CAPTURE_CAP_KEY|CAPTURE_CAP_POINTER|CAPTURE_CAP_SCROLL)

typedef unsigned int capture_cap_t;

#endif /* __TVU_CAPTURE_CAP_H__ */
