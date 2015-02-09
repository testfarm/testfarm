/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* External Agent management                                                */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 08-DEC-2007                                                    */
/****************************************************************************/

/*
 * $Revision: 1153 $
 * $Date: 2010-06-06 11:36:58 +0200 (dim., 06 juin 2010) $
 */

#define __TVU_AGENT_C__

#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <errno.h>
#include <fcntl.h>
#include <glib.h>

#include "child.h"
#include "user_dir.h"

typedef struct agent_s agent_t;

#include "agent.h"

#define eprintf(args...) fprintf(stderr, "testfarm-vu (agent): " args)


struct agent_s {
  child_t *child;           /* Agent child process */
  int in_fd;                /* Agent communication channels */
  int out_fd;
  GIOChannel *out_channel;
  guint out_tag;
  agent_callback_t *callback;
  void *ctx;
};


static void agent_terminate(int status, agent_t *agent)
{
  if ( agent->out_tag > 0 ) {
    g_source_remove(agent->out_tag);
    agent->out_tag = 0;
  }

  if ( agent->out_channel != NULL ) {
    g_io_channel_unref(agent->out_channel);
    agent->out_channel = NULL;
  }

  if ( agent->in_fd > 0 ) {
    close(agent->in_fd);
    agent->in_fd = -1;
  }

  if ( agent->out_fd > 0 ) {
    close(agent->out_fd);
    agent->out_fd = -1;
  }

  if ( agent->child != NULL ) {
    child_handler(agent->child, NULL, NULL);
    child_terminate(agent->child);
    agent->child = NULL;
  }

  if ( agent->callback != NULL ) {
    agent->callback(agent->ctx, NULL, 0);
  }
}


static gboolean agent_event_read(GIOChannel *source, GIOCondition condition, agent_t *agent)
{
  /* Read data from link connection */
  if ( condition & G_IO_IN ) {
    GIOStatus status;
    GError *error = NULL;
    gchar *buf;
    gsize size;

    while ( (status = g_io_channel_read_line(source, &buf, &size, NULL, &error)) == G_IO_STATUS_NORMAL ) {
      if ( buf != NULL ) {
	if ( agent->callback != NULL ) {
	  agent->callback(agent->ctx, buf, size);
	}

	g_free(buf);
      }

      /* Give-up if process has been killed during callback processing */
      if (agent->out_channel == NULL)
	      break;
    }

    if ( status == G_IO_STATUS_ERROR ) {
      eprintf("read error (fd=%d): %s\n", agent->out_fd, error->message);
      condition |= G_IO_HUP;
    }
  }

  /* Check for remote disconnection */
  if ( condition & G_IO_HUP ) {
    agent_terminate(0, agent);
    return FALSE;
  }

  return TRUE;
}


agent_t *agent_create(char *argv[], agent_callback_t *callback, void *ctx)
{
  int stdin_pipe[2];
  int stdout_pipe[2];
  int stderr_fd;
  FILE *f;
  char *basename, *logname, *suffix, *logpath;
  int len;
  agent_t *agent;

  /* Create standard input pipe */
  if ( pipe(stdin_pipe) == -1 ) {
    eprintf("Cannot create input pipe: %s\n", strerror(errno));
    return NULL;
  }

  /* Create standard output pipe */
  if ( pipe(stdout_pipe) == -1 ) {
    close(stdin_pipe[0]);
    close(stdin_pipe[1]);
    eprintf("Cannot create output pipe: %s\n", strerror(errno));
    return NULL;
  }

  /* Enable close-on-exec mode on local pipe endpoints */
  fcntl(stdin_pipe[1], F_SETFD, FD_CLOEXEC);
  fcntl(stdout_pipe[0], F_SETFD, FD_CLOEXEC);

  /* Alloc agent descriptor */
  agent = (agent_t *) malloc(sizeof(agent_t));
  memset(agent, 0, sizeof(agent_t));
  agent->callback = callback;
  agent->ctx = ctx;

  /* Create standard error log */
  basename = g_path_get_basename(argv[0]);
  if ( (suffix = strchr(basename, '.')) != NULL )  /* Strip pgm suffix */
    *suffix = '\0';
  len = strlen(basename);
  logname = (char *) malloc(len+5);
  strcpy(logname, basename);
  strcpy(logname+len, ".log");
  logpath = user_dir("log", logname);

  f = fopen(logpath, "w");
  if ( f == NULL ) {
    eprintf("*WARNING* Cannot open error log file '%s': %s\n", logpath, strerror(errno));
    stderr_fd = -1;
  }
  else {
    stderr_fd = fileno(f);
    if ( stderr_fd == -1 ) {
      eprintf("*WARNING* Cannot get error log file descriptor: %s\n", strerror(errno));
    }
  }

  free(logpath);
  free(logname);
  free(basename);

  /* Spawn agent subprocess */
  agent->child = child_spawn(argv, stdin_pipe[0], stdout_pipe[1], stderr_fd,
			     (child_handler_t *) agent_terminate, agent);

  /* Close useless file descriptors (only used by agent subprocess) */
  close(stdin_pipe[0]);
  close(stdout_pipe[1]);
  if ( f != NULL )
    fclose(f);

  if ( agent->child == NULL ) {
    close(stdin_pipe[1]);
    close(stdout_pipe[0]);
    return NULL;
  }

  /* Setup stdio endpoints */
  agent->in_fd = stdin_pipe[1];

  agent->out_fd = stdout_pipe[0];
  fcntl(agent->out_fd, F_SETFL, O_NONBLOCK);
  agent->out_channel = g_io_channel_unix_new(agent->out_fd);
  agent->out_tag = g_io_add_watch(agent->out_channel, G_IO_IN | G_IO_HUP,
			       (GIOFunc) agent_event_read, agent);

  return agent;
}


void agent_destroy(agent_t *agent)
{
  /* Clear callback */
  agent->callback = NULL;
  agent->ctx = NULL;

  /* Kill agent subprocess */
  agent_terminate(0, agent);
}


int agent_request(agent_t *agent, char *buf, int size)
{
  int ret;

  ret = write(agent->in_fd, buf, size);
  if ( ret == -1 ) {
    eprintf("write error (fd=%d): %s\n", agent->in_fd, strerror(errno));
    agent_terminate(0, agent);
  }

  return ret;
}
