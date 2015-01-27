/****************************************************************************/
/* TestFarm                                                                 */
/* Elapsed time string generation                                           */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 28-APR-2004                                                    */
/****************************************************************************/

/*
 * $Revision: 42 $
 * $Date: 2006-06-03 15:30:01 +0200 (sam., 03 juin 2006) $
 */

#include <stdio.h>
#include "report.h"


char *report_elapsed(long dt_ms)
{
  long dt_h, dt_min, dt_s;
  static char str[80];

  dt_s = dt_ms / 1000;
  dt_ms %= 1000;
  dt_min = dt_s / 60;
  dt_s %= 60;
  dt_h = dt_min / 60;
  dt_min %= 60;

  if ( (dt_h+dt_min) > 0 ) {
    snprintf(str, sizeof(str), "%02ld:%02ld:%02ld", dt_h, dt_min, dt_s);
  }
  else {
    snprintf(str, sizeof(str), "%ld.%03ld s", dt_s, dt_ms);
  }

  return str;
}
