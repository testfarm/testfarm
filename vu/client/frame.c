/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* Frame Management                                                         */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 06-NOV-2007                                                    */
/****************************************************************************/

/*
 * $Revision: 1107 $
 * $Date: 2010-01-12 13:35:35 +0100 (mar., 12 janv. 2010) $
 */

#include <stdio.h>
#include <string.h>
#include <malloc.h>

#include "error.h"
#include "filter.h"
#include "frame.h"
#include "match.h"


#define _dprintf(args...) //fprintf(stderr, args);
#define dprintf(args...) _dprintf("[DEBUG:FRAME] " args);


frame_t *frame_alloc_child(char *id, frame_t *parent, frame_geometry_t *g)
{
  frame_buf_t *fb;
  int shmid;
  frame_t *frame;

  /* Clip requested geometry to parent frame size */
  if ( parent != NULL ) {
    frame_clip_geometry(parent, g);
    if ( (g->width == 0) || (g->height == 0) ) {
      error(NULL, "Position of frame %s outside of frame %s (%ux%u)", id,
	    parent->hdr.id, parent->hdr.g0.width, parent->hdr.g0.height);
      return NULL;
    }
  }

  /* Alloc frame buffer */
  fb = frame_buf_alloc(g->width, g->height, &shmid);
  if ( fb == NULL ) {
    error(NULL, "Failed to map buffer for frame %s", id);
    return NULL;
  }

  /* Alloc frame descriptor */
  frame = (frame_t *) malloc(sizeof(frame_t));
  memset(frame, 0, sizeof(frame_t));

  if ( id != NULL )
    frame->hdr.id = strdup(id);
  frame->hdr.shmid = shmid;
  frame->hdr.fb = fb;
  frame->hdr.g0 = *g;

  frame->parent = parent;
  frame->g = *g;

  if ( parent != NULL ) {
    frame->hdr.g0.x += parent->hdr.g0.x;
    frame->hdr.g0.y += parent->hdr.g0.y;
    frame->capture = parent->capture;
    parent->children = g_list_append(parent->children, frame);
  }

  /* Setup OCR agent */
  frame->ocr = ocr_alloc(shmid);

  /* Update frame buffer */
  frame_update_all(frame);

  return frame;
}


frame_t *frame_alloc_root(capture_t *capture)
{
  frame_buf_t *fb;
  frame_t *frame;

  /* Map frame buffer */
  fb = frame_buf_map(capture->shmid, 1);
  if ( fb == NULL ) {
    error(NULL, "Failed to map root frame buffer");
    return NULL;
  }

  /* Alloc frame descriptor */
  frame = (frame_t *) malloc(sizeof(frame_t));
  memset(frame, 0, sizeof(frame_t));

  frame->hdr.id = strdup("(root)");
  frame->hdr.shmid = capture->shmid;
  frame->hdr.fb = fb;
  frame->hdr.g0.x = 0;
  frame->hdr.g0.y = 0;
  frame->hdr.g0.width = fb->rgb.width;
  frame->hdr.g0.height = fb->rgb.height;

  frame->capture = capture;

  frame->g = frame->hdr.g0;

  /* Setup OCR agent */
  frame->ocr = ocr_alloc(capture->shmid);

  return frame;
}


void frame_free(frame_t *frame, char *hdr)
{
  /* Remove frame from parent's children list */
  if ( frame->parent != NULL ) {
    frame->parent->children = g_list_remove(frame->parent->children, frame);
  }

  /* Clear children list */
  g_list_foreach(frame->children, (GFunc) frame_free, hdr);
  g_list_free(frame->children);
  frame->children = NULL;

  /* Free pattern list */
  g_list_free(frame->patterns);
  frame->patterns = NULL;

  /* Display removal message if requested */
  if ( hdr != NULL ) {
    printf("%s%s removed\n", hdr, frame->hdr.id);
  }

  /* If the frame is not in IDLE state, i.e. if a filter or a 
     pattern matching is in progress on it, do not destroy it now,
     but put it in the 'removed' list, so that destruction is deffered
     when the frame will be back IDLE */
  if ( frame->state != FRAME_STATE_IDLE ) {
    frame->state = FRAME_STATE_DELETED;
    return;
  }

  /* Shut down OCR agent */
  if ( frame->ocr != NULL ) {
    ocr_destroy(frame->ocr);
    frame->ocr = NULL;
  }

  /* Free filter chain */
  g_list_foreach(frame->filters, (GFunc) filter_destroy, NULL);
  frame->filters = NULL;

  /* Free frame control buffer */
  if ( frame->hdr.fb != NULL ) {
    frame_buf_free(frame->hdr.fb);
    frame->hdr.fb = NULL;
  }

  /* Free frame identifier */
  if ( frame->hdr.id != NULL ) {
    free(frame->hdr.id);
    frame->hdr.id = NULL;
  }

  frame->hdr.shmid = -1;
  frame->parent = NULL;
  frame->capture = NULL;

  free(frame);
}


