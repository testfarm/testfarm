/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* JPEG frame decoder                                                       */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 30-NOV-2007                                                    */
/****************************************************************************/

/*
 * $Revision: 977 $
 * $Date: 2008-03-03 10:39:17 +0100 (lun., 03 mars 2008) $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <jpeglib.h>

#include "jpegdecode.h"

#define eprintf(args...) fprintf(stderr, "testfarm-vu (device-JPEG): " args)


typedef struct {
  struct jpeg_source_mgr src;  /* must be first */
  struct jpeg_decompress_struct cinfo;
  struct jpeg_error_mgr jerr;

  unsigned char *buf;
  int size;
} jd_ctx_t;

typedef struct {
  decoder_t h;
  jd_ctx_t ctx;
} jd_t;



static void jd_init_source(struct jpeg_decompress_struct *cinfo)
{
  jd_ctx_t *ctx = (jd_ctx_t *) cinfo->src;
  cinfo->src->next_input_byte = ctx->buf;
  cinfo->src->bytes_in_buffer = ctx->size;
}


static boolean jd_fill_input_buffer(struct jpeg_decompress_struct *cinfo)
{
  eprintf("*PANIC* No more input data\n");
  exit(1);
}


static void jd_skip_input_data(struct jpeg_decompress_struct *cinfo, long num_bytes)
{
  cinfo->src->next_input_byte += num_bytes;
}


static void jd_term_source (j_decompress_ptr cinfo)
{
  /* nothing */
}


static void jd_destroy(decoder_t *_jd)
{
  jd_t *jd = (jd_t *) _jd;

  jpeg_destroy_decompress(&(jd->ctx.cinfo));

  memset(jd, 0, sizeof(jd_t));  /* Ghost pointer paranoia */
  free(jd);
}


static int jd_process(decoder_t *_jd,
		      unsigned char *source_buf, int source_size,
		      unsigned char *target_buf, int target_size)
{
  jd_t *jd = (jd_t *) _jd;
  struct jpeg_decompress_struct *cinfo = &(jd->ctx.cinfo);
  int ret = 0;

  jd->ctx.buf = source_buf;
  jd->ctx.size = source_size;

  /* Decode and store RGB data */
  if ( jpeg_read_header(cinfo, TRUE) == JPEG_HEADER_OK ) {
    unsigned int height = cinfo->image_height;
    unsigned int bytesperline = cinfo->image_width * cinfo->output_components;
    unsigned int imagesize = height * bytesperline;

    //fprintf(stderr, "-- JPEG header: %ux%u bytesperline=%u imagesize=%u\n", cinfo->image_width, height, bytesperline, imagesize);

    if ( target_size >= imagesize ) {
      cinfo->out_color_space = JCS_RGB;
      jpeg_start_decompress(cinfo);

      ret = 0;

      while ( cinfo->output_scanline < cinfo->output_height ) {
	jpeg_read_scanlines(cinfo, &target_buf, 1);
	target_buf += bytesperline;
	ret += bytesperline;
      }

      jpeg_finish_decompress(cinfo);
    }
    else {
      eprintf("target buffer is too small (%d bytes) to store incomming frame (%d bytes)\n", target_size, imagesize);
      ret = -1;
    }
  }
  else {
    eprintf("error reading JPEG header\n");
    ret = -1;
  }

  return ret;
}


decoder_t *jd_create(int size)
{
  jd_t *jd;
  jd_ctx_t *ctx;

  jd = (jd_t *) malloc(sizeof(jd_t));
  memset(jd, 0, sizeof(jd_t));

  jd->h.process = jd_process;
  jd->h.destroy = jd_destroy;

  ctx = &(jd->ctx);

  ctx->cinfo.err = jpeg_std_error(&(ctx->jerr));
  jpeg_create_decompress(&(ctx->cinfo));

  ctx->src.init_source = jd_init_source;
  ctx->src.fill_input_buffer = jd_fill_input_buffer;
  ctx->src.skip_input_data = jd_skip_input_data;
  ctx->src.resync_to_restart = jpeg_resync_to_restart;
  ctx->src.term_source = jd_term_source;
  ctx->src.bytes_in_buffer = 0;
  ctx->src.next_input_byte = NULL;
  ctx->cinfo.src = &(ctx->src);

  return (decoder_t *) jd;
}
