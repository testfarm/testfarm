/****************************************************************************/
/* TestFarm                                                                 */
/* Shell Script Interpreter Library - Signal handling                       */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 26-AUG-1999                                                    */
/****************************************************************************/

/*
 * $Revision: 1076 $
 * $Date: 2009-10-29 10:34:48 +0100 (jeu., 29 oct. 2009) $
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

