/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* PPM image file management                                                */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 23-DEC-2003                                                    */
/****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <limits.h>
#include <ctype.h>
#include <errno.h>
#include <unistd.h>
#include <zlib.h>

#include "image_file.h"
#include "ppm.h"


#define eprintf(args...) fprintf(stderr, "testfarm-vu (ppm): " args)


int ppm_save_buf(unsigned char *rgb_buf, unsigned int rgb_width,
                 frame_geometry_t *g, char *fname)
{
  gzFile zf;
  unsigned int y;
  unsigned int rowwidth = FRAME_RGB_BPP * g->width;
  unsigned int rowstride = FRAME_RGB_BPP * rgb_width;
  unsigned char *buf;

  if ( (zf = gzopen(fname, "wb6")) == NULL ) {
    eprintf("%s: Failed to create PPM file: %s\n", fname, strerror(errno));
    return -1;
  }

  gzprintf(zf, "P6\n");
  gzprintf(zf, "# Created by TestFarm Virtual User\n");
  gzprintf(zf, "%u %u\n", g->width, g->height);
  gzprintf(zf, "%u\n", (1 << CHAR_BIT) - 1);

  buf = rgb_buf + (rowstride * g->y) + (FRAME_RGB_BPP * g->x);

  for (y = 0; y < g->height; y++) {
    gzwrite(zf, buf, rowwidth);
    buf += rowstride;
  }

  gzclose(zf);

  return 0;
}


char *ppm_filename_suffix(char *filename)
{
  char *suffix = NULL;
  char *tstamp = NULL;
  int len;
  int ofs;

  if ( filename == NULL )
    return NULL;

  len = strlen(filename);

  ofs = len - 3;
  if ( ofs >= 0 ) {
    char *gz = filename + ofs;

    if ( strcmp(gz, ".gz") == 0 ) {
      *gz = '\0';
      ofs = len - 7;
    }
    else {
      gz = NULL;
      ofs = len - 4;
    }

    if ( ofs >= 0 ) {
      suffix = filename + ofs;
      if ( strcmp(suffix, ".ppm") ) {
	suffix = NULL;
	ofs = len;
      }
    }

    if ( gz != NULL ) {
      *gz = '.';
    }
  }
  else {
    ofs = len;
  }

  /* Include frame time stamp as a part of suffix */
  ofs--;
  while ( (ofs >= 0) && (isdigit(filename[ofs])) ) {
    tstamp = filename + ofs;
    ofs--;
  }

  if ( tstamp != NULL ) {
    if ( (ofs >= 0) && (filename[ofs] == '.') )
      suffix = filename + ofs;
  }

  return suffix;
}


char *ppm_filename(char *filename)
{
  return image_filename(filename, PPM_SUFFIX, ppm_filename_suffix, "\\.ppm(\\.gz)?");
}


int ppm_save(frame_rgb_t *rgb, frame_geometry_t *g, char *filename)
{
  frame_geometry_t gg;

  if ( g == NULL ) {
    gg.x = 0;
    gg.y = 0;
    gg.width = rgb->width;
    gg.height = rgb->height;
    g = &gg;
  }

  return ppm_save_buf(rgb->buf, rgb->width, g, filename);
}


static int ppm_gzipped(char *fname)
{
  char *p = strrchr(fname, '.');

  if ( p == NULL )
    return 0;
  return (strcmp(p, ".gz") == 0);
}


