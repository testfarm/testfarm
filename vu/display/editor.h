/****************************************************************************/
/* TestFarm RFB Interface                                                   */
/* Remote Frame Buffer display - Pattern editor                             */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 14-JAN-2004                                                    */
/****************************************************************************/

/*
 * $Revision: 889 $
 * $Date: 2008-01-08 11:31:39 +0100 (mar., 08 janv. 2008) $
 */

#ifndef __RFB_DISPLAY_EDITOR_H
#define __RFB_DISPLAY_EDITOR_H

#include "frame_hdr.h"
#include "pattern.h"

typedef enum {
  EDITOR_EVENT_MODIFIED=0,
  EDITOR_EVENT_UPDATE,
} editor_event_t;

typedef int editor_func_t(editor_event_t event, pattern_t *pattern);

extern int editor_init(GtkWindow *window, editor_func_t *event_fn);
extern void editor_done(void);

extern void editor_set_frame(frame_hdr_t *frame);
extern void editor_remove_frame(frame_hdr_t *frame);
extern void editor_set_selection(frame_geometry_t *g);

extern int editor_show(pattern_t *pattern);
extern void editor_show_match(int state);

#endif /* __RFB_DISPLAY_EDITOR_H */