int frame_set_ocr_agent(frame_t *frame, char *ocr_agent, int ocr_argc, char *ocr_argv[])
{
  return ocr_set_agent(frame->ocr, ocr_agent, ocr_argc, ocr_argv);
}


char *frame_get_ocr_agent(frame_t *frame)
{
  return ocr_get_agent(frame->ocr);
}


static gboolean frame_check_deleted(frame_t *frame)
{
  if ( frame->state != FRAME_STATE_DELETED )
    return FALSE;

  frame->state = FRAME_STATE_IDLE;
  frame_free(frame, NULL);

  return TRUE;
}


void frame_get_default_geometry(frame_t *frame, frame_geometry_t *g)
{
  if ( frame->parent == NULL ) {
    /* If root frame, take the active window of capture device */
    capture_get_window(frame->capture, g);
  }
  else {
    /* If not root frame, take the whole frame size */
    g->x = 0;
    g->y = 0;
    g->width = frame->g.width;
    g->height = frame->g.height;
  }
}


void frame_clip_geometry(frame_t *frame, frame_geometry_t *g)
{
  if ( (g->width == 0) || (g->height == 0) ) {
    frame_get_default_geometry(frame, g);
  }

  frame_rgb_clip_geometry(&(frame->hdr.fb->rgb), g);
}


typedef struct {
  char *hdr;
  int recursive;
  int level;
} frame_show_data_t;


static void frame_show_filters(frame_t *frame, frame_show_data_t *data)
{
  int size = strlen(data->hdr) + strlen(frame->hdr.id) + 4;
  char *hdr2 = malloc(size);

  snprintf(hdr2, size, "%s%s |", data->hdr, frame->hdr.id);
  g_list_foreach(frame->filters, (GFunc) filter_show, hdr2);
  free(hdr2);
}


void frame_show_item(frame_t *frame, frame_show_data_t *data)
{
  int i;

  printf("%s", data->hdr);

  for (i = 0; i < data->level; i++)
    printf("+ ");

  printf("%s", frame->hdr.id);
  if ( frame->parent != NULL )
    printf(" frame=%s", frame->parent->hdr.id);
  printf(" window=%s", frame_geometry_str(&(frame->g)));
  printf(" shmid=%d", frame->hdr.shmid);
  if ( frame->refresh_flags & FRAME_REFRESH_LAZY )
    printf(" lazy");
  if ( frame->dt > 0 )
    printf(" dt=%llu.%03llums", frame->dt/1000, frame->dt%1000);
  printf("\n");

  if ( frame->filters != NULL )
    frame_show_filters(frame, data);
}


void frame_show_tree_guts(frame_t *frame, frame_show_data_t *data)
{
  GList *l;

  frame_show_item(frame, data);

  if ( data->level >= 0 )
    data->level++;

  l = frame->children;
  while ( l ) {
    frame_t *child = l->data;
    frame_show_tree_guts(child, data);
    l = g_list_next(l);
  }

  if ( data->level >= 0 )
    data->level--;
}


void frame_show_tree(frame_t *frame, char *hdr)
{
  frame_show_data_t data = { hdr, 1, 0 };
  frame_show_tree_guts(frame, &data);
}


void frame_show(frame_t *frame, char *hdr)
{
  frame_show_data_t data = { hdr, 0, -1 };
  frame_show_item(frame, &data);
}


void frame_foreach_child(frame_t *frame, GFunc func, gpointer data, int rev)
{
  GList *l = frame->children;

  while ( l ) {
    frame_t *child = l->data;
    if ( ! rev )
      func(child, data);
    frame_foreach_child(child, func, data, rev);
    if ( rev )
      func(child, data);
    l = g_list_next(l);
  }
}


static int frame_compare_by_id(frame_t *frame, char *id)
{
  return
    (frame->hdr.id != id) &&
    ((frame->hdr.id == NULL) || strcmp(frame->hdr.id, id));
}


static int frame_compare_by_shmid(frame_t *frame, int shmid)
{
  return (frame->hdr.shmid != shmid);
}


