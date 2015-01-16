/****************************************************************************/
/* TestFarm                                                                 */
/* Shell Script Interpreter Library - Standard sleep commands               */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 16-APR-2007                                                    */
/****************************************************************************/

/*
 * $Revision: 479 $
 * $Date: 2007-04-16 14:44:00 +0200 (lun., 16 avril 2007) $
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
