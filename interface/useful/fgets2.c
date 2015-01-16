/****************************************************************************/
/* TestFarm                                                                 */
/* Allocate + Read line buffer from file stream                             */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 22-APR-2004                                                    */
/****************************************************************************/

/*
 * $Revision: 29 $
 * $Date: 2006-06-03 10:39:29 +0200 (sam., 03 juin 2006) $
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
