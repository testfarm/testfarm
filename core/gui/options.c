/****************************************************************************/
/* TestFarm                                                                 */
/* Test Suite User Interface: command options                               */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 25-SEP-2002                                                    */
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

#define MIN_OUTPUT_DEPTH 50

int opt_noflags = 0;
int opt_go = 0;
int opt_quit = 0;
char *opt_target = NULL;
unsigned long opt_depth = 0;
char *opt_operator = NULL;
char *opt_release = NULL;
int opt_command = 0;
int opt_service = 2345;
int opt_nogui = 0;


int opt_get(int argc, char *argv[])
{
  int argx = 1;

  while ( (argx < argc) && (argv[argx][0] == '-') ) {
    if ( strcmp(argv[argx], "--noflags") == 0 ) {
      opt_noflags = 1;
    }
    else if ( strcmp(argv[argx], "--go") == 0 ) {
      opt_go = 1;
    }
    else if ( strcmp(argv[argx], "--quit") == 0 ) {
      opt_quit = 1;
    }
    else if ( strcmp(argv[argx], "--target") == 0 ) {
      argx++;
      opt_target = argv[argx];

      if ( opt_target == NULL )
        return -1;

      /* Check target directory is valid */
      if ( access(opt_target, W_OK) ) {
	fprintf(stderr, "WARNING: Cannot access target directory '%s': %s\n", opt_target, strerror(errno));
	fprintf(stderr, "WARNING: Using <Tree Root>/report as target directory\n");
	opt_target = NULL;
      }

    }
    else if ( strcmp(argv[argx], "--depth") == 0 ) {
      argx++;
      opt_depth = strtoul(argv[argx], (char **) NULL, 0);

      if ( opt_depth < MIN_OUTPUT_DEPTH )
        return -1;
    }
    else if ( strcmp(argv[argx], "--operator") == 0 ) {
      argx++;
      opt_operator = argv[argx];

      if ( opt_operator == NULL )
        return -1;
    }
    else if ( strcmp(argv[argx], "--release") == 0 ) {
      argx++;
      opt_release = argv[argx];

      if ( opt_release == NULL )
        return -1;
    }
    else if ( strcmp(argv[argx], "--service") == 0 ) {
      argx++;
      opt_service = atoi(argv[argx]);
    }
    else if ( strcmp(argv[argx], "--nogui") == 0 ) {
      opt_nogui = 1;
    }
    else if ( strcmp(argv[argx], "-c") == 0 ) {
      opt_command = 1;
    }
    else {
      return -1;
    }

    argx++;
  }

  return argx;
}


void opt_usage(int summary)
{
  if ( summary ) {
    fprintf(stderr, "[--noflags] [--go] [--quit] [--target <directory>] [--depth <value>] [--operator <name>] [--release <name>] [-c]");
  }
  else {
    fprintf(stderr,
            "  --noflags: Do not take the tree flags file into account (skip and breakpoint flags)\n"
            "  --go     : Automatically launch execution after loading\n"
            "  --quit   : Automatically exit when execution is finished or aborted\n"
            "  --target <directory> : Set target directory for files generated during test execution\n"
            "  --depth <value> : Set max depth of output dump display (minimum=%d, default=unlimited)\n"
            "  --operator <name> : Set operator name\n"
            "  --release <name> : Set test output release name\n"
            "  --service <port> : Listen for remote TCP/IP service connections from port number <port>\n"
            "  --nogui : Run Test Suite without Graphical User Interface\n"
            "  -c : Enable stdin control\n",
            MIN_OUTPUT_DEPTH);
  }
}
