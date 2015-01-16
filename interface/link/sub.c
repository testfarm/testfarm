/****************************************************************************/
/* TestFarm                                                                 */
/* Data Logger Interface :            subprocesses management               */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 28-MAR-2000                                                    */
/****************************************************************************/

/*
 * $Revision: 1220 $
 * $Date: 2011-12-04 17:01:50 +0100 (dim., 04 déc. 2011) $
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>

#ifdef ENABLE_GLIB
#include <glib.h>
#endif

#include "child.h"
#include "link.h"
#include "sub.h"


/*******************************************************/
/* Subprocess item                                     */
/*******************************************************/

typedef struct {
  int num;
  char *cmd;
  child_t *child;
  int stdin_fd;
  int stdout_fd;
} sub_item_t;


static void sub_sigchld(int status, sub_item_t *sub);


static void sub_item_zero(sub_item_t *sub)
{
  sub->cmd = NULL;
  sub->child = NULL;
  sub->stdin_fd = -1;
  sub->stdout_fd = -1;
}


static int sub_item_spawn(sub_item_t *sub, char **argv, char **errmsg)
{
  int stdin_pipe[2];
  int stdout_pipe[2];
  child_t *child;
  int size;

  /* Check access to command */
  if ( access(argv[0], X_OK) == -1 ) {
    *errmsg = "Cannot access command for execution";
    return -1;
  }

  /* Create standard input pipe */
  if ( pipe(stdin_pipe) == -1 ) {
    *errmsg = "Cannot create standard input pipe";
    return -1;
  }

  /* Create standard output pipe */
  if ( pipe(stdout_pipe) == -1 ) {
    close(stdin_pipe[0]);
    close(stdin_pipe[1]);
    *errmsg = "Cannot create standard output pipe";
    return -1;
  }

  /* Enable close-on-exec mode on local pipe endpoints */
  fcntl(stdin_pipe[1], F_SETFD, FD_CLOEXEC);
  fcntl(stdout_pipe[0], F_SETFD, FD_CLOEXEC);

  /* Create process */
  child = child_spawn(argv, stdin_pipe[0], stdout_pipe[1], -1, (child_handler_t *) sub_sigchld, sub);

  if ( child == NULL ) {
    close(stdin_pipe[0]);
    close(stdin_pipe[1]);
    close(stdout_pipe[0]);
    close(stdout_pipe[1]);
    *errmsg = "Cannot fork subprocess";
    return -1;
  }

  /* Feed command line */
  sub->cmd = NULL;
  size = 0;
  while ( *argv != NULL ) {
    char *str = *argv;
    int len = strlen(str);

    sub->cmd = (char *) realloc(sub->cmd, size+len+2);
    strcpy(&((sub->cmd)[size]), str);
    size += len;

    argv++;
    if ( *argv != NULL )
      sub->cmd[size++] = ' ';
  };

  /* Set child descriptor */
  sub->child = child;

  /* Feed standard i/o descriptors */
  close(stdin_pipe[0]);
  sub->stdin_fd = stdin_pipe[1];
  //fcntl(sub->stdin_fd, F_SETFL, O_NONBLOCK);

  close(stdout_pipe[1]);
  sub->stdout_fd = stdout_pipe[0];
  fcntl(sub->stdout_fd, F_SETFL, O_NONBLOCK);

  return 0;
}


static void sub_item_kill(sub_item_t *sub)
{
  child_t *child;

  if ( sub == NULL )
    return;

  child = sub->child;
  child_handler(child, NULL, NULL);
  sub->child = NULL;
  child_kill(child, SIGTERM);

  if ( sub->cmd != NULL )
    free(sub->cmd);
  if ( sub->stdin_fd != -1 )
    close(sub->stdin_fd);
  if ( sub->stdout_fd != -1 )
    close(sub->stdout_fd);

  sub_item_zero(sub);
}


/*******************************************************/
/* Subprocess descriptors                              */
/*******************************************************/

#define SUB_N 8

static sub_item_t sub_tab[SUB_N];
static int sub_ctrl[2];

#ifdef ENABLE_GLIB
static GIOChannel *sub_ctrl_channel = NULL;
static guint sub_ctrl_tag = 0;
#endif /* ENABLE_GLIB */


static int sub_ctrl_read(void)
{
  int num;
  int size;
  int ret = -1;

  size = read(sub_ctrl[0], &num, sizeof(num));

  if ( size == sizeof(num) ) {
    if ( (num >= 0) && (num < SUB_N) ) {
      sub_item_t *sub = &(sub_tab[num]);

      sub->child = NULL;
      ret = 0;
    }
    else {
      fprintf(stderr, "*PANIC* Bad index received from subprocess control pipe\n");
    }
  }
  else if ( size < 0 ) {
    fprintf(stderr, "*PANIC* Error reading subprocess control pipe: %s\n", strerror(errno));
  }
  else {
    fprintf(stderr, "*PANIC* Bad data received from subprocess control pipe\n");
  }

  return ret;
}


#ifdef ENABLE_GLIB

static gboolean sub_ctrl_read_event(void)
{
  sub_ctrl_read();
  return TRUE;
}

#endif /* ENABLE_GLIB */


/*******************************************************/
/* Subprocess i/o multiplexing                         */
/*******************************************************/

#ifdef USE_SELECT

