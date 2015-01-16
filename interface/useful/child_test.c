/****************************************************************************/
/* TestFarm                                                                 */
/* test Program for Child processes management                              */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 14-OCT-2003                                                    */
/****************************************************************************/

/*
 * $Revision: 29 $
 * $Date: 2006-06-03 10:39:29 +0200 (sam., 03 juin 2006) $
 */

#include <stdio.h>
#include <signal.h>

#include "child.h"

static void sigchld(int status, void *arg)
{
  fprintf(stderr, "SIGCHLD status=0x%04X\n", status);
}


int main(int argc, char *argv[])
{
  char *cargv[] = {"./child_test.pl", NULL};
  child_t *child;

  child_init();
  
  child = child_spawn(cargv, -1, -1, -1, sigchld, NULL);
  if ( child == NULL ) {
    fprintf(stderr, "Child spawn failed\n");
    return 1;
  }

  fprintf(stderr, "Sleeping 5 seconds...\n");
  sleep(5);

  child_terminate(child);

  sleep(1);

  child_done();
  return 0;
}
