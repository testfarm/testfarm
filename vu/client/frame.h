/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* Frame Management                                                         */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 06-NOV-2007                                                    */
/****************************************************************************/

/*
 * $Revision: 1035 $
 * $Date: 2008-12-06 21:01:31 +0100 (sam., 06 déc. 2008) $
 */

#ifndef __TVU_FRAME_H__
#define __TVU_FRAME_H__

#include <glib.h>

#include "tstamp.h"
#include "frame_hdr.h"
#include "pattern.h"
#include "capture.h"
#include "filter.h"
#include "ocr.h"

#define FRAME_TAG "FRAME  "

#define FRAME_REFRESH_LAZY    1
#define FRAME_REFRESH_LOCKED  2

typedef enum {
  FRAME_STATE_IDLE=0,
  FRAME_STATE_UPDATE,
  FRAME_STATE_UPDATED,
  FRAME_STATE_PROCESS,
  FRAME_STATE_DELETED,
} frame_state_t;

typedef struct frame_s frame_t;

struct frame_s {
  frame_hdr_t hdr;       /* Frame standard header */
  capture_t *capture;    /* Root capture device */
  frame_t *parent;       /* Parent frame descriptor */
  frame_geometry_t g;    /* Geometry in parent frame */
  GList *children;       /* List of children frames */
  GList *filters;        /* List of filters to apply when updating */

  void *match;           /* Pattern matching context */
  ocr_t *ocr;            /* OCR agent */
  GList *patterns;       /* Patterns attached to this frame */

  frame_state_t state;        /* Frame processing state */
  tstamp_t t0;                /* Frame processing time */
  tstamp_t dt;
  frame_geometry_t updated;   /* Updated area, in root coordinates */
  unsigned int refresh_flags;        /* Frame refresh flags */
  frame_geometry_t refresh_window;   /* Optimized Display Tool refresh area */
  GList *cur_filter;
};

extern frame_t *frame_alloc_root(capture_t *capture);
extern frame_t *frame_alloc_child(char *id, frame_t *parent, frame_geometry_t *g);
extern void frame_free(frame_t *frame, char *hdr);

extern int frame_set_ocr_agent(frame_t *frame, char *ocr_agent, int ocr_argc, char *ocr_argv[]);
extern char *frame_get_ocr_agent(frame_t *frame);

extern void frame_get_default_geometry(frame_t *frame, frame_geometry_t *g);
extern void frame_clip_geometry(frame_t *frame, frame_geometry_t *g);
extern void frame_show(frame_t *frame, char *hdr);
extern void frame_show_tree(frame_t *frame, char *hdr);

extern void frame_foreach_child(frame_t *frame, GFunc func, gpointer data, int rev);
extern frame_t *frame_get_child_by_id(frame_t *frame, char *id);
extern frame_t *frame_get_child_by_shmid(frame_t *frame, int shmid);
extern frame_t *frame_get_by_shmid(frame_t *root, int shmid);

extern void frame_add_filter(frame_t *frame, filter_t *filter);

extern void frame_add_pattern(frame_t *frame, pattern_t *pattern);
extern void frame_remove_pattern(frame_t *frame, pattern_t *pattern);

extern void frame_update(frame_t *frame, frame_geometry_t *updated);
extern void frame_update_all(frame_t *frame);
extern void frame_pattern_done(frame_t *frame, pattern_t *pattern);

/* Static callback implemented elsewhere,
   used for signaling update events to the Display Tool */
extern void frame_updated(frame_t *frame, frame_geometry_t *g);

#endif /* __TVU_FRAME_H__ */
