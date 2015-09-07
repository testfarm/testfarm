/****************************************************************************/
/* TestFarm                                                                 */
/* Test Engine                                                              */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 17-AUG-1999                                                    */
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
#include <unistd.h>
#include <signal.h>

#include "useful.h"
#include "sig.h"
#include "debug.h"
#include "shell.h"
#include "periph.h"
#include "result.h"
#include "run.h"


/*-----------------------------------------------*/
/* Program arguments                             */
/*-----------------------------------------------*/

static char *wait_output = NULL;
static char *global_log = NULL;
static int script_argc = 0;
static char **script_argv = { NULL };


int arguments_init(int argc, char *argv[])
{
  int i = 1;

  /* Parse program arguments */
  while ( (i < argc) && (script_argc == 0) ) {
    if ( strcmp(argv[i], "-debug") == 0 ) {
      debug_flag = 1;
    }
    else if ( strcmp(argv[i], "-wait") == 0 ) {
      i++;
      if ( i >= argc )
        return -1;
      wait_output = argv[i];
    }
    else if ( strcmp(argv[i], "-o") == 0 ) {
      i++;
      if ( i >= argc )
        return -1;
      global_log = argv[i];
    }
    else if ( argv[i][0] != '-' ) {
      script_argc = argc - i;
      script_argv = &(argv[i]);
    }
    else {
      return -1;
    }

    i++;
  }

  return 0;
}


void arguments_done(void)
{
}


/*-----------------------------------------------*/
/* Peripherals management                        */
/*-----------------------------------------------*/

periph_t *periph = NULL;


/*-----------------------------------------------*/
/* Program body                                  */
/*-----------------------------------------------*/

void terminate(void)
{
  /* Abort runtime */
  run_done();

  /* Abort peripheral links */
  if ( periph != NULL ) {
    periph_done(periph);
    periph = NULL;
  }

  /* Free arguments */
  arguments_done();
}


void sighdl(int sig)
{
  fprintf(stderr, NAME ": Signal %d caught\n", sig);
}


int main(int argc, char *argv[])
{
  int interactive;
  int ret;

  /* Retrieve arguments */
  if ( arguments_init(argc, argv) ) {
    fprintf(stderr, "TestFarm Test Engine version " VERSION " (" __DATE__ ")\n");
    fprintf(stderr, "Usage: " NAME " [-debug] [-wait <pipe-name-or-fd>] [-o <global-log>] [<script-file> [<arguments>]]\n");
    exit(EXIT_FAILURE);
  }

  /* Interactive mode if script read from standard input on a terminal */
  if ( script_argc > 0 )
    interactive = 0;
  else
    interactive = isatty(STDIN_FILENO);
  debug("Interactive mode %sabled\n", interactive ? "en" : "dis");

  /* Force global log to stdout if interactive mode */
  if ( interactive )
    global_log = NULL;

  /* Check WAIT synchronization output */
  if ( wait_output != NULL )
    debug("WAIT synchronization output is %s\n", wait_output);

  /* Hook termination procedure at exit stack and signal handling */
  atexit(terminate);
  sig_init(terminate);
  signal(SIGPIPE, sighdl);

  /* Line-buffered standard i/o */
  setbuf(stdin, NULL);
  setbuf(stdout, NULL);

  /* Init peripheral descriptors */
  periph = periph_init();

  /* Check peripheral initialization */
  if ( periph == NULL )
    exit(EXIT_FAILURE);

  /* Init runtime */
  if ( run_init(periph, script_argc, script_argv, interactive, wait_output) )
    exit(EXIT_FAILURE);

  /* Setup global log file configuration */
  result_global_set(global_log);

  /* Enter server loop */
  ret = run_loop();

  /* That's all Folks... */
  exit((ret == 0) ? EXIT_SUCCESS : EXIT_FAILURE);
  return 0;
}
