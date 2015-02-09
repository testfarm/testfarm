/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* JPEG image file management                                               */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 27-NOV-2007                                                    */
/****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <malloc.h>
#include <glib.h>
#include <jpeglib.h>

#include "jpeg.h"


#define eprintf(args...) fprintf(stderr, "testfarm-vu (jpeg): " args)


char *jpeg_filename_suffix(char *filename)
{
  char *suffix;

  if ( filename == NULL )
    return NULL;

  suffix = strrchr(filename, '.');
  if ( suffix != NULL ) {
    char *suffix2 = g_ascii_strdown(suffix, -1);

    if ( strcmp(suffix2, ".jpg" ) && strcmp(suffix2, ".jpeg") )
      suffix = NULL;

    g_free(suffix2);
  }

  return suffix;
}


int jpeg_load(char *fname, unsigned int *pwidth, unsigned int *pheight, unsigned char **pbuf)
{
  FILE *f;
  struct jpeg_decompress_struct cinfo;
  struct jpeg_error_mgr jerr;
  int ret = 0;

  /* Open input file */
  if ((f = fopen(fname, "rb")) == NULL) {
    eprintf("Cannot open JPEG file %s: %s\n", fname, strerror(errno));
    return -1;
  }

  /* Init JPEG decompression engine */
  cinfo.err = jpeg_std_error(&jerr);
  jpeg_create_decompress(&cinfo);
  jpeg_stdio_src(&cinfo, f);

  /* Read input file header */
  if ( jpeg_read_header(&cinfo, TRUE) == JPEG_HEADER_OK ) {
    if ( pwidth != NULL )
      *pwidth = cinfo.image_width;
    if ( pheight != NULL )
      *pheight = cinfo.image_height;

    /* Load JPEG data (if requested) */
    if ( pbuf != NULL ) {
      int rowstride;
      unsigned char *ptr;

      /* Start decompression process to RGB24 color space */
      cinfo.out_color_space = JCS_RGB;
      jpeg_start_decompress(&cinfo);

      rowstride = cinfo.output_width * cinfo.output_components;

      *pbuf = ptr = (unsigned char *) malloc(rowstride * cinfo.output_height);

      while (cinfo.output_scanline < cinfo.output_height) {
	jpeg_read_scanlines(&cinfo, &ptr, 1);
	ptr += rowstride;
      }

      /* Finish decompression process */
      jpeg_finish_decompress(&cinfo);
    }
  }
  else {
    eprintf("Cannot read JPEG header of file %s\n", fname);
    ret =-1;
  }

  /* Destroy JPEG decompression engine */
  jpeg_destroy_decompress(&cinfo);

  /* Close input file */
  fclose(f);

  return ret;
}


int jpeg_size(char *fname, unsigned int *pwidth, unsigned int *pheight)
{
  return jpeg_load(fname, pwidth, pheight, NULL);
}
