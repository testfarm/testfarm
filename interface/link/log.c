/****************************************************************************/
/* TestFarm                                                                 */
/* Test Interface library: log management                                   */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 30-MAR-2000                                                    */
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
#include <string.h>

#include "tstamp.h"
#include "log.h"


char *log_hdr(tstamp_t tstamp, char *tag)
{
  static char buf[80];

  /* Display timestamp and tag */
  snprintf(buf, sizeof(buf), "%llu %s ", tstamp, tag);

  return buf;
}


char *log_hdr_(char *tag)
{
  return log_hdr(tstamp_get(), tag);
}


void log_error(char *tag, char *fmt, ...)
{
  va_list ap;

  printf("%s*ERROR* ", log_hdr_(tag));
 
  if ( fmt != NULL ) {
    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);
  }

  printf("\n");
}


void log_dump(char *hdr, char *s)
{
  char *p = s;

  while ( (p != NULL) && (*p != '\0') ) {
    /* Seek end of line */
    char *end = strchr(p, '\n');
    if ( (end != NULL) && (*end != '\0') )
      *(end++) = '\0';

    /* Display header + line */
    printf("%s%s\n", hdr, p);

    p = end;
  }
}
