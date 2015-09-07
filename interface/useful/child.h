/****************************************************************************/
/* TestFarm                                                                 */
/* Child processes management                                               */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 17-DEC-1999                                                    */
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

#ifndef __CHILD_H__
#define __CHILD_H__

#include <unistd.h>
#include <sys/types.h>

typedef void child_handler_t(int status, void *arg);

typedef struct {
  pid_t pid;
  child_handler_t *handler;
  void *arg;
} child_t;

extern int child_init(void);
extern void child_done(void);

#ifdef ENABLE_GLIB
extern int child_use_glib(void);
#endif

extern child_t *child_spawn(char *argv[], int fd_in, int fd_out, int fd_err,
                            child_handler_t *handler, void *arg);
extern pid_t child_handler(child_t * child, child_handler_t *handler, void *arg);
extern int child_kill(child_t *child, int sig);
extern int child_waitpid(child_t *child, int *status);
extern void child_terminate(child_t *child);

#endif /* __CHILD_H__ */
