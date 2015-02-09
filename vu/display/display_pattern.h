/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* Display Tool - pattern management                                        */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 22-MAR-2007                                                    */
/****************************************************************************/

/*
 * $Revision: 1131 $
 * $Date: 2010-03-31 16:01:06 +0200 (mer., 31 mars 2010) $
 */

#ifndef __RFB_DISPLAY_PATTERN_H__
#define __RFB_DISPLAY_PATTERN_H__

#include "fuzz.h"
#include "display.h"
#include "frame_hdr.h"

extern int display_pattern_init(void);
extern void display_pattern_done(void);
extern void display_pattern_popup(void);

extern void display_pattern_set_ctl(int sock);
extern void display_pattern_set_selection(frame_geometry_t *g);
extern void display_pattern_set_frame(frame_hdr_t *frame);
extern void display_pattern_remove_frame(frame_hdr_t *frame);

extern void display_pattern_show_selection(void);
extern void display_pattern_clear(void);
extern void display_pattern_update(char *id, frame_hdr_t *frame, char *source, frame_geometry_t *g, unsigned int mode, int type,
				   fuzz_t fuzz, unsigned char loss[2]);
extern void display_pattern_match(display_t *d, char *id, int state);

#endif /* __RFB_DISPLAY_PATTERN_H__ */
