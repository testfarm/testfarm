/****************************************************************************/
/* TestFarm                                                                 */
/* Shell Script Interpreter Library                                         */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 26-AUG-1999                                                    */
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
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>

#include "useful.h"
#include "shell_exec.h"


#ifdef ENABLE_GLIB
static int shell_exec_read_watch(shell_t *shell);


void shell_use_glib(shell_t *shell, GMainLoop *loop)
{
  shell->use_glib = TRUE;
  shell->glib_loop = loop;
}
#endif

int shell_get_fd(shell_t *shell)
{
#ifdef ENABLE_GLIB
  if ( shell->io_channel == NULL )
    return -1;
  return g_io_channel_unix_get_fd(shell->io_channel);
#else
  return shell->fd;
#endif
}


shell_t *shell_alloc(char *name, int argc, char **argv, shell_t *parent)
{
  shell_t *shell;
  int i;

  shell = (shell_t *) malloc(sizeof(shell_t));
  shell->name = (name != NULL) ? strdup(name) : NULL;
  shell->file = NULL;
  if ( (argv != NULL) && (argv[0] != NULL ) )
    shell->file = strdup(argv[0]);
  shell->line = 0;
  shell->parent = parent;

#ifdef ENABLE_GLIB
  shell->io_channel = NULL;
  shell->io_tag = 0;

  shell->sleep_tag = 0;
  shell->paused = 0;

#ifdef ENABLE_CMD_SOURCE
  shell->source_shell = NULL;
  shell->source_pipe[0] = shell->source_pipe[1] = -1;
  shell->source_channel = NULL;
  shell->source_done_fn = NULL;
  shell->source_done_arg = NULL;
#endif
#else  /* ENABLE_GLIB */
  shell->fd = -1;
#endif /* !ENABLE_GLIB */

  if ( parent == NULL ) {
    /* No parent => Nothing to inherit from */
    shell->shell_argv = shell_argv_allocv(argc, argv, NULL);
#ifdef ENABLE_CMD_ALIAS
    shell->shell_alias = shell_alias_init(NULL);
#endif
#ifdef ENABLE_CMD_HISTORY
    shell_history_init(&shell->history);
#endif
    shell->input_echo_handler = NULL;
    shell->input_echo_tag = NULL;
    for (i = 0; i < SHELL_HASH_SIZE; i++)
      shell->cmd_hash[i] = NULL;
    shell->prologue_handler = NULL;
    shell->prologue_arg = NULL;
    shell->unknown_handler = NULL;
    shell->unknown_tag = NULL;
  }
  else {
    /* Parent => Inherit from it */
    shell->shell_argv = shell_argv_allocv(argc, argv, parent->shell_argv);
#ifdef ENABLE_CMD_ALIAS
    shell->shell_alias = shell_alias_init(parent->shell_alias);
#endif
    shell->input_echo_handler = parent->input_echo_handler;
    shell->input_echo_tag = (parent->input_echo_tag != NULL) ? strdup(parent->input_echo_tag) : NULL;
    for (i = 0; i < SHELL_HASH_SIZE; i++)
      shell->cmd_hash[i] = parent->cmd_hash[i];
    shell->prologue_handler = parent->prologue_handler;
    shell->prologue_arg = parent->prologue_arg;
    shell->unknown_handler = parent->unknown_handler;
    shell->unknown_tag = parent->unknown_tag;
  }

  /* Interactive mode flag not inherited from parent shell */
  shell->interactive = 0;
  shell->noabort = 0;

#ifdef ENABLE_GLIB
  shell->use_glib = FALSE;
  shell->glib_loop = NULL;
#endif
  shell->exit_code = 0;

  return shell;
}


void shell_set_prologue(shell_t *shell, shell_prologue_handler_t *handler, void *arg)
{
  shell->prologue_handler = handler;
  shell->prologue_arg = arg;
}


#ifdef ENABLE_GLIB
#ifdef ENABLE_CMD_SOURCE

void shell_set_source_done(shell_t *shell, shell_event_handler_t *handler, void *arg)
{
  shell->source_done_fn = handler;
  shell->source_done_arg = arg;
}

#endif
#endif


