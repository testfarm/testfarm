/****************************************************************************/
/* TestFarm                                                                 */
/* Graphical User Interface - Stdout/Stderr pipes management                */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 14-DEC-1999                                                    */
/****************************************************************************/

/*
 * $Revision: 42 $
 * $Date: 2006-06-03 15:30:01 +0200 (sam., 03 juin 2006) $
 */

#ifndef __GUI_PIPE_H__
#define __GUI_PIPE_H__

typedef struct pipe_s pipe_t;

typedef void pipe_notify_t(pipe_t *p, void *user_data);

struct pipe_s {
  int flags;
  pipe_notify_t *notify;
  void *user_data;

  int fds[2];
  int tag;
  char *buf;
  long size;
  long index;
};

extern pipe_t *pipe_init(int flags);
extern void pipe_done(pipe_t *p);
extern void pipe_input(pipe_t *pipe);
extern int pipe_fd(pipe_t *p);
extern void pipe_notify(pipe_t *p, pipe_notify_t *notify, void *user_data);

#endif /* __GUI_PIPE_H__ */
