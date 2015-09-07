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

#include <stdio.h>
#include <malloc.h>
#include <unistd.h>
#include <fcntl.h>
#include <gtk/gtk.h>

#include "useful.h"
#include "pipe.h"

#define PIPE_NIBBLE_SIZE 256
#define LF '\n'

static void pipe_input_guts(pipe_t *pipe, char *buf, long size)
{
  while ( size > 0 ) {
    char c = *(buf++);
    size--;

    /* Grow buffer by one nibble if necessary */
    if ( pipe->index >= (pipe->size-1) ) {
      pipe->size += PIPE_NIBBLE_SIZE;
      pipe->buf = realloc(pipe->buf, pipe->size);
    }

    /* If end of line encountered, transfer data to target buffer... */
    if ( c == LF ) {
      /* Terminate buffer with a NUL character */
      pipe->buf[pipe->index] = NUL;

      /* Notify event */
      if ( pipe->notify != NULL ) {
        pipe->notify(pipe, pipe->user_data);
      }

      /* Clear i/o buffer indexing */
      pipe->index = 0;
    }
    else {
      pipe->buf[(pipe->index)++] = c;
    }
  }
}


void pipe_input(pipe_t *pipe)
{
  char buf[PIPE_NIBBLE_SIZE];
  int size;

  if ( pipe == NULL )
    return;

  while ( (size = read(pipe->fds[0], buf, sizeof(buf))) > 0 )
    pipe_input_guts(pipe, buf, size);
}


static void pipe_input_handler(pipe_t *pipe, int fd, GdkInputCondition condition)
{
  pipe_input(pipe);
}


void pipe_notify(pipe_t *p, pipe_notify_t *notify, void *user_data)
{
  if ( p == NULL )
    return;

  p->notify = notify;
  p->user_data = user_data;
}


pipe_t *pipe_init(int flags)
{
  pipe_t *p = (pipe_t *) malloc(sizeof(pipe_t));

  p->flags = flags;
  p->notify = NULL;
  p->user_data = NULL;
  p->tag = -1;
  p->buf = NULL;
  p->size = 0;
  p->index = 0;

  if ( pipe(p->fds) ) {
    p->fds[0] = p->fds[1] = -1;
    fprintf(stderr, "Cannot open redirection pipe\n");
  }

  if ( p->fds[0] != -1 ) {
    fcntl(p->fds[0], F_SETFD, FD_CLOEXEC);
    fcntl(p->fds[0], F_SETFL, O_NONBLOCK);
    p->tag = gdk_input_add(p->fds[0], GDK_INPUT_READ,
                           (GdkInputFunction) pipe_input_handler, p);
  }

  return p;
}


void pipe_done(pipe_t *p)
{
  if ( p == NULL )
    return;

  if ( p->tag != -1 ) {
    gdk_input_remove(p->tag);
    p->tag = -1;
  }

  if ( p->fds[0] != -1 ) {
    close(p->fds[0]);
    p->fds[0] = -1;
  }

  if ( p->fds[1] != -1 ) {
    close(p->fds[1]);
    p->fds[1] = -1;
  }

  if ( p->buf != NULL ) {
    free(p->buf);
    p->buf = NULL;
  }

  free(p);
}


int pipe_fd(pipe_t *p)
{
  return (p == NULL) ? -1 : p->fds[1];
}
