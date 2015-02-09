/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* Graphical Patterns                                                       */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 23-DEC-2003                                                    */
/****************************************************************************/

/*
 * $Revision: 1152 $
 * $Date: 2010-06-06 10:36:03 +0200 (dim., 06 juin 2010) $
 */

#ifndef __TVU_PATTERN_H__
#define __TVU_PATTERN_H__

#include <glib.h>
#include <pcre.h>

#include "tstamp.h"
#include "frame_geometry.h"
#include "frame_rgb.h"
#include "frame_hdr.h"
#include "pattern_mask.h"
#include "fuzz.h"

typedef enum {
  PATTERN_TYPE_NONE=0,
  PATTERN_TYPE_IMAGE,
  PATTERN_TYPE_TEXT,
  PATTERN_NTYPES
} pattern_type_t;

typedef unsigned int pattern_mode_t;

#define PATTERN_MODE_APPEAR    1
#define PATTERN_MODE_DISAPPEAR 2
#define PATTERN_MODE_RETRIGGER 4
#define PATTERN_MODE_INVERSE   8  /* Video inverse mode for OCR-based text pattern */
#define PATTERN_MODE_BRIEF     16 /* Display brief message when pattern event occurs */
#define PATTERN_MODE_DUMP      32 /* Dump some infos when processing pattern */

typedef unsigned int pattern_flag_t;

#define PATTERN_FLAG_BUSY    1
#define PATTERN_FLAG_REQUEST 2
#define PATTERN_FLAG_REMOVE  4
#define PATTERN_FLAG_USR(_num_)  (8 << (_num_))


#define PATTERN_IMAGE_OPT_MAP    1

typedef struct {
  unsigned int width;           /* Image width */
  unsigned int height;          /* Image height */
  fuzz_t fuzz;                  /* Color fuzz for RGB pixel comparison */
  unsigned char *buf;           /* Image buffer */
  pattern_mask_t mask;          /* Image mask */
  unsigned long npixels;        /* Number of unmasked pixels */
  unsigned long opt;            /* Match options */
  unsigned int  badpixels_rate; /* Acceptable bad pixel rate in % (0..100) */
  unsigned long badpixels_max;  /* Maximum number of acceptable bad pixels */
  unsigned long badpixels;      /* Actual number of bad pixels in the last match */
  unsigned int  potential_rate; /* Acceptable potential rate in % (0..100) */
  unsigned long potential_scale;/* Potential scale for the pattern */
  unsigned long potential_max;  /* Acceptable potential value for the pattern */
  unsigned long potential;      /* Measured potential value in the last match */
} pattern_image_t;

typedef struct {
  pcre *re;                     /* Compiled regular expression */
  pcre_extra *hints;            /* Extra regex study info */
  char *str;
  int *ovec;                    /* Substring capture offset vector */
  int ovec_n;                   /* Number of available elements in ovec */
  frame_geometry_t *mvec;       /* Substring capture geometry vector */
  int mvec_n;                   /* Number of captured elements in mvec */
} pattern_text_t;

typedef struct {
  char *id;                     /* Pattern id */
  frame_hdr_t *frame;           /* Pointer to source frame buffer */
  frame_geometry_t window;      /* Pattern matching window, relative to pattern's frame */
  void *ctx;                    /* Pattern matching context pointer */
  frame_geometry_t matched;     /* Last pattern match location */
  frame_geometry_t matched0;    /* Previous pattern match location */
  tstamp_t t0;                  /* Pattern match request time stamp */
  tstamp_t dt;                  /* Pattern match request/reply time */
  pattern_mode_t mode;          /* Pattern detection mode */
  pattern_flag_t flag;          /* Pattern detection flags */
  int state;                    /* Pattern detection state */
  pattern_type_t type;          /* Type of pattern data */
  char *source;                 /* Pattern source (image file, regex, etc.) */
  union {                       /* Pattern data */
    pattern_image_t image;
    pattern_text_t text;
  } d;
} pattern_t;

extern pattern_t *pattern_alloc(char *id, frame_hdr_t *frame, frame_geometry_t *g, pattern_mode_t mode);
extern int pattern_set_source(pattern_t *pattern, pattern_type_t type, char *source, char **errptr);
extern int pattern_set_options(pattern_t *pattern, GList *options, char **errptr);
extern void pattern_copy(pattern_t *pattern, pattern_t *pattern0);
extern pattern_t *pattern_clone(pattern_t *pattern0);
extern void pattern_free(pattern_t *pattern);

extern int pattern_compare(pattern_t *pattern, char *id);
extern int pattern_diff(pattern_t *p1, pattern_t *p2);
extern int pattern_set_window(pattern_t *pattern, frame_geometry_t *g);

extern int pattern_set_image(pattern_t *pattern, char *filename);

extern int pattern_str(pattern_t *pattern, char *buf, int size);
extern void pattern_show(pattern_t *pattern);

#endif /* __RFB_PATTERN_H__ */
