/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* Input event recorder                                                     */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 01-APR-2007                                                    */
/****************************************************************************/

/*
 * $Revision: 586 $
 * $Date: 2007-06-21 14:11:50 +0200 (jeu., 21 juin 2007) $
 */

#ifndef __RFB_RECORD_H__
#define  __RFB_RECORD_H__

extern void record_init(FILE *f);
extern int record_set_async(int async);

extern void record_date(void);
extern void record_delay(void);

extern void record_key(unsigned long keyval, unsigned char down);
extern int record_position(int x, int y);
extern void record_buttons(unsigned char buttons);
extern void record_scroll(unsigned char direction);

#endif /*  __RFB_RECORD_H__ */
