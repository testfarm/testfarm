/****************************************************************************/
/* TestFarm                                                                 */
/* XML to raw string conversion                                             */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 23-APR-2004                                                    */
/****************************************************************************/

/*
 * $Revision: 29 $
 * $Date: 2006-06-03 10:39:29 +0200 (sam., 03 juin 2006) $
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
