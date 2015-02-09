/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* Pattern Matching commands                                                 */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 12-NOV-2007                                                    */
/****************************************************************************/

/*
 * $Revision: 877 $
 * $Date: 2007-12-22 20:14:58 +0100 (sam., 22 déc. 2007) $
 */

#ifndef __TVU_MATCH_COMMAND_H__
#define __TVU_MATCH_COMMAND_H__

#include "shell.h"
#include "frame_display.h"

extern int match_command_init(shell_t *shell, frame_display_t *display);
extern void match_command_done(shell_t *shell);

#endif /* __TVU_MATCH_COMMAND_H__ */
