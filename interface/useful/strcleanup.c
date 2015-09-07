/****************************************************************************/
/* TestFarm                                                                 */
/* String cleanup utility                                                   */
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

#include "useful.h"

char *strcleanup(char *s)
{
  char *p = s;

  while ( *p != NUL ) {
    if ( *p < ' ' )
      *p = ' ';
    p++;
  }

  while (p > s) {
	  p--;
	  if (*p != ' ')
		  break;
	  *p = '\0';
  }

  return s;
}
