/****************************************************************************/
/* TestFarm                                                                 */
/* Test Engine - Nibble Allocation                                          */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 18-NOV-1999                                                    */
/****************************************************************************/

/*
 * $Revision: 42 $
 * $Date: 2006-06-03 15:30:01 +0200 (sam., 03 juin 2006) $
 */

#include <stdio.h>
#include <malloc.h>

#define NIBBLE_SIZE 80

typedef struct {
  int size;
  char *buf;
} nibble_t;

nibble_t *nibble_realloc(nibble_t *nibble, int size)
{
  if ( nibble == NULL ) {
    nibble = (nibble_t *) malloc(sizeof(nibble_t));
    nibble->size = 0;
    nibble->buf = NULL;
  }

  if ( nibble->size < size ) {
    while ( nibble->size < size )
      nibble->size += NIBBLE_SIZE;
    nibble->buf = realloc(nibble->buf, nibble->size);
  }

  return nibble;
}


void nibble_free(nibble_t *nibble)
{
  if ( nibble == NULL )
    return;

  if ( nibble->buf != NULL )
    free(nibble->buf);

  free(nibble);
}
