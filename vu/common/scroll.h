/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* Scroll dir definitions                                                   */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 21-JUN-2007                                                    */
/****************************************************************************/

/*
 * $Revision: 586 $
 * $Date: 2007-06-21 14:11:50 +0200 (jeu., 21 juin 2007) $
 */

#ifndef __TVU_SCROLL_H__
#define __TVU_SCROLL_H__

#define SCROLL_UP    1
#define SCROLL_DOWN  2
#define SCROLL_LEFT  3
#define SCROLL_RIGHT 4

extern char *scroll_str(unsigned char direction);

#endif /* __TVU_SCROLL_H__ */
