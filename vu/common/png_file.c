/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* PNG image file management                                                */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 03-MAR-2008                                                    */
/****************************************************************************/

/*
 * $Revision: 1246 $
 * $Date: 2013-09-06 18:12:46 +0200 (ven., 06 sept. 2013) $
 */

#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <errno.h>
#include <ctype.h>
#include <glib.h>
#include <png.h>

#include "image_file.h"
#include "png_file.h"


#define eprintf(args...) fprintf(stderr, "testfarm-vu (png): " args)


char *png_filename_suffix(char *filename)
{
  char *suffix = NULL;

  if ( filename == NULL )
    return NULL;

  suffix = strrchr(filename, '.');
  if ( suffix != NULL ) {
    char *suffix2 = g_ascii_strdown(suffix, -1);

    if ( strcmp(suffix2, PNG_SUFFIX) )
      suffix = NULL;

    g_free(suffix2);
  }

  /* Include frame time stamp as a part of suffix */
  if ( suffix != NULL ) {
    char *tstamp = NULL;
    char *p = suffix;

    p--;
    while ( (p >= filename) && (isdigit(*p)) ) {
      tstamp = p;
      p--;
    }

    if ( tstamp != NULL ) {
      if ( (p >= filename) && (*p == '.') )
	suffix = p;
    }
  }

  return suffix;
}


char *png_filename(char *filename)
{
  return image_filename(filename, PNG_SUFFIX, png_filename_suffix, "\\.png");
}


int png_load_full(char *fname, unsigned int *pwidth, unsigned int *pheight,
		  unsigned char **pbuf, unsigned char **palpha)
{
  FILE *f;
  unsigned char header[8];
  png_structp png_ptr = NULL;
  png_infop info_ptr = NULL;
  png_uint_32 width, height;
  int bit_depth, color_type, interlace_type;
  int ret = 0;

  /* Open input file */
  if ((f = fopen(fname, "rb")) == NULL) {
    eprintf("%s: Cannot open PNG file: %s\n", fname, strerror(errno));
    return -1;
  }

  /* Check file signature */
  ret = fread(header, 1, sizeof(header), f);
  if ( ret < 0 ) {
    eprintf("%s: Cannot read PNG file: %s\n", fname, strerror(errno));
    goto bailout;
  }

  if ( ret != sizeof(header) ) {
    eprintf("%s: Illegal PNG header\n", fname);
    ret = -1;
    goto bailout;
  }

  if ( png_sig_cmp(header, 0, sizeof(header)) ) {
    eprintf("%s: Not a PNG file\n", fname);
    ret = -1;
    goto bailout;
  }

  ret = 0;

  /* Setup PNG read gears */
  png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if ( png_ptr == NULL ) {
    ret = -1;
    goto bailout;
  }

  info_ptr = png_create_info_struct(png_ptr);
  if ( info_ptr == NULL ) {
    ret = -1;
    goto bailout;
  }

  if ( setjmp(png_jmpbuf(png_ptr)) ) {
    ret = -1;
    goto bailout;
  }

  /* Init file operations */
  png_init_io(png_ptr, f);
  png_set_sig_bytes(png_ptr, sizeof(header));

  /* Get image info */
  png_read_info(png_ptr, info_ptr);
  png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type,
	       &interlace_type, (int *) NULL, (int *) NULL);
  //fprintf(stderr, "-- %s: %lux%lu depth=%d color=%d\n", fname, width, height, bit_depth, color_type);

  if ( pwidth != NULL )
    *pwidth = width;
  if ( pheight != NULL )
    *pheight = height;

  /* We only support RGB images */
  if ( ! (color_type & PNG_COLOR_MASK_COLOR) ) {
    eprintf("%s: Image pixels are not in RGB/RGBA format\n", fname);
    ret = -1;
    goto bailout;
  }

  if ( (pbuf != NULL) && (width > 0) && (height > 0) ) {
    int rowbytes = png_get_rowbytes(png_ptr, info_ptr);
    int bytesperpixel = rowbytes / width;
    png_bytep row_pointers[height];
    unsigned char *buf, *ptr;
    int row;

    //fprintf(stderr, "   rowbytes=%d => bytesperpixel=%d\n", rowbytes, bytesperpixel);

    /* Alloc image buffer */
    ptr = buf = malloc(rowbytes * height);
    for (row = 0; row < height; row++) {
      row_pointers[row] = ptr;
      ptr += rowbytes;
    }

    png_read_image(png_ptr, row_pointers);
    png_read_end(png_ptr, info_ptr);

    if ( color_type & PNG_COLOR_MASK_ALPHA ) {
      int rowbytes2 = 3 * width;
      unsigned char *ptr2;

      //fprintf(stderr, "   rowbytes2=%d\n", rowbytes2);

      ptr2 = *pbuf = malloc(rowbytes2 * height);
      ptr = buf;

      for (row = 0; row < height; row++) {
	unsigned char *xptr = ptr;
	unsigned char *xptr2 = ptr2;
	int x;

	for (x = 0; x < width; x++) {
	  xptr2[0] = xptr[0];
	  xptr2[1] = xptr[1];
	  xptr2[2] = xptr[2];

	  xptr += bytesperpixel;
	  xptr2 += 3;
	}

	ptr += rowbytes;
	ptr2 += rowbytes2;
      }

      /* Set alpha channel buffer */
      if ( palpha != NULL ) {
	int asize = width * height;
	unsigned char *ptr2;
	int i;

	ptr2 = *palpha = malloc(asize);
	ptr = &(buf[3]);

	for (i = 0; i < asize; i++) {
	  *(ptr2++) = *ptr;
	  ptr += bytesperpixel;
	}
      }

      free(buf);
    }
    else {
      *pbuf = buf;

      /* No alpha channel => set alpha buffer pointer to NULL */
      if ( palpha != NULL )
	*palpha = NULL;
    }
  }

  bailout:
  png_destroy_read_struct(&png_ptr, &info_ptr, (png_infop *) NULL);
  fclose(f);

  return ret;
}


