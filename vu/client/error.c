/****************************************************************************/
/* TestFarm VNC Interface                                                   */
/* Error reporting                                                          */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 22-OCT-2001                                                    */
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
#include <stdarg.h>

#include "log.h"
#include "shell.h"
#include "error.h"


/*******************************************************/
/* Error messages reporting                            */
/*******************************************************/

static char error_tag[16] = "*";


void error(char *tag, char *fmt, ...)
{
  va_list ap;

  if ( tag == NULL )
    tag = error_tag;

  printf("%s*ERROR* ", log_hdr_(tag));

  if ( fmt != NULL ) {
    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);
  }

  printf("\n");
}


void eprintf(char *fmt, ...)
{
  va_list ap;

  printf("%s*ERROR* ", log_hdr_(error_tag));

  if ( fmt != NULL ) {
    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);
  }

  printf("\n");
}


void error_default_tag(char *tag)
{
  if ( tag == NULL )
    tag = "*";
  strncpy(error_tag, tag, sizeof(error_tag));
}


/*******************************************************/
/* Commands argument check                             */
/*******************************************************/

int check_argc(shell_t *shell, shell_argv_t *cmd_argv, char *tag,
	       int min, int max)
{
  int argc = cmd_argv->argc;
  char **argv = cmd_argv->argv;

  /* Check number of arguments */
  if ( (min > 0) && (argc < min) ) {
    error(tag, "%s: Too few arguments", argv[0]);
    shell_std_help(shell, argv[0]);
    return -1;
  }

  if ( (max > 0) && (argc > max) ) {
    error(tag, "%s: Too many arguments", argv[0]);
    shell_std_help(shell, argv[0]);
    return -1;
  }

  return 0;
}