void shell_set_input_echo(shell_t *shell, shell_handler_t *handler, char *tag)
{
  shell->input_echo_handler = handler;
  shell->input_echo_tag = ((handler != NULL) && (tag != NULL)) ? strdup(tag) : NULL;
}


int shell_set_interactive(shell_t *shell, int status)
{
  int previous = shell->interactive;

  shell->interactive = status;

  return previous;
}


int shell_set_noabort(shell_t *shell, int status)
{
  int previous = shell->noabort;

  shell->noabort = status;

  return previous;
}


static shell_command_t **shell_hash_entry(shell_t *shell, char c)
{
  if ( (c < SHELL_HASH_MIN) || (c > SHELL_HASH_MAX) )
    c = SHELL_HASH_OTHER;
  return &(shell->cmd_hash[c - SHELL_HASH_MIN]);
}

void shell_set_cmd(shell_t *shell, shell_command_t *cmd_tab)
{
  shell_command_t *cmd = cmd_tab;

  /* Update command hash */
  while ( cmd->keyword != NULL ) {
    shell_command_t **entry = shell_hash_entry(shell, cmd->keyword[0]);
    shell_command_t *item = *entry;

    if ( item != NULL ) {
      while ( item->next != NULL )
        item = item->next;
      item->next = cmd;
    }
    else {
      *entry = cmd;
    }

    cmd->next = NULL;

    cmd++;
  }

  /* Set unknown handler if defined in this command table */
  if ( cmd->handler != NULL ) {
    shell->unknown_handler = cmd->handler;
    shell->unknown_tag = cmd->tag;
  }
}


void shell_unset_cmd(shell_t *shell, shell_command_t *cmd_tab)
{
  shell_command_t *cmd = cmd_tab;

  /* Update command hash */
  while ( cmd->keyword != NULL ) {
    /* Retrieve entry in hash table */
    shell_command_t **entry = shell_hash_entry(shell, cmd->keyword[0]);

    /* Retrieve item in hash table entry */
    if ( *entry != NULL ) {
      shell_command_t *prev = NULL;
      shell_command_t *item = *entry;
      shell_command_t *next = item->next;

      while ( item != cmd ) {
        prev = item;
        item = next;
        next = item->next;
      }

      /* Remove item from hash entry */
      if ( prev != NULL )
        prev->next = next;
      else
        *entry = next;
    }

    cmd++;
  }
}


int shell_error(shell_t *shell, const char *fmt, ...)
{
  va_list ap;
  int count = 0;

  if ( shell->name != NULL )
    count += fprintf(stderr, "%s: ", shell->name);

  if ( (! shell->interactive) && (shell->file != NULL) ) {
    count += fprintf(stderr, "%s:", shell->file);
    if ( shell->line > 0 )
      count += fprintf(stderr, "%d:", shell->line);
    count += fprintf(stderr, " ");
  }

  if ( fmt != NULL ) {
    va_start(ap, fmt);
    count += vfprintf(stderr, fmt, ap);
    va_end(ap);
  }

  return count;
}


#ifdef ENABLE_GLIB

static int shell_exec_prompt(shell_t *shell)
{
  int ret = 0;

  if ( (shell->prologue_handler != NULL) && (shell->parent == NULL) )
    ret = shell->prologue_handler(shell, shell->prologue_arg);

  return ret;
}


#ifdef ENABLE_CMD_SOURCE

static int shell_source_pipe(shell_t *shell)
{
  if ( shell->source_pipe[0] != -1 )
    return 0;

  if ( pipe(shell->source_pipe) ) {
    shell_error(shell, "Cannot create source feeding pipe: %s\n", strerror(errno));
    return -1;
  }

  /* Prevent child processes from inheriting this pipe */
  fcntl(shell->source_pipe[0], F_SETFD, FD_CLOEXEC);
  fcntl(shell->source_pipe[1], F_SETFD, FD_CLOEXEC);

  return 0;
}


static void shell_source_done(shell_t *parent)
{
  if ( parent == NULL )
    return;

  if ( parent->source_done_fn != NULL )
    parent->source_done_fn(parent, parent->source_done_arg);

  parent->source_shell = NULL;

  if ( parent->paused ) {
    shell_exec_prompt(parent);
    shell_exec_read_watch(parent);
  }
}