static void sub_fd_set(link_item_t *link, link_fds_t *lfds)
{
  /* Add control pipe */
  if ( sub_ctrl[0] != -1 ) {
    FD_SET(sub_ctrl[0], lfds->rd);
    if ( sub_ctrl[0] > lfds->max )
      lfds->max = sub_ctrl[0];
  }

  FD_SET(link->fd_in, lfds->rd);

  if ( link->fd_in > lfds->max )
    lfds->max = link->fd_in;
}


static int sub_fd_isset(link_item_t *link, fd_set *fds)
{
  sub_item_t *sub = link->ptr;

  if ( sub == NULL )
    return -1;

  /* From subprocess standard output (high priority) */
  if ( FD_ISSET(link->fd_in, fds) ) {
#ifdef __DEBUG__
    fprintf(stderr, "[DEBUG] Something to read from subprocess stdout\n");
#endif
    return link->fd_in;
  }

  /* From control pipe (low priority) */
  if ( FD_ISSET(sub_ctrl[0], fds) ) {
#ifdef __DEBUG__
    fprintf(stderr, "[DEBUG] Something to read from subprocess control pipe\n");
#endif
    if ( sub_ctrl_read() )
      return 0;
  }

  return -1;
}

#endif /* USE_SELECT */


/*******************************************************/
/* Subprocess create/destroy                           */
/*******************************************************/

static void sub_kill(link_item_t *link)
{
  if ( link->ptr != NULL )
    sub_item_kill(link->ptr);
}


static char *sub_addr(link_item_t *link)
{
  sub_item_t *sub = link->ptr;
  char buf[256];
  int len = 0;

  if ( sub->cmd != NULL )
    len += snprintf(buf+len, sizeof(buf)-len, "%s", sub->cmd);
  if ( sub->child != NULL )
    len += snprintf(buf+len, sizeof(buf)-len, " [%d]", sub->child->pid);

  buf[len] = '\0';
  return strdup(buf);
}


static int sub_connect(link_item_t *link, char **argv, char **errmsg)
{
  sub_item_t *sub = NULL;
  int i;

  /* Find available entry */
  for (i = 0; (i < SUB_N) && (sub == NULL); i++) {
    sub = &(sub_tab[i]);
    if ( sub->child != NULL )
      sub = NULL;
  }

  /* No space left for new subprocess */
  if ( sub == NULL ) {
    *errmsg = "No space left for new subprocess";
    return -1;
  }

  if ( sub_item_spawn(sub, argv, errmsg) )
    return -1;

  link->fd_in = sub->stdout_fd;
  link->fd_out = sub->stdin_fd;
  link->ptr = sub;

  return 0;
}


/*******************************************************/
/* Subprocess SIGCHLD handling                         */
/*******************************************************/

static void sub_sigchld(int status, sub_item_t *sub)
{
  /* Report child death through control pipe */
  if ( sub != NULL ) {
    write(sub_ctrl[1], &(sub->num), sizeof(sub->num));
  }
}


/*******************************************************/
/* Subprocess class initialization                     */
/*******************************************************/

static link_class_t sub_lclass = {
  method : "proc",
  addr : sub_addr,
#ifdef USE_SELECT
  fd_set : sub_fd_set,
  fd_isset : sub_fd_isset,
#endif
  connect : sub_connect,
  kill : sub_kill,
};


int sub_init(void)
{
  int i;

  /* Clear table of subprocesses */
  for (i = 0; i < SUB_N; i++) {
    sub_item_zero(&(sub_tab[i]));
    sub_tab[i].num = i;
  }

  /* Create control pipe */
  if ( pipe(sub_ctrl) == -1 )
    return -1;

  /* Control pipe read endpoint is non-blocking */
  fcntl(sub_ctrl[0], F_SETFL, O_NONBLOCK);

  /* Control pipe is not accessible from exec() subprocesses */
  fcntl(sub_ctrl[0], F_SETFD, FD_CLOEXEC);
  fcntl(sub_ctrl[1], F_SETFD, FD_CLOEXEC);

  /* Setup child process management */
  child_init();

  /* Register link class */
  link_class_register(&sub_lclass);

  return 0;
}


void sub_done(void)
{
  int i;

#ifdef ENABLE_GLIB
  /* Halt control pipe event handling */
  if ( sub_ctrl_tag > 0 ) {
    g_source_remove(sub_ctrl_tag);
    sub_ctrl_tag = 0;
  }

  if ( sub_ctrl_channel != NULL ) {
    g_io_channel_unref(sub_ctrl_channel);
    sub_ctrl_channel = NULL;
  }
#endif

  /* Kill running subprocesses */
  for (i = 0; i < SUB_N; i++)
    sub_item_kill(&(sub_tab[i]));

  /* Close control pipe */
  close(sub_ctrl[0]);
  close(sub_ctrl[1]);
  sub_ctrl[0] = sub_ctrl[1] = -1;

  /* Detach signal handler */
  signal(SIGCHLD, SIG_IGN);
}


#ifdef ENABLE_GLIB

void sub_use_glib(void)
{
  /* Setup control pipe event handling */
  sub_ctrl_channel = g_io_channel_unix_new(sub_ctrl[0]);
  sub_ctrl_tag = g_io_add_watch(sub_ctrl_channel, G_IO_IN,
				(GIOFunc) sub_ctrl_read_event, NULL);
}

#endif
