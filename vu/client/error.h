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

#ifndef __RFB_ERROR_H__
#define __RFB_ERROR_H__

#include "shell.h"

#define NAME    "testfarm-vu"

extern void error(char *tag, char *fmt, ...);
extern void error_default_tag(char *tag);

extern void eprintf(char *fmt, ...);

extern int check_argc(shell_t *shell, shell_argv_t *cmd_argv, char *tag,
		      int min, int max);

#endif /* __RFB_ERROR_H__ */
