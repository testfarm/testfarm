#include <stdio.h>
#include <sys/time.h>

#include "tstamp.h"

tstamp_t tstamp_get(void)
{
  static long t0 = -1;
  struct timeval tv;

  /* Get time information */
  gettimeofday(&tv, NULL);

  /* Set T0 date if not already done */
  if ( t0 < 0 )
    t0 = tv.tv_sec;

  return (((tstamp_t) (tv.tv_sec - t0)) * 1000000ULL) + tv.tv_usec;
}