static void shell_source_close(shell_t *shell)
{
  if ( shell->source_channel != NULL ) {
    g_io_channel_unref(shell->source_channel);
    shell->source_channel = NULL;
  }

  if ( shell->source_pipe[0] != -1 ) {
    close(shell->source_pipe[0]);
    shell->source_pipe[0] = -1;
  }

  if ( shell->source_pipe[1] != -1 ) {
    close(shell->source_pipe[1]);
    shell->source_pipe[1] = -1;
  }

  shell_free(shell);
}


static int shell_source_feed(shell_t *shell)
{
  GIOStatus status;
  GError *error = NULL;
  gchar *buf = NULL;
  gsize size = 0;
  int ret = 0;

  if ( shell->source_channel == NULL )
    return -1;

  status = g_io_channel_read_line(shell->source_channel, &buf, &size, NULL, &error);
  if ( status == G_IO_STATUS_NORMAL ) {
    if ( buf != NULL ) {
      if ( write(shell->source_pipe[1], buf, size) <= 0 )
	ret = -1;
      g_free(buf);
    }
  }
  else if ( status != G_IO_STATUS_AGAIN ) {
    if ( status == G_IO_STATUS_ERROR )
      shell_error(shell, "Error reading command line: %s\n", error->message);

    ret = -1;
  }

  if ( ret ) {
    close(shell->source_pipe[1]);
    shell->source_pipe[1] = -1;
  }

  return ret;
}

#endif /* ENABLE_CMD_SOURCE */
#endif /* ENABLE_GLIB */


static shell_command_t *shell_exec_retrieve(shell_t *shell, char *keyword)
{
  shell_command_t *cmd = *shell_hash_entry(shell, keyword[0]);

  while ( cmd != NULL ) {
    if ( strcmp(keyword, cmd->keyword) == 0 )
      return cmd;
    cmd = cmd->next;
  }

  return NULL;
}


int shell_exec_line(shell_t *shell, char *s, int *aborted)
{
  shell_command_t *cmd;
  shell_argv_t *cmd_argv;
#ifdef ENABLE_CMD_HISTORY
  shell_history_item_t *hsti = NULL;
#endif
  int ret = 0;

#ifdef ENABLE_CMD_HISTORY
  /* Check for history recall */
  if ((s[0] == '!') && isdigit(s[1])) {
	  int id = atoi(s+1);
	  hsti = shell_history_get(&shell->history, id);
	  s = hsti->str;
  }
#endif

  /* Setup command line */
  cmd_argv = shell_argv_alloc(s, shell->shell_argv);

  /* Resolve aliases */
#ifdef ENABLE_CMD_ALIAS
  shell_alias_resolve(shell->shell_alias, cmd_argv);
#endif

  /* Retrieve command handler */
  if ( (cmd = shell_exec_retrieve(shell, cmd_argv->argv[0])) == NULL ) {
    if ( shell->unknown_handler != NULL )
      return shell->unknown_handler(shell, cmd_argv, shell->unknown_tag);
  }

  /* Dump input echo */
  if ( (cmd->keyword != NULL) && (shell->input_echo_handler != NULL) )
    shell->input_echo_handler(shell, cmd_argv, shell->input_echo_tag);

  /* Execute command handler */
  if ( cmd->handler != NULL )
    ret = cmd->handler(shell, cmd_argv, cmd->tag);
  else
    *aborted = 1;

  /* Free argument list */
  shell_argv_free(cmd_argv);

#ifdef ENABLE_CMD_HISTORY
  /* Update command history */
  if (hsti != NULL) {
	  shell_history_first(&shell->history, hsti);
  }
  else {
	  if (!cmd->no_history)
		  hsti = shell_history_update(&shell->history, s);
  }

  if (hsti != NULL)
	  hsti->ret = ret;
#endif

  return ret;
}


#ifdef ENABLE_GLIB

static void shell_exec_read_freeze(shell_t *shell)
{
  if ( shell->io_tag > 0 ) {
    g_source_remove(shell->io_tag);
    shell->io_tag = 0;
  }
}