int png_load(char *fname, unsigned int *pwidth, unsigned int *pheight,
	     unsigned char **pbuf)
{
  return png_load_full(fname, pwidth, pheight, pbuf, NULL);
}


int png_size(char *fname, unsigned int *pwidth, unsigned int *pheight)
{
  return png_load(fname, pwidth, pheight, NULL);
}


int png_save_buf(unsigned char *rgb_buf, unsigned int rgb_width,
                 frame_geometry_t *g, char *fname)
{
  FILE *f;
  png_structp png_ptr = NULL;
  png_infop info_ptr = NULL;
  int ret = 0;
  unsigned char *ptr;
  int i;

  if ( (f = fopen(fname, "wb")) == NULL ) {
    eprintf("%s: Failed to create PNG file: %s\n", fname, strerror(errno));
    return -1;
  }
  
  png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if ( png_ptr == NULL ) {
    ret = -1;
    goto bailout;
  }

  info_ptr = png_create_info_struct(png_ptr);
  if ( info_ptr == NULL ) {
    ret = -1;
    goto bailout;
  }

  if ( setjmp(png_jmpbuf(png_ptr)) ) {
    ret = -1;
    goto bailout;
  }

  png_init_io(png_ptr, f);

  png_set_IHDR(png_ptr, info_ptr, g->width, g->height, 8, PNG_COLOR_TYPE_RGB,
	       PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

  png_write_info(png_ptr, info_ptr);

  {
   int rowbytes = rgb_width * FRAME_RGB_BPP;
   png_bytep row_pointers[g->height];

    ptr = rgb_buf + (rowbytes * g->y) + (FRAME_RGB_BPP * g->x);
    for (i = 0; i < g->height; i++) {
      row_pointers[i] = ptr;
      ptr += rowbytes;
    }

    png_write_image(png_ptr, row_pointers);
  }

  png_write_end(png_ptr, info_ptr);

 bailout:
  png_destroy_write_struct(&png_ptr, &info_ptr);
  fclose(f);

  return ret;
}


int png_save(frame_rgb_t *rgb, frame_geometry_t *g, char *filename)
{
  frame_geometry_t gg;

  if ( g == NULL ) {
    gg.x = 0;
    gg.y = 0;
    gg.width = rgb->width;
    gg.height = rgb->height;
    g = &gg;
  }

  return png_save_buf(rgb->buf, rgb->width, g, filename);
}
