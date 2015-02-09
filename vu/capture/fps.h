/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* Video Frame Rate statistics                                              */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 04-MAY-2007                                                    */
/****************************************************************************/

/*
 * $Revision: 835 $
 * $Date: 2007-12-04 19:33:15 +0100 (mar., 04 déc. 2007) $
 */

#ifndef __TVU_FPS_H__
#define __TVU_FPS_H__

#define FPS_NFRAMES 100

typedef struct {
  unsigned long long t0;
  unsigned long nframes[FPS_NFRAMES];
} fps_t;

extern void fps_init(fps_t *fps);
extern void fps_update(fps_t *fps, unsigned long long tstamp);
extern unsigned long fps_compute(fps_t *fps);

#endif /* __TVU_FPS_H__ */