static void shell_exec_read_close(shell_t *shell)
{
  if ( shell->sleep_tag > 0 ) {
    g_source_remove(shell->sleep_tag);
    shell->sleep_tag = 0;
  }

  shell_exec_read_freeze(shell);

  if ( shell->io_channel != NULL ) {
    g_io_channel_unref(shell->io_channel);
    shell->io_channel = NULL;
  }
}


static int shell_exec_read(shell_t *shell, int *_aborted)
{
  GIOStatus status;
  int aborted = 0;
  GError *error = NULL;
  gchar *buf = NULL;
  int ret = 0;

  do {
    status = g_io_channel_read_line(shell->io_channel, &buf, NULL, NULL, &error);
    //fprintf(stderr, "-SHELL- read %d\n", status);

    if ( status == G_IO_STATUS_NORMAL ) {
      (shell->line)++;

      /* Cleanup read line */
      if ( buf != NULL ) {
	char *s;

	strcleanup(buf);
	s = strskip_spaces(buf);
	//fprintf(stderr, "-SHELL- read [%d] '%s'\n", shell->line, s);

	if ( *s != NUL ) {
	  /* Parse and execute command */
	  ret = shell_exec_line(shell, s, &aborted);
	}

	g_free(buf);
      }

      /* Abort if an error occured in non-interactive mode */
      if ( (ret != 0) && (! shell->interactive) )
	aborted = 1;

      /* Reset abort flag if noabort is enabled */
      if ( shell->noabort )
	aborted = 0;
    }
  } while ( (! aborted) && (status == G_IO_STATUS_NORMAL) );

  if ( status == G_IO_STATUS_ERROR ) {
    shell_error(shell, "Error reading command line: %s\n", error->message);
  }

  if ( (status == G_IO_STATUS_ERROR) || (status == G_IO_STATUS_EOF) ) {
    shell_exec_read_close(shell);
    aborted = 1;
  }

  if ( (ret == 0) && (! aborted) && (! shell->paused) && (shell->sleep_tag == 0) ) {
    ret = shell_exec_prompt(shell);
  }

  if ( aborted ) {
    /* Update abort flag */
    if ( _aborted != NULL )
      *_aborted = 1;
  }
#ifdef ENABLE_CMD_SOURCE
  else {
    /* Feed next command for source execution (if any) */
    shell_source_feed(shell);
  }
#endif

  return ret;
}


static gboolean shell_exec_read_event(GIOChannel *source, GIOCondition condition, shell_t *shell)
{
  int aborted = 0;
  int ret = 0;

  if ( condition & G_IO_IN )
    ret = shell_exec_read(shell, &aborted);
  else
    aborted = 1;

  if ( aborted ) {
    shell_t *parent = shell->parent;

    /* Close source feeding (if any) */
    if ( parent != NULL ) {
#ifdef ENABLE_CMD_SOURCE
      shell_source_close(shell);
      shell_source_done(parent);
#endif
    }
    else {
      shell->exit_code = ret;

      if ( shell->glib_loop != NULL ) {
	g_main_loop_quit(shell->glib_loop);
      }
      else {
	exit(ret ? EXIT_FAILURE : EXIT_SUCCESS);
      }
    }

    return FALSE;
  }

  return TRUE;
}


static int shell_exec_read_watch(shell_t *shell)
{
  /* Setup command input handler */
  shell->io_tag = g_io_add_watch(shell->io_channel, G_IO_IN | G_IO_HUP,
				 (GIOFunc) shell_exec_read_event, shell);

  shell->paused = 0;

  return 0;
}


static int shell_exec_read_loop(shell_t *shell)
{
  int ret = 0;
  int aborted = 0;

  /* Process incoming commands */
  while ( (shell->io_channel != NULL) && (!aborted) ) {
    ret = shell_exec_read(shell, &aborted);
  }

  /* Close input file */
  shell_exec_read_close(shell);

  return ret;
}


static gboolean shell_exec_sleep_expired(shell_t *shell)
{
  if ( shell->sleep_tag > 0 )
    g_source_remove(shell->sleep_tag);
  shell->sleep_tag = 0;

  if ( shell->use_glib ) {
    if ( ! shell->paused ) {
      shell_exec_prompt(shell);
      shell_exec_read_watch(shell);
    }
  }
  else {
    shell_exec_read_loop(shell);
  }

  return FALSE;
}


