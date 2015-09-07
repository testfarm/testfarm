/****************************************************************************/
/* TestFarm                                                                 */
/* Test Program for Child processes management                              */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 14-OCT-2003                                                    */
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
