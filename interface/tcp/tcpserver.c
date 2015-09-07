/****************************************************************************/
/* TestFarm                                                                 */
/* TCP/IP server wrapper - Test server                                      */
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
