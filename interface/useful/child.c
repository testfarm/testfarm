/****************************************************************************/
/* TestFarm                                                                 */
/* General Library - Child processes management                             */
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <wait.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>

#include "child.h"

#define CHILD_NB 8

#define dprintf(args...) //fprintf(stderr, "[CHILD] " args);

static child_t child_tab[CHILD_NB];

#ifdef ENABLE_GLIB

#include <glib.h>

static int child_ctrl[2] = {-1, -1};
static GIOChannel *child_ctrl_channel = NULL;
static guint child_ctrl_tag = 0;

typedef struct {
  unsigned char index;
  int status;
} child_ctrl_msg_t;


static gboolean child_event_ctrl(GIOChannel *source, GIOCondition condition, void *data)
{
  child_ctrl_msg_t msg = { CHILD_NB, 0 };

  if ( condition & G_IO_IN ) {
    int ret = read(child_ctrl[0], &msg, sizeof(msg));
    if ( ret <= 0 ) {
      if ( ret < 0 )
	fprintf(stderr, "** Child signal: Error reading control pipe: %s", strerror(errno));

      condition |= G_IO_HUP;
    }
  }

  if ( msg.index < CHILD_NB ) {
    child_t *child = &(child_tab[msg.index]);
    if ( child->handler != NULL )
      child->handler(msg.status, child->arg);
  }

  if ( condition & G_IO_HUP )
    return FALSE;

  return TRUE;
}


int child_use_glib(void)
{
  /* Create control pipe */
  if ( pipe(child_ctrl) == -1 ) {
    fprintf(stderr, "** Child pipe: Cannot create control pipe: %s", strerror(errno));
    return -1;
  }

  /* Enable close-on-exec mode on control pipe endpoints */
  fcntl(child_ctrl[0], F_SETFD, FD_CLOEXEC);
  fcntl(child_ctrl[1], F_SETFD, FD_CLOEXEC);

  /* Enable non-blocking mode on control pipe read endpoint */
  fcntl(child_ctrl[0], F_SETFL, O_NONBLOCK);

  /* Setup control pipe read event handler */
  child_ctrl_channel = g_io_channel_unix_new(child_ctrl[0]);
  child_ctrl_tag = g_io_add_watch(child_ctrl_channel, G_IO_IN | G_IO_HUP,
				  (GIOFunc) child_event_ctrl, NULL);

  return 0;
}

#endif /* ENABLE_GLIB */


static void child_sigchld(int sig)
{
  pid_t pid;
  int status;
  child_t *child;

  /* Acknowledge child death */
  while ( (pid = waitpid(-1, &status, WNOHANG)) > 0 ) {
    unsigned char index;

    /* Retrieve child entry */
    child = NULL;
    for (index = 0; (index < CHILD_NB) && (child == NULL); index++) {
      child = &(child_tab[index]);
      if ( child->pid != pid )
	child = NULL;
    }

    if ( child != NULL ) {
      child->pid = -1;

#ifdef ENABLE_GLIB
      if ( child_ctrl[1] > 0 ) {
	child_ctrl_msg_t msg = { index, status };
	write(child_ctrl[1], &msg, sizeof(msg));
      }
      else {
	if ( child->handler != NULL )
	  child->handler(status, child->arg);
      }
#else
      if ( child->handler != NULL )
        child->handler(status, child->arg);
#endif
    }
  }
}


int child_init(void)
{
  struct sigaction act;
  int i;

  /* Clear child table */
  for (i = 0; i < CHILD_NB; i++) {
    child_tab[i].pid = -1;
    child_tab[i].handler = NULL;
    child_tab[i].arg = NULL;
  }

  /* Setup SIGCHLD signal handling */
  sigemptyset(&act.sa_mask);
  act.sa_flags = SA_RESTART;
  act.sa_handler = child_sigchld;
  sigaction(SIGCHLD, &act, NULL);

  return 0;
}


