/****************************************************************************/
/* TestFarm                                                                 */
/* LF-terminated read primitive                                             */
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
