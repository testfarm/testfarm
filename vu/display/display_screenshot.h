/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* Screenshot Grab Management                                               */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 03-MAR-2008                                                    */
/****************************************************************************/

/*
 * $Revision: 1130 $
 * $Date: 2010-03-31 11:48:21 +0200 (mer., 31 mars 2010) $
 */

#ifndef __TVU_DISPLAY_SCREENSHOT_H__
#define __TVU_DISPLAY_SCREENSHOT_H__

#include "frame_geometry.h"
#include "frame_hdr.h"

extern void display_screenshot_init(GtkWindow *window);
extern void display_screenshot_done(void);

extern void display_screenshot_set_frame(frame_hdr_t *frame);
extern void display_screenshot_set_selection(frame_geometry_t *g);
extern void display_screenshot_set_dir(void);

#endif /* __TVU_DISPLAY_SCREENSHOT_H__ */
