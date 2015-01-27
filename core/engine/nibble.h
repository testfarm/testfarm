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

#ifndef __ENGINE_NIBBLE_H__
#define __ENGINE_NIBBLE_H__

#define NIBBLE_SIZE 80

typedef struct {
  int size;
  char *buf;
} nibble_t;

extern nibble_t *nibble_realloc(nibble_t *nibble, int size);
extern void nibble_free(nibble_t *nibble);

#endif /* __ENGINE_NIBBLE_H__ */
