/****************************************************************************/
/* TestFarm                                                                 */
/* Shell Script Interpreter Library - Standard sleep commands               */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 16-APR-2007                                                    */
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

#include "shell.h"


int shell_std_sleep(shell_t *shell, shell_argv_t *cmd_argv, char *tag)
{
  int argc = cmd_argv->argc;
  char **argv = cmd_argv->argv;
  char *hdr = shell_std_header(tag);
  long time;

  /* Check arguments */
  if ( (argc != 2) && (argc != 3) ) {
    printf("%sToo %s arguments", hdr, (argc < 2) ? "few":"many");
    shell_std_help(shell, argv[0]);
    return -1;
  }

  /* Get duration value */
  time = strtol(argv[1], (char **) NULL, 0);
  if ( time < 0 ) {
    printf("%sBad duration value", hdr);
    shell_std_help(shell, argv[0]);
    return -1;
  }

  /* Null delay? */
  if ( time == 0 )
    return 0;

  /* Compute time in milliseconds */
  if ( argc == 3 ) {
    if ( strcmp(argv[2], "h") == 0 )
      time *= 1000 * 60 * 60;
    else if ( strcmp(argv[2], "min") == 0 )
      time *= 1000 * 60;
    else if ( strcmp(argv[2], "s") == 0 )
      time *= 1000;
    else if ( strcmp(argv[2], "ms") != 0 ) {
      printf("%sBad duration unit", hdr);
      shell_std_help(shell, argv[0]);
      return -1;
    }
  }

  /* Wait */
  shell_exec_sleep(shell, time);

  return 0;
}
