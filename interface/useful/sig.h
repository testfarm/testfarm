/****************************************************************************/
/* TestFarm                                                                 */
/* Shell Script Interpreter Library - Signal handling                       */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 26-AUG-1999                                                    */
/****************************************************************************/

/*
 * $Revision: 1075 $
 * $Date: 2009-10-29 10:33:09 +0100 (jeu., 29 oct. 2009) $
 */

#ifndef __SHELL_SIG_H__
#define __SHELL_SIG_H__

typedef void sig_terminate_t(void);

extern void sig_init(void (*terminate)(void));
extern void sig_done(void);

#endif /* __SHELL_SIG_H__ */
