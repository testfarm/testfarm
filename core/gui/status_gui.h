/****************************************************************************/
/* TestFarm                                                                 */
/* Graphical User Interface : status bar                                    */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 10-MAY-2000                                                    */
/****************************************************************************/

/*
 * $Revision: 770 $
 * $Date: 2007-10-09 14:34:18 +0200 (mar., 09 oct. 2007) $
 */

#ifndef __STATUS_GUI_H
#define __STATUS_GUI_H

#include <gtk/gtk.h>

#include "status.h"

extern void status_gui_init(GtkWidget *window);
extern void status_gui_done(void);

#endif /* __STATUS_GUI_H */