void shell_exec_sleep(shell_t *shell, long delay)
{
  if ( shell->use_glib ) {
    shell_exec_read_freeze(shell);

    if ( shell->sleep_tag > 0 )
      g_source_remove(shell->sleep_tag);
    shell->sleep_tag = g_timeout_add(delay, (GSourceFunc) shell_exec_sleep_expired, shell);
  }
  else {
    sleep_ms(delay);
  }
}


int shell_exec(shell_t *shell)
{
  int ret = 0;

  /* Clear line counter */
  shell->line = 0;

  /* Open input file */
  if ( shell->file != NULL ) {
    GIOChannel *channel;
    GError *error = NULL;

    channel = g_io_channel_new_file(shell->file, "r", &error);
    if ( channel == NULL ) {
      if ( shell->parent != NULL )
	shell_error(shell->parent, "Error opening file '%s': %s\n", shell->file, error->message);
      else
	shell_error(shell, "Error opening this file: %s\n", error->message);
      return -1;
    }

#ifdef ENABLE_CMD_SOURCE
    if ( shell->use_glib ) {
      /* Create source feeding pipe */
      ret = shell_source_pipe(shell);

      shell->source_channel = channel;
      shell->io_channel = g_io_channel_unix_new(shell->source_pipe[0]);

      /* Start source feeding */
      shell_source_feed(shell);
    }
    else {
      shell->source_channel = NULL;
      shell->io_channel = channel;
    }
#else
    shell->io_channel = channel;
#endif
  }
  else {
    shell->io_channel = g_io_channel_unix_new(fileno(stdin));
  }

  if ( shell->io_channel != NULL ) {
    GError *error = NULL;
    GIOFlags flags = g_io_channel_get_flags(shell->io_channel);
    g_io_channel_set_flags(shell->io_channel, flags | G_IO_FLAG_NONBLOCK, &error);
  }

  /* Prompt user for command */
  ret = shell_exec_prompt(shell);
  if ( (ret != 0) && (! shell->interactive) )
    return ret;

  if ( shell->use_glib ) {
    ret = shell_exec_read_watch(shell);
  }
  else {
    ret = shell_exec_read_loop(shell);
  }

  return ret;
}


#ifdef ENABLE_CMD_SOURCE

int shell_source(shell_t *shell, int argc, char **argv)
{
  shell_t *subshell;
  int ret = -1;

  /* Forebid nested file sources */
  if ( shell->parent != NULL ) {
    shell_error(shell, "Cannot nest file sources: script %s ignored\n", argv[0]);
    return -1;
  }

  subshell = shell->source_shell;

  /* Source execution control */
  if ( argv[0][0] == '-' ) {
    char *ctrl = &(argv[0][1]);

    if ( strcmp(ctrl, "pause") == 0 ) {
      if ( subshell != NULL ) {
	ret = 0;
	subshell->paused = 1;
	shell_exec_read_freeze(subshell);
      }
    }
    else if ( strcmp(ctrl, "resume") == 0 ) {
      if ( subshell != NULL ) {
	ret = 0;
	shell_exec_read_watch(subshell);
      }
    }
    else if ( strcmp(ctrl, "stop") == 0 ) {
      if ( subshell != NULL ) {
	ret = 0;
	shell_source_close(subshell);
	shell_source_done(shell);
      }
    }
    else if ( strcmp(ctrl, "wait") == 0 ) {
      ret = 0;
      if ( subshell != NULL ) {
	shell->paused = 1;
	shell_exec_read_freeze(shell);
      }
    }
    else {
      shell_error(shell, "Unknown source execution control keyword '%s'\n", ctrl);
      return -1;
    }

    if ( ret ) {
      shell_error(shell, "No source execution is currently active\n");
    }
  }

  /* Source execution startup */
  else {
    /* Check for source non-redundancy */
    if ( subshell == NULL ) {
      /* Create source subshell */
      subshell = shell_alloc(shell->name, argc, argv, shell);

      /* Exec subshell */
      ret = shell_exec(subshell);

      if ( subshell->source_channel == NULL ) {
	/* Destroy subshell */
	shell_free(subshell);
      }
      else {
	shell->source_shell = subshell;
      }
    }
    else {
      shell_error(shell, "Source execution is currently active from %s\n", shell->source_shell->file);
    }
  }

  return ret;
}

