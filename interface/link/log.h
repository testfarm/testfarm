/****************************************************************************/
/* TestFarm                                                                 */
/* Test Interface library: log management                                   */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 30-MAR-2000                                                    */
/****************************************************************************/

/*
 * $Revision: 305 $
 * $Date: 2006-11-26 16:42:47 +0100 (dim., 26 nov. 2006) $
 */

#ifndef __INTERFACE_LOG_H__
#define __INTERFACE_LOG_H__

#include "tstamp.h"

#define TAG_INIT  "INIT   "
#define TAG_DONE  "DONE   "

extern char *log_hdr(tstamp_t tstamp, char *tag);
extern char *log_hdr_(char *tag);

extern void log_error(char *tag, char *fmt, ...);
extern void log_dump(char *hdr, char *s);

#endif /* __INTERFACE_LOG_H__ */