child_t *child_spawn(char *argv[], int fd_in, int fd_out, int fd_err,
                     child_handler_t *handler, void *arg)
{
  child_t *child;
  pid_t pid;
  int i;

  /* Find a free child entry */
  child = NULL;
  for (i = 0; (i < CHILD_NB) && (child== NULL); i++) {
    child = &(child_tab[i]);
    if ( child->pid >= 0 )
      child = NULL;
  }

  if ( child == NULL ) {
    fprintf(stderr, "** Child table is full\n");
    return NULL;
  }

  /* Spawn child */
  switch ( (pid = fork()) ) {
  case 0: /* Child */
    /* Redirect standard input */
    if ( fd_in >= 0 ) {
      if ( dup2(fd_in, STDIN_FILENO) == -1 ) {
        fprintf(stderr, "** Child dup2(stdin): %s (pid=%d): %s\n", argv[0], getpid(), strerror(errno));
        exit(254);
      }
      close(fd_in);
    }

    /* Redirect standard output */
    if ( fd_out >= 0 ) {
      if ( dup2(fd_out, STDOUT_FILENO) == -1 ) {
        fprintf(stderr, "** Child dup2(stdout): %s (pid=%d): %s\n", argv[0], getpid(), strerror(errno));
        exit(254);
      }
      close(fd_out);
    }

    /* Redirect standard error output */
    if ( fd_err >= 0 ) {
      if ( dup2(fd_err, STDERR_FILENO) == -1 ) {
        fprintf(stderr, "** Child dup2(stderr): %s (pid=%d): %s\n", argv[0], getpid(), strerror(errno));
        exit(254);
      }
      close(fd_err);
    }

    /* Perform exec (returnless call if success) */
    execvp(argv[0], argv);

    /* Return from exec: something went wrong */
    fprintf(stderr, "** Child execvp: %s (pid=%d): %s\n", argv[0], getpid(), strerror(errno));
    exit(255);
    break;

  case -1: /* Error */
    perror("** Child fork");
    return NULL;
    break;

  default : /* Parent */
    child->pid = pid;
    child->handler = handler;
    child->arg = arg;
    break;
  }

  return child;
}


pid_t child_handler(child_t * child, child_handler_t *handler, void *arg)
{
  if ( child == NULL )
    return -1;

  child->handler = handler;
  child->arg = arg;

  return child->pid;
}


int child_kill(child_t *child, int sig)
{
  if ( child == NULL )
    return -1;
  if ( child->pid < 0 )
    return -1;
  if ( kill(child->pid, sig) )
    return -1;

  /* Child death should be caught by SIGCHLD handler
     or awaited by child_waitpid() */

  return 0;
}


static void child_sigalrm(int sig)
{
  dprintf("SIGALRM caught\n");
}


int child_waitpid(child_t *child, int *status)
{
  struct sigaction act;
  struct sigaction oldact;
  pid_t pid0, pid;
  int ret;

  if ( child == NULL )
    return 0;

  /* Return now if child death has already been caught */
  if ( child->pid <= 0 )
    return 0;

  /* Check whether the child is not just going down */
  pid0 = child->pid;
  pid = waitpid(pid0, status, WNOHANG);
  if ( pid != 0 ) {
    if ( pid < 0 )
      dprintf("waitpid(%d): %s\n", pid0, strerror(errno));
    return 0;
  }

  dprintf("waitpid(%d): Not yet terminated - Waiting.\n", pid0);

  /* Setup SIGALRM signal handling */
  sigemptyset(&act.sa_mask);
  act.sa_flags = 0;
  act.sa_handler = child_sigalrm;
  sigaction(SIGALRM, &act, &oldact);

  /* Setup soft kill timeout */
  alarm(2);

  /* Killing me softly... */
  pid = waitpid(pid0, status, 0);
  if ( pid == pid0 )
    child->pid = -1;
  if ( pid < 0 ) {
    dprintf("waitpid(%d): %s\n", pid0, strerror(errno));
  }

  /* If soft kill does not seem to work, kill violently */
  ret = 0;
  if ( (pid == -1) && (errno == EINTR) ) {
    ret = -1;
    dprintf("waitpid(%d) timed out\n", pid0);
  }

  /* Restore SIGALRM signal handling */
  alarm(0);
  sigaction(SIGALRM, &oldact, NULL);

  return ret;
}


void child_terminate(child_t *child)
{
  if ( child == NULL )
    return;
  if ( child->pid <= 0 )
    return;

  child_handler(child, NULL, NULL);

  dprintf("Sending signal SIGTERM to child process %d\n", child->pid);
  child_kill(child, SIGTERM);

  if ( child_waitpid(child, NULL) ) {
    fprintf(stderr, "** Process %d termination failed: killing it.\n", child->pid);
    child_kill(child, SIGKILL);
  }
}


void child_done(void)
{
  int i;

  /* Destroy control pipe */
#ifdef ENABLE_GLIB
  if ( child_ctrl_tag > 0 ) {
    g_source_remove(child_ctrl_tag);
    child_ctrl_tag = 0;
  }

  if ( child_ctrl_channel != NULL ) {
    g_io_channel_unref(child_ctrl_channel);
    child_ctrl_channel = NULL;
  }

  if ( child_ctrl[0] > 0 ) {
    close(child_ctrl[0]);
    child_ctrl[0] = -1;
  }
  if ( child_ctrl[1] > 0 ) {
    close(child_ctrl[1]);
    child_ctrl[1] = -1;
  }
#endif

  /* Kill all child processes */
  for (i = 0; i < CHILD_NB; i++)
    child_kill(&(child_tab[i]), SIGTERM);
  for (i = 0; i < CHILD_NB; i++)
    child_waitpid(&(child_tab[i]), NULL);
  for (i = 0; i < CHILD_NB; i++)
    child_kill(&(child_tab[i]), SIGKILL);
}
