/****************************************************************************/
/* TestFarm VNC Interface                                                   */
/* Error reporting                                                          */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 22-OCT-2001                                                    */
/****************************************************************************/

/*
 * $Revision: 486 $
 * $Date: 2007-04-25 12:32:21 +0200 (mer., 25 avril 2007) $
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
