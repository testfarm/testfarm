/****************************************************************************/
/* TestFarm                                                                 */
/* Test Engine - Debug information                                          */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 17-AUG-1999                                                    */
/****************************************************************************/

/*
 * $Revision: 42 $
 * $Date: 2006-06-03 15:30:01 +0200 (sam., 03 juin 2006) $
 */

#include <stdio.h>
#include <stdarg.h>

#include "debug.h"

int debug_flag = 0;


void debug_printf(const char *fmt, ...)
{
  va_list ap;

  if ( ! debug_flag )
    return;

  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  va_end(ap);
}


void debug(const char *fmt, ...)
{
  va_list ap;

  if ( ! debug_flag )
    return;

  fprintf(stderr, DEBUG_HEADER);

  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  va_end(ap);
}


void debug_errno(const char *fmt, ...)
{
  va_list ap;

  if ( ! debug_flag )
    return;

  fprintf(stderr, DEBUG_HEADER);

  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  va_end(ap);

  perror("");
}
