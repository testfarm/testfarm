/****************************************************************************/
/* TestFarm                                                                 */
/* Shell Script Interpreter Library                                         */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 26-AUG-1999                                                    */
/****************************************************************************/

/*
 * $Revision: 1231 $
 * $Date: 2012-04-08 16:06:18 +0200 (dim., 08 avril 2012) $
 */

#ifndef __SHELL_EXEC_H__
#define __SHELL_EXEC_H__

#include <stdio.h>

#include "shell_config.h"

#ifdef ENABLE_GLIB
#include <glib.h>
#endif

#include "shell_argv.h"

#ifdef ENABLE_CMD_ALIAS
#include "shell_alias.h"
#endif

#ifdef ENABLE_CMD_HISTORY
#include "shell_history.h"
#endif

typedef struct _shell_s shell_t;
typedef int shell_event_handler_t(shell_t *shell, void *arg);
typedef shell_event_handler_t shell_prologue_handler_t;
typedef int shell_handler_t(shell_t *shell, shell_argv_t *cmd_argv, char *tag);

#define SHELL_HASH_MIN ' '
#define SHELL_HASH_MAX 'z'
#define SHELL_HASH_OTHER (SHELL_HASH_MAX + 1)
#define SHELL_HASH_SIZE (SHELL_HASH_OTHER - SHELL_HASH_MIN + 1)

typedef struct shell_command_s shell_command_t;
struct shell_command_s {
  char *keyword;
  shell_handler_t *handler;
  char *tag;
  int no_history;

  shell_command_t *next;
};

struct _shell_s {
  char *name;
  char *file;
  int line;
#ifdef ENABLE_GLIB
  GIOChannel *io_channel;
  guint io_tag;
  guint sleep_tag;
  int paused;

#ifdef ENABLE_CMD_SOURCE
  shell_t *source_shell;
  int source_pipe[2];
  GIOChannel *source_channel;
  shell_event_handler_t *source_done_fn;
  void *source_done_arg;
#endif  /* ENABLE_CMD_SOURCE */
#else   /* ENABLE_GLIB */
  int fd;
#endif  /* !ENABLE_GLIB */

  shell_t *parent;
  shell_argv_t *shell_argv;
#ifdef ENABLE_CMD_ALIAS
  shell_alias_t *shell_alias;
#endif
#ifdef ENABLE_CMD_HISTORY
  shell_history_t history;
#endif

  shell_prologue_handler_t *prologue_handler;
  void *prologue_arg;

  shell_handler_t *unknown_handler;
  char *unknown_tag;

  shell_handler_t *input_echo_handler;
  char *input_echo_tag;

  shell_command_t *cmd_hash[SHELL_HASH_SIZE];

  int interactive;  /* Interactive input mode flag */
  int noabort;      /* Do not abort when an error occurs */

#ifdef ENABLE_GLIB
  gboolean use_glib;
  GMainLoop *glib_loop;
#endif
  int exit_code;
};


extern shell_t *shell_alloc(char *name, int argc, char **argv, shell_t *parent);
extern void shell_set_prologue(shell_t *shell, shell_prologue_handler_t *handler, void *arg);
extern void shell_set_input_echo(shell_t *shell, shell_handler_t *handler, char *tag);
extern int shell_set_interactive(shell_t *shell, int status);
extern int shell_set_noabort(shell_t *shell, int status);
extern void shell_set_cmd(shell_t *shell, shell_command_t *cmd_tab);
extern void shell_unset_cmd(shell_t *shell, shell_command_t *cmd_tab);
extern void shell_free(shell_t *shell);
extern int shell_error(shell_t *shell, const char *fmt, ...);

extern void shell_exec_sleep(shell_t *shell, long delay);
extern int shell_exec_line(shell_t *shell, char *s, int *exit);
extern int shell_exec(shell_t *shell);

#ifdef ENABLE_CMD_SOURCE
extern int shell_source(shell_t *shell, int argc, char **argv);
extern void shell_set_source_done(shell_t *shell, shell_event_handler_t *handler, void *arg);
#endif

extern int shell_get_fd(shell_t *shell);

#ifdef ENABLE_GLIB
extern void shell_use_glib(shell_t *shell, GMainLoop *loop);
#endif

#endif /* __SHELL_EXEC_H__ */
