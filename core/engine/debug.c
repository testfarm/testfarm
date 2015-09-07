/****************************************************************************/
/* TestFarm                                                                 */
/* Test Engine - Debug information                                          */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 17-AUG-1999                                                    */
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
