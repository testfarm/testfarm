/****************************************************************************/
/* TestFarm                                                                 */
/* DataLogger interface : time stamp                                        */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 22-FEB-2005                                                    */
/****************************************************************************/

/*
 * $Revision: 29 $
 * $Date: 2006-06-03 10:39:29 +0200 (sam., 03 juin 2006) $
 */

#ifndef __INTERFACE_TSTAMP_H__
#define __INTERFACE_TSTAMP_H__

typedef unsigned long long tstamp_t;

extern tstamp_t tstamp_get(void);

#endif /* __INTERFACE_TSTAMP_H__ */
