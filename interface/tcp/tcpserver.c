/****************************************************************************/
/* TestFarm                                                                 */
/* TCP/IP server wrapper - Test server                                      */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 17-AUG-1999                                                    */
/****************************************************************************/

/*
 * $Revision: 29 $
 * $Date: 2006-06-03 10:39:29 +0200 (sam., 03 juin 2006) $
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>

char *process(char *s)
{
  char *p = s;

  while ( *p != '\0' ) {
    *p = toupper(*p);
    p++;
  }

  return s;
}

int main(int argc, char *argv[])
{
  char buf[BUFSIZ];

  /* Set line-buffered standard output */
  setlinebuf(stdout);

  while ( !feof(stdin) ) {
    /* Get request */
    if ( fgets(buf, BUFSIZ-1, stdin) != NULL ) {
      if ( *buf != '\0' )
        fprintf(stderr, "%s: received: %s", argv[0], buf);

      /* Convert to upper case */
      process(buf);

      /* Send reply */
      fprintf(stderr, "%s: sending: %s", argv[0], buf);
      printf("%s", buf);
    }
  }

  fprintf(stderr, "%s: connection closed\n", argv[0]);

  return 0;
}
