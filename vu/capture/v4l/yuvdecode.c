/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* YUV frame decoder                                                       */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 28-FEB-2008                                                    */
/****************************************************************************/

/*
 * $Revision: 974 $
 * $Date: 2008-02-29 22:35:59 +0100 (ven., 29 févr. 2008) $
 */

#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <errno.h>
#include <unistd.h>
#include <linux/videodev2.h>

#include "yuvdecode.h"

#define eprintf(args...) fprintf(stderr, "testfarm-vu (device-YUV): " args)


typedef struct {
  decoder_t h;
  unsigned long fmt;
  int width;
  int height;
} yuv_t;


static void yuv_destroy(decoder_t *_yuv)
{
  yuv_t *yuv = (yuv_t *) _yuv;

  memset(yuv, 0, sizeof(yuv_t));  /* Ghost pointer paranoia */
  free(yuv);
}


static inline unsigned char clip(int v)
{
  if ( v < 0 )
    v = 0;
  if ( v > 255 )
    v = 255;
  return v;
}


static int yuv_process(decoder_t *_yuv,
		       unsigned char *source_buf, int source_size,
		       unsigned char *target_buf, int target_size)
{
  yuv_t *yuv = (yuv_t *) _yuv;
  int size = yuv->width * yuv->height;
  unsigned char *src, *ptr;
  int uofs, vofs;
  int xi, yi;

  ptr = target_buf;

  if ( yuv->fmt == V4L2_PIX_FMT_YUV420 ) {
    uofs = size;
    vofs = uofs + size/4;
  }
  else {
    vofs = size;
    uofs = vofs + size/4;
  }

  src = source_buf;

  for (yi = 0; yi < yuv->height; yi++) {
    for (xi = 0; xi < yuv->width; xi++) {
      unsigned char y = *(src++);
      int i = ((yi/2) * (yuv->width/2)) + (xi/2);
      unsigned char u = source_buf[uofs+i];
      unsigned char v = source_buf[vofs+i];

      int cc = 9535 * ((int) y - 16);
      int d = (int) u - 128;
      int e = (int) v - 128;

      ptr[0] = clip((cc           + 13074*e ) >> 13);
      ptr[1] = clip((cc - 3203*d  - 6660*e  ) >> 13);
      ptr[2] = clip((cc + 16351*d           ) >> 13);

      ptr += 3;
    }
  }

  return size * 3;
}


decoder_t *yuv_create(unsigned long fmt, int width, int height)
{
  yuv_t *yuv;

  yuv = (yuv_t *) malloc(sizeof(yuv_t));
  memset(yuv, 0, sizeof(yuv_t));

  yuv->h.process = yuv_process;
  yuv->h.destroy = yuv_destroy;

  yuv->fmt = fmt;
  yuv->width = width;
  yuv->height = height;

  return (decoder_t *) yuv;
}
