/****************************************************************************/
/* TestFarm                                                                 */
/* Allocate + Read line buffer from file stream                             */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 22-APR-2004                                                    */
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
#include <malloc.h>

#include "useful.h"


int fgets2(FILE *f, char **pbuf, int *psize)
{
  int c = 0;
  int len = 0;

  if ( feof(f) )
    return -1;

  while ( (!feof(f)) && (c != '\n') ) {
    c = fgetc(f);

    if ( c == -1 )
      return -1;

    /* Grow buffer if not large enough */
    if ( (len+1) >= (*psize) ) {
      (*psize) += 128;
      (*pbuf) = realloc((*pbuf), (*psize));
    }

    if ( c != '\n' ) {
      (*pbuf)[len++] = c;
    }
  }

  if ( (*pbuf) != NULL )
    (*pbuf)[len] = '\0';

  return len;
}
