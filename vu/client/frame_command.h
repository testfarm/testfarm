/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* Frame Management command                                                 */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 09-NOV-2007                                                    */
/****************************************************************************/

/*
 * $Revision: 877 $
 * $Date: 2007-12-22 20:14:58 +0100 (sam., 22 déc. 2007) $
 */

#ifndef __TVU_FRAME_COMMAND_H__
#define __TVU_FRAME_COMMAND_H__

#include "shell.h"
#include "frame_display.h"

extern int frame_command_init(shell_t *shell, frame_display_t *display);
extern void frame_command_done(shell_t *shell);

#endif /* __TVU_FRAME_COMMAND_H__ */
