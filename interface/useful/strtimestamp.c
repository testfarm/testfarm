/****************************************************************************/
/* TestFarm                                                                 */
/* String-formatted time stamp                                              */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 26-AUG-1999                                                    */
/****************************************************************************/

/*
 * $Revision: 29 $
 * $Date: 2006-06-03 10:39:29 +0200 (sam., 03 juin 2006) $
 */

#include <stdio.h>
#include <sys/time.h>

char *strtimestamp(void)
{
  static long t0 = -1;
  static char buf[80];
  struct timeval tv;

  /* Get time information */
  gettimeofday(&tv, NULL);

  /* Set T0 date if not already done */
  if ( t0 < 0 )
    t0 = tv.tv_sec;

  /* Display local timestamp and subset-id */
  sprintf(buf, "%ld%06ld ", tv.tv_sec - t0, tv.tv_usec);

  return buf;
}
