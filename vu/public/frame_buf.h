/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* Display control buffer                                                   */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 05-JAN-2004                                                    */
/****************************************************************************/

/*
 * $Revision: 957 $
 * $Date: 2008-02-25 14:10:30 +0100 (lun., 25 févr. 2008) $
 */

#ifndef __TVU_FRAME_BUF_H__
#define __TVU_FRAME_BUF_H__

#include "frame_rgb.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct {
  frame_rgb_t rgb;            /* RGB frame buffer */
} frame_buf_t;

extern frame_buf_t *frame_buf_alloc(unsigned int width, unsigned int height, int *_shmid);
extern frame_buf_t *frame_buf_map(int shmid, int readonly);
extern void frame_buf_free(frame_buf_t *fb);

#ifdef __cplusplus
}
#endif

#endif /* __TVU_FRAME_BUF_H__ */
