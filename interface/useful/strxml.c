/****************************************************************************/
/* TestFarm                                                                 */
/* XML to raw string conversion                                             */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 23-APR-2004                                                    */
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

#include <string.h>

#include "useful.h"


char *strxml(char *str)
{
  int i_get = 0;
  int i_put = 0;

  while ( str[i_get] != NUL ) {
    char c = str[i_get++];

    if ( c == '&' ) {
      if ( strncmp(str+i_get, "amp;", 4) == 0 ) {
        i_get += 4;
      }
      else if ( strncmp(str+i_get, "lt;", 3) == 0 ) {
        i_get += 3;
        c = '<';
      }
      else if ( strncmp(str+i_get, "gt;", 3) == 0 ) {
        i_get += 3;
        c = '>';
      }
      else if ( strncmp(str+i_get, "apos;", 5) == 0 ) {
        i_get += 5;
        c = '\'';
      }
      else if ( strncmp(str+i_get, "quot;", 5) == 0 ) {
        i_get += 5;
        c = '"';
      }
    }

    str[i_put++] = c;
  }

  str[i_put] = NUL;

  return str;
}
