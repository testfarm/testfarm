/****************************************************************************/
/* TestFarm                                                                 */
/* Shell Script Interpreter Library - Sample and Test program               */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 31-AUG-1999                                                    */
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
#include <errno.h>
#include <unistd.h>

#include "sig.h"
#include "shell.h"

#define NAME    "shell"
#define PROMPT  "SHELL> "


/*
 * version display
 */

int test_version(shell_t *shell, shell_argv_t *cmd_argv, char *tag)
{
  /* Check arguments */
  if ( cmd_argv->argc != 1 ) {
    shell_std_help(shell, cmd_argv->argv[0]);
    return -1;
  }

  /* Display version */
  printf("%sSHELL " VERSION " (" __DATE__ ")\n", shell_std_header(tag));

  return 0;
}


/*
 * Help display
 */

shell_std_help_t help[] = {
  { "version", "version\n  Display shell interpreter version\n" },
  { "sleep",   "sleep <time> [h|min|s|ms]\n"
               "  Wait <time> milliseconds or [hours|minutes|seconds|milliseconds]\n" },
  { NULL,     NULL }
};


/*
 * Script interpreter
 */

#ifdef ENABLE_GLIB
static GMainLoop *loop = NULL;
#else
static int input_wait(shell_t *shell)
{
	int input_fd = shell_get_fd(shell);
	int ret = 0;
	fd_set rds;
	int max;
	int count;

	FD_ZERO(&rds);
	FD_SET(input_fd, &rds);
	max = input_fd;

	count = select(max+1, &rds, NULL, NULL, NULL);
	if (count < 0) {
		if ((errno != EAGAIN) && (errno != EINTR)) {
			perror(NAME ": select");
			ret = -1;
		}
	}
	else if (count > 0) {
		if ( FD_ISSET(input_fd, &rds) ) {
			ret = 1;
		}
	}

	return ret;
}
#endif

shell_t *shell = NULL;

shell_command_t commands[] = {
  { "version", test_version,      "VERSION" },
  { "sleep",   shell_std_sleep,   "SLEEP  " },
  { NULL,      shell_std_unknown, NULL }
};


int prologue(shell_t *shell, void *arg)
{
  /* Display prompt if interactive mode */
  if ( shell->interactive ) {
    printf(PROMPT);
    fflush(stdout);
  }

  /* Wait for input from STDIN if not in GLib mode */
#ifndef ENABLE_GLIB
  int ret;
  while ((ret = input_wait(shell)) == 0);
  if ( ret < 0 )
    exit(1);
#endif

  return 0;
}


void terminate(void)
{
  /* Free shell manager */
  if ( shell != NULL ) {
    shell_free(shell);
    shell = NULL;
  }
}


int main(int argc, char *argv[])
{
  int interactive;
#ifndef ENABLE_GLIB
  int ret;
#endif

  /* Interactive mode if script read from standard input on a terminal */
  if ( argc > 1 )
    interactive = 0;
  else
    interactive = isatty(STDIN_FILENO);

  /* Hook termination procedure at exit stack and signal handling */
  atexit(terminate);
  sig_init(terminate);

  /* Line-buffered server i/o */
  setlinebuf(stdin);
  setlinebuf(stdout);

  /* Setup I/O management mode */
#ifdef ENABLE_GLIB
  fprintf(stderr, NAME ": Using GLib event handling\n");
#else
  fprintf(stderr, NAME ": Using Unix i/o multiplexing\n");
#endif

  /* Create command shell descriptor */
  shell = shell_alloc(NAME, argc-1, argv+1, NULL);

  /* Set interactive mode if standard input is a terminal */
  shell_set_interactive(shell, interactive);

  /* Setup prologue routine, used for displaying user's prompt */
  shell_set_prologue(shell, prologue, NULL);

  /* Input echo : default configuration */
  shell_set_input_echo(shell, shell_std_input_echo, NULL);

  /* Setup special commands */
  shell_set_cmd(shell, commands);

  /* Setup standard commands, standard header routine, help table */
  shell_std_setup(shell, help);

  /* Perform shell interpreter processing */
#ifdef ENABLE_GLIB
  /* Create GLib main loop */
  loop = g_main_loop_new(NULL, FALSE);
  shell_use_glib(shell, loop);

  if ( shell_exec(shell) )
    exit(EXIT_FAILURE);

  /* Enter GLib main loop */
  g_main_loop_run(loop);
#else
  ret = shell_exec(shell);
#endif

  fprintf(stderr, NAME ": Shell execution terminated with code %d\n", shell->exit_code);
  exit(shell->exit_code);

  return 0;
}
