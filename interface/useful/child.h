/****************************************************************************/
/* TestFarm                                                                 */
/* Child processes management                                               */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 17-DEC-1999                                                    */
/****************************************************************************/

/*
 * $Revision: 1218 $
 * $Date: 2011-12-04 16:31:17 +0100 (dim., 04 déc. 2011) $
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
