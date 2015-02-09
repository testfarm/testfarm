/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* Display control buffer - RGB frame buffer                                */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 28-JUN-2007                                                    */
/****************************************************************************/

/*
 * $Revision: 1016 $
 * $Date: 2008-08-13 16:16:39 +0200 (mer., 13 août 2008) $
 */

#ifndef __TVU_FRAME_RGB_H__
#define __TVU_FRAME_RGB_H__

#include "frame_geometry.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define FRAME_RGB_BPP 3


/* Allocate this amount of unused spare bytes to avoid memory faults
   when accessing buffer using 64-bit MMX instructions. */
#define FRAME_RGB_MMTAIL (8-FRAME_RGB_BPP)


typedef struct {
  unsigned int width, height;          /* RGB frame size */
  unsigned int bpp;                    /* Numbre of bytes per pixel */
  unsigned int rowstride;              /* Number of bytes per row */
  unsigned char buf[1];                /* RGB frame buffer */
} frame_rgb_t;


extern unsigned int frame_rgb_bufsize(unsigned int width, unsigned int height);
extern void frame_rgb_init(frame_rgb_t *rgb, unsigned int width, unsigned int height);

extern void frame_rgb_clip_geometry(frame_rgb_t *rgb, frame_geometry_t *g);
extern int frame_rgb_parse_geometry(frame_rgb_t *rgb, char *str, frame_geometry_t *g);

#ifdef __cplusplus
}
#endif

#endif /* __TVU_FRAME_RGB_H__ */
