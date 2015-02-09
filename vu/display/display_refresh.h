/****************************************************************************/
/* TestFarm RFB Interface                                                   */
/* Display refresh control                                                  */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 02-APR-2007                                                    */
/****************************************************************************/

/*
 * $Revision: 616 $
 * $Date: 2007-07-06 19:58:55 +0200 (ven., 06 juil. 2007) $
 */

#ifndef __RFB_DISPLAY_REFRESH_H__
#define __RFB_DISPLAY_REFRESH_H__

#include "frame_geometry.h"

extern int display_refresh_init(GtkWindow *window);
extern void display_refresh_done(void);
extern void display_refresh_set_ctl(int sock);
extern void display_refresh_set_selection(frame_geometry_t *g);

extern void display_refresh_now(void);
extern void display_refresh_updated(unsigned int period);

#endif /* __RFB_DISPLAY_REFRESH_H__ */
