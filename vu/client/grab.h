/**********************************************************************/
/* TestFarm Virtual User                                              */
/* Grab command and utilities                                         */
/**********************************************************************/
/* Author: Sylvain Giroudon                                           */
/* Creation: 29-AUG-2007                                              */
/**********************************************************************/

/*
 * $Revision: 813 $
 * $Date: 2007-11-22 17:54:18 +0100 (jeu., 22 nov. 2007) $
 */

#ifndef __TVU_GRAB_H__
#define __TVU_GRAB_H__

#include "shell.h"
#include "frame_display.h"

#define GRAB_TAG "GRAB   "

extern int grab_init(shell_t *shell, frame_display_t *display);
extern void grab_done(shell_t *shell);

extern void grab_update(frame_geometry_t *g);

#endif /* __TVU_GRAB_H__ */
