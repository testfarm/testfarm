/****************************************************************************/
/* TestFarm                                                                 */
/* Test Engine - Debug information                                          */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 17-AUG-1999                                                    */
/****************************************************************************/

/*
 * $Revision: 174 $
 * $Date: 2006-07-26 17:43:56 +0200 (mer., 26 juil. 2006) $
 */

#ifndef __DEBUG_H__
#define __DEBUG_H__

#define NAME "testfarm-engine"
#define DEBUG_HEADER "[" NAME "] DEBUG: "

/* Debug enable flag */
extern int debug_flag;

/* Debug information dump primitives */
extern void debug(const char *fmt, ...);
extern void debug_printf(const char *fmt, ...);
extern void debug_errno(const char *fmt, ...);

#endif /* __DEBUG_H__ */
