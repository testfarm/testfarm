/****************************************************************************/
/* TestFarm                                                                 */
/* Data Logger Interface : subprocesses management                          */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 28-MAR-2000                                                    */
/****************************************************************************/

/*
 * $Revision: 1220 $
 * $Date: 2011-12-04 17:01:50 +0100 (dim., 04 déc. 2011) $
 */

#ifndef __INTERFACE_SUB_H__
#define __INTERFACE_SUB_H__

extern int sub_init(void);
extern void sub_done(void);

#ifdef ENABLE_GLIB
extern void sub_use_glib(void);
#endif

#endif /* __INTERFACE_SUB_H__ */
