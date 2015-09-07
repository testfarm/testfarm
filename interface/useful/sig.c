/****************************************************************************/
/* TestFarm                                                                 */
/* Shell Script Interpreter Library - Signal handling                       */
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
#include <string.h>
#include <signal.h>
#include <sys/signal.h>  /* Solaris needs both... */

#include "sig.h"


static sig_terminate_t *sig_terminate = NULL;

static int sig_list[] = {
  SIGHUP, SIGINT, SIGQUIT, SIGTERM, SIGBUS, SIGFPE, SIGSEGV, SIGPIPE,
  -1
};


static void sig_handler(int sig)
{
  /* Clear all signal handlers */
  sig_done();

  /* Tell about what's up */
  if ( sig != SIGTERM )
    fprintf(stderr, "Signal %d caught\n", sig);

  /* Abort everything */
  if ( sig_terminate != NULL )
    sig_terminate();

  /* Invoke original signal handler */
  raise(sig);
}


void sig_init(sig_terminate_t *terminate)
{
  int *sig = sig_list;

  sig_terminate = terminate;

  while ( *sig != -1 )
    signal(*(sig++), sig_handler);
}


void sig_done(void)
{
  int *sig = sig_list;

  while ( *sig != -1 )
    signal(*(sig++), SIG_DFL);
}

