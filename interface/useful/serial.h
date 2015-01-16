/****************************************************************************/
/* TestFarm                                                                 */
/* Test Engine - Serial line access                                         */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 17-AUG-1999                                                    */
/****************************************************************************/

/*
 * $Revision: 29 $
 * $Date: 2006-06-03 10:39:29 +0200 (sam., 03 juin 2006) $
 */

#ifndef __SERIAL_H__
#define __SERIAL_H__

extern int serial_open(char *dev);
extern void serial_close(void);

#endif /* __SERIAL_H__ */
