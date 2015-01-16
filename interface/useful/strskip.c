/****************************************************************************/
/* TestFarm                                                                 */
/* Leading characters skip utility                                          */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 17-AUG-1999                                                    */
/****************************************************************************/

/*
 * $Revision: 29 $
 * $Date: 2006-06-03 10:39:29 +0200 (sam., 03 juin 2006) $
 */

#include "useful.h"

char *strskip_spaces(char *s)
{
  while ( (*s != NUL) && (*s <= ' ') )
    s++;
  return s;
}


char *strskip_chars(char *s)
{
  while ( *s > ' ' )
    s++;
  return s;
}
