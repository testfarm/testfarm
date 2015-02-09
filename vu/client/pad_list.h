/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* Color padding                                                            */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 03-AUG-2007                                                    */
/****************************************************************************/

/*
 * $Revision: 923 $
 * $Date: 2008-02-04 09:54:32 +0100 (lun., 04 févr. 2008) $
 */

#ifndef __TVU_PAD_LIST_H__
#define __TVU_PAD_LIST_H__

#include <glib.h>
#include "frame_rgb.h"
#include "frame_geometry.h"
#include "fuzz.h"
#include "frame.h"


typedef struct {
  char *id;
  char *frame_id;
  int x0, y0;
  unsigned int width, height;
  unsigned int gap;
  GList *list;
} pad_list_t;

extern pad_list_t *pad_list_add(char *id,
				frame_t *frame, frame_geometry_t *g,
				unsigned long color_value, fuzz_t *fuzz,
				unsigned int gap,
				unsigned int min_width, unsigned int min_height);

extern void pad_list_destroy(pad_list_t *list);
extern void pad_list_show(pad_list_t *list, char *hdr);
extern frame_geometry_t *pad_list_tab(pad_list_t *list, int *nmemb);

#endif /* __TVU_PAD_LIST_H__ */
