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
