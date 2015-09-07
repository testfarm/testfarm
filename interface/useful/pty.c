/****************************************************************************/
/* TestFarm                                                                 */
/* Test Engine - Pseudo-terminal management                                 */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 22-NOV-1999                                                    */
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
