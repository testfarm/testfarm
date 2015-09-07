/****************************************************************************/
/* TestFarm                                                                 */
/* Elapsed time string generation                                           */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 28-APR-2004                                                    */
/****************************************************************************/

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
