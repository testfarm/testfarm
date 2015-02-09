/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* Display Tool - command entry and history                                 */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 08-AUG-2010                                                    */
/****************************************************************************/

/*
 * $Revision: 1185 $
 * $Date: 2010-08-08 15:34:16 +0200 (dim., 08 août 2010) $
 */

#ifndef __TVU_DISPLAY_COMMAND_H__
#define __TVU_DISPLAY_COMMAND_H__

#include <gtk/gtk.h>

extern void display_command_init(GtkWindow *window);
extern void display_command_done(void);

extern void display_command_connect(int sock);
extern void display_command_disconnect(void);

extern void display_command_history(int id, char *cmd);

#endif /* __TVU_DISPLAY_COMMAND_H__ */