static frame_t *frame_list_find(GList *list, GCompareFunc compare, gpointer data)
{
  GList *l = list;
  frame_t *frame = NULL;

  while ( (l != NULL) && (frame == NULL) ) {
    frame = l->data;

    if ( compare(frame, data) ) {
      frame = frame_list_find(frame->children, compare, data);
    }

    l = g_list_next(l);
  }

  return frame;
}


frame_t *frame_get_child_by_id(frame_t *frame, char *id)
{
  return frame_list_find(frame->children, (GCompareFunc) frame_compare_by_id, id);
}


frame_t *frame_get_child_by_shmid(frame_t *frame, int shmid)
{
  return frame_list_find(frame->children, (GCompareFunc) frame_compare_by_shmid, GINT_TO_POINTER(shmid));
}


frame_t *frame_get_by_shmid(frame_t *root, int shmid)
{
  if ( shmid == root->hdr.shmid )
    return root;
  else
    return frame_get_child_by_shmid(root, shmid);
}


/*==================================================================*/
/* Filters management                                               */
/*==================================================================*/

static void frame_update_next(frame_t *frame);


void frame_add_filter(frame_t *frame, filter_t *filter)
{
  /* Append filter to frame filter list */
  frame->filters = g_list_append(frame->filters, filter);

  /* Set filter completion callback */
  filter_set_callback(filter, (filter_callback_func) frame_update_next, frame);

  filter->index = g_list_length(frame->filters);
}


/*==================================================================*/
/* Patterns management                                              */
/*==================================================================*/

static void frame_setup_ocr(frame_t *frame)
{
  GList *l = frame->patterns;
  gboolean have_text_pattern = FALSE;

  /* Check if frame has some text patterns */
  while ( l ) {
    pattern_t *pattern = l->data;

    if ( pattern->type == PATTERN_TYPE_TEXT )
      have_text_pattern = TRUE;
    
    l = g_list_next(l);
  }

  /* Enable OCR management if at least one text pattern is present */
  ocr_enable(frame->ocr, have_text_pattern);
}


void frame_add_pattern(frame_t *frame, pattern_t *pattern)
{
  frame->patterns = g_list_append(frame->patterns, pattern);
  frame_setup_ocr(frame);
}


void frame_remove_pattern(frame_t *frame, pattern_t *pattern)
{
  frame->patterns = g_list_remove(frame->patterns, pattern);
  frame_setup_ocr(frame);
}


/*==================================================================*/
/* Requests completion events                                       */
/*==================================================================*/

static void frame_update_start(frame_t *frame);


static void frame_process_completed(frame_t *frame)
{
  /* Destroy frame if marked for deletion */
  if ( frame_check_deleted(frame) )
    return;

  /* Switch back to IDLE state if we are in PROCESS state */
  if ( frame->state != FRAME_STATE_PROCESS )
    return;

  /* Compute frame processing time */
  frame->dt = tstamp_get() - frame->t0;

  /* All conditions are met at this point,
     so we can switch to IDLE state */
  dprintf("frame_process_completed(%s) -> IDLE\n", frame->hdr.id);
  frame->state = FRAME_STATE_IDLE;

  /* Restart frame processing if an update is pending */
  if ( (frame->updated.width > 0) && (frame->updated.height > 0) ) {
    frame_update_start(frame);
  }
}


void frame_pattern_done(frame_t *frame, pattern_t *pattern)
{
  dprintf("frame_pattern_done(%s) : pattern=%s\n", frame->hdr.id, pattern->id);
  frame_process_completed(frame);
}


static void frame_process_start(frame_t *frame)
{
  /* Process patterns attached to this frame.
     All patterns are allowed to be processed together:
     The pattern processing completion rendez-vous
     is in function frame_pattern_done() */
  g_list_foreach(frame->patterns, (GFunc) match_process, &(frame->refresh_window));

  /* Recursively update children frames.
     This can be done in parallel with pattern processing */
  g_list_foreach(frame->children, (GFunc) frame_update, &(frame->refresh_window));

  /* Some pattern matching functions may be sequential:
     call the process completion function here to
     ensure all got their jobs done */
  dprintf("frame_process_start(%s) -> PROCESS\n", frame->hdr.id);
  frame->state = FRAME_STATE_PROCESS;
  frame_process_completed(frame);
}


