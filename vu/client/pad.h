/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* Color padding                                                            */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 06-AUG-2007                                                    */
/****************************************************************************/

/*
 * $Revision: 923 $
 * $Date: 2008-02-04 09:54:32 +0100 (lun., 04 févr. 2008) $
 */

#ifndef __TVU_PAD_H__
#define __TVU_PAD_H__

#include <glib.h>
#include "frame_display.h"
#include "pad_list.h"

#define PAD_TAG "PAD    "

typedef struct {
  frame_display_t *display;
  GList *pads;
} pad_t;

extern pad_t *pad_alloc(frame_display_t *display);
extern void pad_destroy(pad_t *pad);

extern pad_list_t *pad_add(pad_t *pad, char *id,
			   frame_t *frame, frame_geometry_t *window,
			   unsigned long color_value, fuzz_t *fuzz,
			   unsigned int gap,
			   unsigned int min_width, unsigned int min_height);

extern pad_list_t *pad_retrieve(pad_t *pad, char *id);

extern int pad_remove(pad_t *pad, char *id);

extern void pad_show(pad_t *pad, char *id, char *hdr);

#endif /* __TVU_PAD_H__ */
