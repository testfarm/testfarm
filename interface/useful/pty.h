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

#ifndef __PTY_H__
#define __PTY_H__

/* Pseudo-terminal configuration */
#define PTY_NAME_LEN  10
#define PTY_NAME_PTY  "/dev/pty"
#define PTY_NAME_TTY  "/dev/tty"
#define PTY_NAME_NULL "/dev/null"

typedef struct {
  char pty[PTY_NAME_LEN+1]; /* Master pseudo-tty name */
  char tty[PTY_NAME_LEN+1]; /* Slave pseudo-tty name */
  int fd;                   /* Master pseudo-tty filedesc */
} pty_t;

extern pty_t *pty_open(void);
extern void pty_close(pty_t *pty);

#endif /* __PTY_H__ */
