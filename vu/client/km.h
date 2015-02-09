/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* Keyboard/Mouse commands                                                  */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 24-APR-2007                                                    */
/****************************************************************************/

/*
 * $Revision: 1036 $
 * $Date: 2008-12-07 14:18:30 +0100 (dim., 07 déc. 2008) $
 */

#ifndef __TVU_KM_H__
#define __TVU_KM_H__

#include "shell.h"
#include "frame.h"

extern int km_init(shell_t *shell, frame_t *root);
extern void km_done(shell_t *shell);

#endif /* __TVU_KM_H__ */