static void frame_update_completed(frame_t *frame)
{
  /* Mark lazy frame has updated */
  if ( frame->refresh_flags & FRAME_REFRESH_LAZY )
    frame->refresh_flags |= FRAME_REFRESH_LOCKED;

  /* At this point, all filters are completed,
     so we can switch to UPDATED state
     and send a refresh event to the Display Tool */
  dprintf("frame_update_completed(%s) -> UPDATED\n", frame->hdr.id);
  frame->state = FRAME_STATE_UPDATED;
  frame_updated(frame, &(frame->refresh_window));

  frame_process_start(frame);
}


static void frame_update_next(frame_t *frame)
{
  /* Destroy frame if marked for deletion */
  if ( frame_check_deleted(frame) )
    return;

  if ( frame->cur_filter != NULL ) {
    filter_t *filter = frame->cur_filter->data;

    frame->cur_filter = g_list_next(frame->cur_filter);

    /* Filter processing function will call back frame_update_next()
       when finished */
    dprintf("frame_update_next(%s) : FILTER |%d\n", frame->hdr.id, filter->index);
    filter_apply(filter, &(frame->hdr));
  }
  else {
    frame_update_completed(frame);
  }
}


/*==================================================================*/
/* Update event                                                     */
/*==================================================================*/

static void frame_update_start(frame_t *frame)
{
  frame_geometry_t source_window;

  /* Do nothing if frame does not intersect the updated area */
  if ( ! frame_geometry_intersect(&(frame->updated), &(frame->hdr.g0), &source_window) )
    return;

  /* Do nothing if lazy frame has already been updated previously */
  if ( frame->refresh_flags & FRAME_REFRESH_LOCKED )
    return;

  /* Frame should be in IDLE state to be updated without
     disturbing pattern matching and/or children frames */
  if ( frame->state != FRAME_STATE_IDLE ) {
    /* TODO: Burst a warning message if realtime mode is enabled */
    // fprintf(stderr, "** FRAME UPDATE OVERFLOW\n");
    dprintf("frame_update_start(%s) : UPDATE OVERFLOW %s ", frame->hdr.id, frame_geometry_str(&(frame->updated)));
    _dprintf("source=%s state=%d\n", frame_geometry_str(&source_window), frame->state);
    return;
  }

  dprintf("frame_update_start(%s) : UPDATE %s ", frame->hdr.id, frame_geometry_str(&(frame->updated)));
  _dprintf("source=%s\n", frame_geometry_str(&source_window));
  frame->updated = frame_geometry_null;
  frame->state = FRAME_STATE_UPDATE;

  /* Set frame processing start date */
  frame->t0 = tstamp_get();

  /* Compute Display Tool refresh area 
     If frame has filters, it may have been completly modified,
     so all the frame should be refreshed.
     Otherwise, only the updated intersection needs to be refreshed */
  if ( frame->filters != NULL )
    frame->refresh_window = frame->hdr.g0;
  else
    frame->refresh_window = source_window;

  /* Paste parent window to frame pix buffer */
  if ( frame->parent != NULL ) {
    frame_geometry_t target_window;
    frame_rgb_t *source, *target;
    unsigned int source_stride, target_stride;
    unsigned char *source_ybuf, *target_ybuf;
    unsigned int x, y;
    unsigned int xlen;

    source = &(frame->parent->hdr.fb->rgb);
    source_stride = source->rowstride;
    source_ybuf = source->buf + (source_stride * source_window.y) + (source->bpp * source_window.x);

    /* Set target frame update window */
    target_window.x = source_window.x - frame->hdr.g0.x;
    target_window.y = source_window.y - frame->hdr.g0.y;
    target_window.width = source_window.width;
    target_window.height = source_window.height;

    target = &(frame->hdr.fb->rgb);
    target_stride = target->rowstride;
    target_ybuf = target->buf + (target_stride * target_window.y) + (target->bpp * target_window.x);

    xlen = source->bpp * source_window.width;

    for (y = 0; y < source_window.height; y++) {
      unsigned char *source_xbuf = source_ybuf;
      unsigned char *target_xbuf = target_ybuf;

      for (x = 0; x < xlen; x++) {
	*(target_xbuf++) = *(source_xbuf++);
      }

      source_ybuf += source_stride;
      target_ybuf += target_stride;
    }
  }

  /* Start filter update pump */
  frame->cur_filter = frame->filters;
  frame_update_next(frame);
}


void frame_update(frame_t *frame, frame_geometry_t *updated)
{
  frame_geometry_union(&(frame->updated), updated);
  frame_update_start(frame);
}


void frame_update_all(frame_t *frame)
{
  /* Force lazy frames to be updated */
  frame->refresh_flags &= ~FRAME_REFRESH_LOCKED;

  frame->updated = frame->hdr.g0;
  frame_update_start(frame);
}
