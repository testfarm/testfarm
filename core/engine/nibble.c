/****************************************************************************/
/* TestFarm                                                                 */
/* Test Engine - Nibble Allocation                                          */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 18-NOV-1999                                                    */
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
