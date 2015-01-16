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