#endif /* ENABLE_CMD_SOURCE */

#else  /* ENABLE_GLIB */

void shell_exec_sleep(shell_t *shell, long delay)
{
  sleep_ms(delay);
}


int shell_exec(shell_t *shell)
{
  FILE *f = stdin;
  char buf[BUFSIZ];
  char *s;
  int aborted = 0;
  int ret = 0;

  /* Clear line counter */
  shell->line = 0;

  /* Open input file */
  if ( shell->file != NULL ) {
    if ( (f = fopen(shell->file, "r")) == NULL ) {
      if ( shell->parent != NULL )
        shell_error(shell->parent, "Unable to open file '%s'\n", shell->file);
      else
        shell_error(shell, "Unable to open this file\n");
      return -1;
    }
  }

  shell->fd = fileno(f);

  while ( (!feof(f) ) && (!aborted) ) {
    /* Call prologue handler if any */
    if ( shell->prologue_handler != NULL ) {
      ret = shell->prologue_handler(shell, shell->prologue_arg);
      if ( (ret != 0) && (! shell->interactive) )
        aborted = 1;
    }

    if ( ! aborted ) {
      /* Get command */
      if ( (s = fgets(buf, sizeof(buf)-1, f)) != NULL ) {
        (shell->line)++;

        /* Cleanup read line */
        strcleanup(buf);
        //if ( (s = strchr(buf, '#')) != NULL )
        //  *s = NUL;
        s = strskip_spaces(buf);

        if ( *s != NUL ) {
          /* Exec line; Abort if an error occured in non-interactive mode */
          ret = shell_exec_line(shell, s, &aborted);
          if ( (ret != 0) && (! shell->interactive) )
            aborted = 1;
        }
      }
    }

    /* Reset abort flag if noabort is enabled */
    if ( shell->noabort )
      aborted = 0;
  }

  /* Close input file */
  if ( f != stdin )
    fclose(f);
  shell->fd = -1;

  return ret;
}


#ifdef ENABLE_CMD_SOURCE

int shell_source(shell_t *shell, int argc, char **argv)
{
  char *parent_file;
  int parent_line;
  shell_argv_t *parent_argv;
  int ret;

  /* Save and change script context */
  parent_file = shell->file;
  if ( (argv != NULL) && (argv[0] != NULL ) )
    shell->file = strdup(argv[0]);
  else
    shell->file = NULL;

  parent_line = shell->line;
  shell->line = 0;

  parent_argv = shell->shell_argv;
  shell->shell_argv = shell_argv_allocv(argc, argv, shell->shell_argv);

  /* Exec subshell */
  ret = shell_exec(shell);

  /* Restore script identification context */
  if ( shell->file != NULL )
    free(shell->file);
  shell->file = parent_file;

  shell->line = parent_line;

  if ( shell->shell_argv != NULL )
    shell_argv_free(shell->shell_argv);
  shell->shell_argv = parent_argv;

  return ret;
}

#endif /* ENABLE_CMD_SOURCE */

#endif /* !ENABLE_GLIB */


void shell_free(shell_t *shell)
{
#ifdef ENABLE_GLIB

#ifdef ENABLE_CMD_SOURCE
  if ( shell->source_shell != NULL ) {
    shell_source_close(shell->source_shell);
    shell->source_shell = NULL;
  }
#endif /* ENABLE_CMD_SOURCE */

  shell_exec_read_close(shell);
#endif /* ENABLE_GLIB */

  if ( shell->name != NULL )
    free(shell->name);

  if ( shell->file != NULL )
    free(shell->file);

  if ( shell->shell_argv != NULL )
    shell_argv_free(shell->shell_argv);

#ifdef ENABLE_CMD_ALIAS
  if ( shell->shell_alias != NULL )
    shell_alias_done(shell->shell_alias);
#endif

  if ( shell->input_echo_tag != NULL )
    free(shell->input_echo_tag);

  free(shell);
}
