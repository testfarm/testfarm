/****************************************************************************/
/* TestFarm                                                                 */
/* String-wise uppercase conversion utility                                 */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 17-AUG-1999                                                    */
/****************************************************************************/

/*
 * $Revision: 29 $
 * $Date: 2006-06-03 10:39:29 +0200 (sam., 03 juin 2006) $
 */

#include <ctype.h>
#include "useful.h"

char *strupper(char *s)
{
  char *p = s;

  while ( *p != NUL ) {
    *p = toupper(*p);
    p++;
  }

  return s;
}
