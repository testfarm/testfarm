/****************************************************************************/
/* Basil Dev - TestFarm                                                     */
/* Test Suite generator : Files modification checker                        */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 13-MAY-2004                                                    */
/****************************************************************************/

/*
 * $Revision: 765 $
 * $Date: 2007-10-05 12:53:10 +0200 (ven., 05 oct. 2007) $
 */

#ifndef __CHECK_GUI_H__
#define __CHECK_GUI_H__

#include <glib.h>

typedef void check_gui_spot_t(void *data, unsigned long key);
typedef void check_gui_done_t(void *data);

extern void check_gui_files(GList *list,
			    check_gui_spot_t *spot, void *spot_data,
			    check_gui_done_t *done, void *done_data);

#endif /* __CHECK_GUI_H__ */
