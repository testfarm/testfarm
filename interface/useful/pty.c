/****************************************************************************/
/* TestFarm                                                                 */
/* Test Engine - Pseudo-terminal management                                 */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 22-NOV-1999                                                    */
/****************************************************************************/

/*
 * $Revision: 29 $
 * $Date: 2006-06-03 10:39:29 +0200 (sam., 03 juin 2006) $
 */

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <fcntl.h>
#include <unistd.h>

#include "pty.h"


pty_t *pty_open(void)
{
  static char tty_letter[] = "pqr";
  static char tty_digit[] = "0123456789";
  pty_t *pty;
  int i;
  int l, d;

  /* Alloc pty descriptor */
  pty = (pty_t *) malloc(sizeof(pty_t));

  /* Open pseudo-terminal device */
  i = -1; l = 0; d = 0;
  while ( (l < sizeof(tty_letter)) && (i == -1) ) {
    while ( (d < sizeof(tty_digit)) && (i == -1) ) {
      sprintf(pty->pty, "%s%c%c", PTY_NAME_PTY, tty_letter[l], tty_digit[d]);
      i = open(pty->pty, O_RDWR);
      if ( i == -1 )
	d++;
    }
    if ( i == -1 )
      l++;
  }

  if ( i == -1 ) {
    free(pty);
    return NULL;
  }

  sprintf(pty->tty, "%s%c%c", PTY_NAME_TTY, tty_letter[l], tty_digit[d]);
  pty->fd = i;

  return pty;
}


void pty_close(pty_t *pty)
{
  if ( pty == NULL )
    return;

  if ( pty->fd != -1 )
    close(pty->fd);

  free(pty);
}