int ppm_load(char *fname, unsigned int *pwidth, unsigned int *pheight, unsigned char **pbuf)
{
  int ret;
  FILE *f = NULL;
  gzFile zf = NULL;
  char magic[3];
  int rgb_width = 0;
  int rgb_height = 0;
  int rgb_max = 0;
  unsigned char *rgb_buf = NULL;
  int n;

  /* Open PPM file */
  if ( ppm_gzipped(fname) ) {
    if ( (zf = gzopen(fname, "rb")) == NULL )
      return -1;
  }
  else {
    if ( (f = fopen(fname, "rb")) == NULL )
      return -1;
  }

  /* Check PPM file header */
  if ( zf != NULL )
    n = gzread(zf, magic, 3);
  else
    n = fread(magic, 1, 3, f);

  ret = -2;
  if ( (n == 3) && (magic[0] == 'P') && ((magic[1] == '3') || (magic[1] == '6')) && (magic[2] <= ' ') ) {
    int found = 0;
    int comment = 0;
    int idx = 0;
    char str[32];
    int *tab[3] = {&rgb_width, &rgb_height, &rgb_max};
    int count = (pbuf == NULL) ? 2 : 3; /* Number of header items to be fetched */
    char c;

    while ( found < count ) {
      if ( zf != NULL )
        n = gzread(zf, &c, 1);
      else
        n = fread(&c, 1, 1, f);

      if ( n != 1 )
        break;

      if ( comment ) {
        if ( c == '\n' )
          comment = 0;
      }
      else {
        if ( c == '#' ) {
          comment = 1;
        }
        else if ( c <= ' ' ) {
          str[idx] = '\0';
          *(tab[found]) = atoi(str);

          if ( idx > 0 )
            found++;
          idx = 0;
        }
        else if ( idx < 31 ) {
          str[idx++] = c;
        }
      }
    }

    if ( found == count ) {
      if ( rgb_width <= 0 )
        eprintf("%s: Illegal PPM width\n", fname);
      else if ( rgb_height <= 0 )
        eprintf("%s: Illegal PPM height\n", fname);
      else
        ret = 0;
    }
    else {
      eprintf("%s: Wrong PPM file header\n", fname);
    }
  }
  else {
    eprintf("%s: Not a PPM file\n", fname);
  }

  /* Load PPM data (if requested) */
  if ( (ret == 0) && (pbuf != NULL) ) {
    if ( rgb_max < 256 ) {
      int rgb_size = frame_rgb_bufsize(rgb_width, rgb_height);

      rgb_buf = image_dup(NULL, rgb_width, rgb_height);

      if ( magic[1] == '6' ) {
	if ( zf != NULL )
	  n = gzread(zf, rgb_buf, rgb_size);
	else
	  n = fread(rgb_buf, 1, rgb_size, f);

	if ( n < 0 ) {
	  eprintf("%s: %s\n", fname, strerror(errno));
	  ret = -1;
	}
      }
      else {
	int rgb_ofs = 0;
	char buf[1024];
	char str[80];
	int comment = 0;
	int idx = 0;
	int i;

	do {
	  if ( zf != NULL )
	    n = gzread(zf, buf, sizeof(buf));
	  else
	    n = fread(buf, 1, sizeof(buf), f);

	  for (i = 0; i < n; i++) {
	    char c = buf[i];

	    if ( comment ) {
	      if ( c == '\n' )
		comment = 0;
	    }
	    else {
	      if ( c == '#' ) {
		comment = 1;
	      }
	      else if ( c <= ' ' ) {
		str[idx] = '\0';
		idx = 0;

		if ( (str[0] != '\0') && (rgb_ofs < rgb_size) ) {
		  rgb_buf[rgb_ofs++] = atoi(str);
		}
	      }
	      else if ( idx < 79 ) {
		str[idx++] = c;
	      }
	    }
	  }
	} while ( (n > 0) && (rgb_ofs < rgb_size) );

	str[idx] = '\0';
	if ( (str[0] != '\0') && (rgb_ofs < rgb_size) ) {
	  rgb_buf[rgb_ofs++] = atoi(str);
	}
      }

      /* Rescale RGB values to full 8-bits */
      if ( (ret == 0) && (rgb_max < 255) ) {
	int i;

	for (i = 0; i < rgb_size; i++) {
	  unsigned long pix = rgb_buf[i];
	  pix = (pix * 256) / (rgb_max+1);
	  rgb_buf[i] = pix & 0xFF;
	}
      }
    }
    else {
      eprintf("%s: PPM Color value > 255 is not supported\n", fname);
      ret = -3;
    }
  }

  /* Close PPM file */
  if ( zf != NULL )
    gzclose(zf);
  else
    fclose(f);

  if ( ret == 0 ) {
    if ( pwidth != NULL )
      *pwidth = rgb_width;
    if ( pheight != NULL )
      *pheight = rgb_height;
    if ( pbuf != NULL )
      *pbuf = rgb_buf;
  }
  else {
    if ( rgb_buf != NULL )
      free(rgb_buf);
  }

  return ret;
}


int ppm_size(char *fname, unsigned int *pwidth, unsigned int *pheight)
{
  return ppm_load(fname, pwidth, pheight, NULL);
}
