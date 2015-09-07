/****************************************************************************/
/* TestFarm                                                                 */
/* Graphical User Interface - Stdout/Stderr pipes management                */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 14-DEC-1999                                                    */
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

#ifndef __TESTFARM_GUI_PIPE_H__
#define __TESTFARM_GUI_PIPE_H__

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

#endif /* __TESTFARM_GUI_PIPE_H__ */
