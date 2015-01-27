/****************************************************************************/
/* TestFarm                                                                 */
/* Test Engine - Timer messaging                                            */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 19-NOV-1999                                                    */
/****************************************************************************/

/*
 * $Revision: 42 $
 * $Date: 2006-06-03 15:30:01 +0200 (sam., 03 juin 2006) $
 */

#ifndef __ENGINE_TIMER_H__
#define __ENGINE_TIMER_H__

#include <sys/time.h>

#include "shell.h"

extern int timer_value(shell_t *shell, char *s_value, char *s_unit, long *t);
extern void timer_tv(struct timeval *tv, unsigned long t);

#endif /* __ENGINE_TIMER_H__ */
