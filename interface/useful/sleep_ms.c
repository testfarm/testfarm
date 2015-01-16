/****************************************************************************/
/* TestFarm                                                                 */
/* Millisecond accurate sleep                                               */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 04-JUN-1999                                                    */
/****************************************************************************/

/*
 * $Revision: 29 $
 * $Date: 2006-06-03 10:39:29 +0200 (sam., 03 juin 2006) $
 */

#include <unistd.h>

void sleep_ms(long msec)
{
  long sec;

  /* Compute time in timebase units */
  sec = msec / 1000;
  msec = msec % 1000;

  /* Perform wait */
  if ( sec > 0 )
    sleep(sec);
  usleep(1000*msec);
}
