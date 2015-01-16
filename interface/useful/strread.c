/****************************************************************************/
/* TestFarm                                                                 */
/* LF-terminated read primitive                                             */
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
#include <unistd.h>

#include "useful.h"

int strread(int fd, char *s, int size)
{
  char c = NUL;
  int count = 0;

  while ( (c != '\n') && (count < (size-1)) ) {
    c = '\n';
    if ( read(fd, &c, 1) > 0 ) {
      *(s++) = c;
      count++;
    }
  }
  *s = NUL;

  return count;
}
