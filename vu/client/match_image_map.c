/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* Image Pattern matching - Image analysis map generation                   */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 20-DEC-2007                                                    */
/****************************************************************************/

/*
 * $Revision: 1037 $
 * $Date: 2008-12-07 17:37:16 +0100 (dim., 07 déc. 2008) $
 */


#include <stdio.h>
#include <string.h>

#include "png_file.h"
#include "pattern.h"


void match_image_save_map(pattern_t *pattern, unsigned short *pmap)
{
  char *fname = png_filename(pattern->id);
  char *suffix = png_filename_suffix(fname);
  frame_geometry_t g = {
    x: 0,
    y: 0,
    width: pattern->d.image.width,
    height: pattern->d.image.height
  };
  int size1 = g.width * g.height;
  unsigned char *buf, *p;
  FILE *f;
  int i;

  buf = p = (unsigned char *) malloc(FRAME_RGB_BPP * size1);

  for (i = 0; i < size1; i++) {
    unsigned short r = pmap[i];

    if ( r > 255 )
      r = 255;

    p[0] = p[1] = p[2] = r;
    p += FRAME_RGB_BPP;
  }

  png_save_buf(buf, g.width, &g, fname);

  free(buf);

  strcpy(suffix, ".map");

  if ( (f = fopen(fname, "w")) != NULL ) {
    static const unsigned short slice_min[6] = {  1, 24,  72, 168, 360, 744 };
    static const unsigned short slice_max[6] = { 23, 71, 167, 359, 743, 765 };
    unsigned long tab[256*FRAME_RGB_BPP];
    unsigned long slice[6];
    unsigned short r;
    unsigned long max;
    unsigned long cnt;
    unsigned short rmax;
    unsigned short rmed;
    unsigned long rmean;

    fprintf(f, "%s %ux%u : %lu/%lu bad pixels (%lu%%)\n", pattern->id,
	    pattern->d.image.width, pattern->d.image.height,
	    pattern->d.image.badpixels, pattern->d.image.npixels,
	    (pattern->d.image.badpixels * 100) / pattern->d.image.npixels);

    /* Show absolute total potential against pattern */
    fprintf(f, "Cumulative potential: %lu/%lu (%lu%%)\n",
	    pattern->d.image.potential, pattern->d.image.potential_scale,
	    (pattern->d.image.potential * 100) / pattern->d.image.potential_scale);

    /* Compute & Show mean potential */
    rmean = 0;
    if ( pattern->d.image.badpixels > 0 )
      rmean = pattern->d.image.potential / pattern->d.image.badpixels;
    fprintf(f, "Mean potential: %lu\n", rmean);

    /* Compute & Show max potential */
    memset(tab, 0, sizeof(tab));
    rmax = 0;
    max = 0;
    for (i = 0; i < size1; i++) {
      r = pmap[i];

      if ( r > 0 ) {
	unsigned long *pv = &(tab[r]);

	(*pv)++;

	if ( *pv > max )
	  max = *pv;
	if ( r > rmax )
	  rmax = r;
      }
    }

    fprintf(f, "Max potential: %d\n", rmax);

    /* Compute & Show median potential */
    memset(slice, 0, sizeof(slice));
    rmed = 0;
    cnt = 0;
    for (r = 1; r <= rmax; r++) {
      unsigned long v = tab[r];

      for (i = 0; i < 5; i++) {
	if ( r <= slice_max[i] )
	  break;
      }

      slice[i] += v;

      cnt += v;
      if ( (rmed == 0) && (cnt > (pattern->d.image.badpixels_max / 2)) ) {
	rmed = r;
      }
    }

    fprintf(f, "Median potential: %d\n", rmed);

    /* Show potential distribution slices */
    fprintf(f, "\n");
    fprintf(f, "POTENTIAL : PIXELS  : %% BAD PIXELS : %% MAX BAD PIXELS : %% PIXELS\n");
    cnt = 0;
    for (i = 0; i < 6; i++) {
      fprintf(f, "%3d-%3d   : %6lu  : %5.1f%%       : %5.1f%%           : %5.1f%%\n",
	      slice_min[i], slice_max[i], slice[i],
	      (100.0 * slice[i]) / pattern->d.image.badpixels,
	      (100.0 * slice[i]) / pattern->d.image.badpixels_max,
	      (100.0 * slice[i]) / pattern->d.image.npixels);
    }

    /* Show per-potential pixel population */
    fprintf(f, "\n");
    for (r = 1; r <= rmax; r++) {
      unsigned long v = tab[r];

      fprintf(f, "%3d : %3ld : ", r, v);
      for (i = 0; i < ((v * 65) / max); i++)
	fprintf(f, "#");
      if ( r == rmed )
	fprintf(f, " (median potential)");
      if ( r == rmean )
	fprintf(f, " (mean potential)");
      fprintf(f, "\n");
    }

    fclose(f);
  }

  free(fname);
}
