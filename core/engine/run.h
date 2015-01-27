/****************************************************************************/
/* TestFarm                                                                 */
/* Test Engine - Runtime processing                                         */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 17-AUG-1999                                                    */
/****************************************************************************/

/*
 * $Revision: 42 $
 * $Date: 2006-06-03 15:30:01 +0200 (sam., 03 juin 2006) $
 */

#ifndef __RUN_H__
#define __RUN_H__

extern int run_init(periph_t *periph, int argc, char **argv, int interactive, char *wait);
extern int run_done(void);
extern int run_loop(void);

#endif /* __RUN_H__ */
