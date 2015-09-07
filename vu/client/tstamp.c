/*
    This file is part of TestFarm,
    the Test Automation Tool for Embedded Software.
    Please visit http://www.testfarm.org.

    TestFarm is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    TestFarm is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with TestFarm.  If not, see <http://www.gnu.org/licenses/>.
*/

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
