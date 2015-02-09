/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* Display Frame pattern matching                                           */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 23-DEC-2003                                                    */
/****************************************************************************/

/*
 * $Revision: 1131 $
 * $Date: 2010-03-31 16:01:06 +0200 (mer., 31 mars 2010) $
 */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <malloc.h>
#include <unistd.h>
#include <glib.h>

#include "log.h"
#include "error.h"
#include "frame_display.h"
#include "frame.h"
#include "pattern.h"
#include "match_link.h"
#include "match_image.h"
#include "match_text.h"
#include "match.h"


#define _dprintf(args...) //fprintf(stderr, args);
#define dprintf(args...) _dprintf("[DEBUG:MATCH] " args);


/*==================================================================*/
/* Pattern retrieval                                                */
/*==================================================================*/

static pattern_t *match_find(match_t *match, char *id)
{
  GList *l;

  if ( id == NULL )
    return NULL;

  l = g_list_find_custom(match->list, id, (GCompareFunc) pattern_compare);
  return (l != NULL) ? l->data : NULL;
}


/*==================================================================*/
/* Pattern creation                                                 */
/*==================================================================*/

pattern_t *match_add(match_t *match, char *id,
		     frame_t *frame, frame_geometry_t *g,
		     pattern_mode_t mode,
		     pattern_type_t type, char *source)
{
  pattern_t *old_pattern;
  pattern_t *pattern;

  /* Remove the old pattern that may exist with the same id */
  old_pattern = match_find(match, id);
  if ( old_pattern != NULL )
    match_remove_pattern(old_pattern);

  /* Create new pattern */
  pattern = pattern_alloc(id, FRAME_HDR(frame), g, mode);

  /* Set pattern source */
  if ( pattern != NULL ) {
    char *err = NULL;

    if ( pattern_set_source(pattern, type, source, &err) == -1 ) {
      if ( err != NULL ) {
	eprintf("Pattern %s: %s", pattern->id, err);
	free(err);
      }

      pattern_free(pattern);
      pattern = NULL;
    }
  }

  /* Add new pattern to match list */
  if ( pattern != NULL ) {
    pattern->frame = FRAME_HDR(frame);
    pattern->ctx = match;
    match->list = g_list_append(match->list, pattern);
    frame_add_pattern(frame, pattern);
  }

  /* Remove pattern from display if replacement failed */
  if ( (pattern == NULL) && (old_pattern != NULL) )
    frame_display_pattern_remove(match->display, id);

  return pattern;
}


/*==================================================================*/
/* Pattern removal                                                  */
/*==================================================================*/

static void match_remove_pattern_guts(match_t *match, pattern_t *pattern)
{
  /* Remove pattern from frame pattern list */
  if ( pattern->frame != NULL ) {
    frame_remove_pattern((frame_t *) pattern->frame, pattern);
  }

  /* If pattern is busy, mark it for remove */
  if ( pattern->flag & PATTERN_FLAG_BUSY ) {
    pattern->flag |= PATTERN_FLAG_REMOVE;
    dprintf("match_remove_pattern_guts(%s) -> REMOVE : flag=%d\n", pattern->id, pattern->flag);
    match->removed = g_list_prepend(match->removed, pattern);
  }

  /* If pattern is not busy, free pattern descriptor now */
  else {
    pattern_free(pattern);
  }
}


void match_remove_pattern(pattern_t *pattern)
{
  match_t *match = pattern->ctx;

  /* Remove pattern from list */
  match->list = g_list_remove(match->list, pattern);

  /* Free pattern */
  match_remove_pattern_guts(match, pattern);
}


static void match_remove_item(pattern_t *pattern, match_t *match)
{
  /* Print operation */
  printf("%s%s removed\n",  log_hdr_(MATCH_TAG), pattern->id);

  /* Update Display Tool */
  frame_display_pattern_remove(match->display, pattern->id);

  /* Free pattern (or mark it for deletion) */
  match_remove_pattern_guts(match, pattern);
}


int match_remove(match_t *match, char *id)
{
  if ( id == NULL ) {
    g_list_foreach(match->list, (GFunc) match_remove_item, match);
    g_list_free(match->list);
    match->list = NULL;
  }
  else {
    pattern_t *pattern = match_find(match, id);

    if ( pattern == NULL )
      return -1;

    match->list = g_list_remove(match->list, pattern);
    match_remove_item(pattern, match);
  }

  return 0;
}


void match_remove_frame(frame_t *frame)
{
  while ( frame->patterns != NULL ) {
    pattern_t *pattern = frame->patterns->data;
    match_t *match = pattern->ctx;

    match->list = g_list_remove(match->list, pattern);
    match_remove_item(pattern, match);
  }
}


/*==================================================================*/
/* Pattern display                                                  */
/*==================================================================*/

static int match_show_matched(pattern_t *pattern, char *str, int size)
{
  int len;

  len = snprintf(str, size, "@%s", frame_geometry_str(&(pattern->matched)));
  len += snprintf(str+len, size-len, " dt=%llu.%03llums", pattern->dt/1000, pattern->dt%1000);

  /* Specific pattern info */
  switch ( pattern->type ) {
  case PATTERN_TYPE_IMAGE: len += match_image_info(pattern, str+len, size-len); break;
  case PATTERN_TYPE_TEXT:  len += match_text_info(pattern, str+len, size-len); break;
  default:                 break;
  }

  return len;
}


void match_show_pattern(pattern_t *pattern, char *hdr)
{
  match_t *match = pattern->ctx;

  if ( hdr != NULL ) {
    printf("%s", hdr);
    pattern_show(pattern);
    if ( pattern->state ) {
      char str[256];
      match_show_matched(pattern, str, sizeof(str));
      printf(" %s", str);
    }
    printf("\n");
  }

  frame_display_pattern_show(match->display, pattern);
}


int match_show(match_t *match, char *id, char *hdr)
{
  if ( id == NULL ) {
    g_list_foreach(match->list, (GFunc) match_show_pattern, hdr);
  }
  else {
    pattern_t *pattern = match_find(match, id);

    if ( pattern == NULL )
      return -1;

    match_show_pattern(pattern, hdr);
  }

  return 0;
}


/*==================================================================*/
/* Display Tool operations                                          */
/*==================================================================*/

static void match_display_set(void *data, frame_ctl_cmd *cmd)
{
  match_t *match = data;
  char *id = cmd->pattern.id_source;
  char *source = id + strlen(id) + 1;
  char *hdr = log_hdr_(MATCH_TAG);
  frame_t *frame;

  error_default_tag(MATCH_TAG);

  /* Retrieve frame buffer */
  frame = frame_get_by_shmid(match->display->root, cmd->pattern.shmid);

  /* Execute pattern operation */
  if ( *source != '\0' ) {
    pattern_t *pattern = match_add(match, id, frame,
				   &(cmd->pattern.g), cmd->pattern.mode,
				   cmd->pattern.type, source);

    if ( pattern != NULL ) {
      switch (pattern->type) {
      case PATTERN_TYPE_IMAGE:
	      pattern->d.image.fuzz = cmd->pattern.fuzz;
	      pattern->d.image.badpixels_rate = cmd->pattern.loss[0];
	      pattern->d.image.potential_rate = cmd->pattern.loss[1];
	      break;
      default:
	      break;
      }
      printf("\r");
      match_show_pattern(pattern, hdr);
      match_process(pattern, NULL);
    }
    else {
      error(MATCH_TAG, "Failed to add/update pattern from display tool");
    }
  }
  else {
    pattern_t *pattern = match_find(match, id);

    if ( pattern != NULL ) {
      match_remove_pattern(pattern);
      printf("\r%s%s removed\n", hdr, id);
    }
  }
}


static void match_display_get_pattern(pattern_t *pattern, frame_display_t *display)
{
  frame_display_pattern_add(display, pattern);
}


static void match_display_get(void *data, frame_ctl_cmd *cmd)
{
  match_t *match = data;

  /* Send all existing patterns */
  g_list_foreach(match->list, (GFunc) match_display_get_pattern, match->display);

  /* Send all current pattern matches */
  match_show(match, NULL, NULL);
}


/*==================================================================*/
/* Match list creation/destruction                                  */
/*==================================================================*/

match_t *match_alloc(frame_display_t *display)
{
  match_t *match = (match_t *) malloc(sizeof(match_t));
  match->display = display;
  match->list = NULL;
  match->removed = NULL;

  /* Init Display Tool operations handling */
  frame_display_set_pattern_operations(display,
				       match_display_set,
				       match_display_get,
				       match);

  return match;
}

void match_destroy(match_t *match)
{
  if ( match == NULL )
    return;

  match->display = NULL;

  g_list_foreach(match->list, (GFunc) pattern_free, NULL);
  g_list_free(match->list);
  match->list = NULL;

  g_list_foreach(match->removed, (GFunc) pattern_free, NULL);
  g_list_free(match->removed);
  match->removed = NULL;

  free(match);
}


/*==================================================================*/
/* Match list processing                                            */
/*==================================================================*/

static void match_process_request(pattern_t *pattern)
{
  int ret = 0;

  /* Do not process the pattern if it has not to be,
     depending on the detection mode */
  if ( (pattern->mode & PATTERN_MODE_RETRIGGER) == 0 ) {
    if ( pattern->mode & PATTERN_MODE_DISAPPEAR ) {
      if ( pattern->state < 0 )
	ret = 1;
    }
    else if ( pattern->mode & PATTERN_MODE_APPEAR ) {
      if ( pattern->state > 0 )
	ret = 1;
    }
  }

  dprintf("match_process_request(%s) : %d\n", pattern->id, ret);

  if ( ret == 0 ) {
    /* Clear match info */
    pattern->matched = frame_geometry_null;

    pattern->t0 = tstamp_get();
    pattern->dt = 0;

    /* Set BUSY flag */
    pattern->flag |= PATTERN_FLAG_BUSY;
    dprintf("match_process_request(%s) -> BUSY : flag=%d\n", pattern->id, pattern->flag);

    /* Request pattern matching */
    switch ( pattern->type ) {
    case PATTERN_TYPE_IMAGE:
      ret = match_image_request(pattern);
      break;
    case PATTERN_TYPE_TEXT:
      ret = match_text_request(pattern);
      break;
    default:
      ret = -1;
      break;
    }
  }

  if ( ret != 0 ) {
    /* Clear BUSY flag if something went wrong */
    pattern->flag &= ~PATTERN_FLAG_BUSY;
    dprintf("match_process_request(%s) FAILED : flag=%d\n", pattern->id, pattern->flag);

    /* Assume pattern is done if no request in progress */
    frame_pattern_done((frame_t *) pattern->frame, pattern);
  }
}


void match_process_reply(pattern_t *pattern, int result)
{
  match_t *match = pattern->ctx;
  tstamp_t tstamp;
  int changed;

  dprintf("match_process_reply(%s,%d)\n", pattern->id, result);

  /* If pattern is marked for REMOVE, destroy it and abort */
  if ( pattern->flag & PATTERN_FLAG_REMOVE ) {
    match->removed = g_list_remove(match->removed, pattern);
    pattern_free(pattern);
    return;
  }

  /* Clear BUSY flag */
  pattern->flag &= ~PATTERN_FLAG_BUSY;
  dprintf("match_process_reply(%s) -> !BUSY : flag=%d\n", pattern->id, pattern->flag);

  /* Get reply time stamp; Compute request/reply elapsed time */
  tstamp = tstamp_get();
  pattern->dt = tstamp - pattern->t0;

  /* Process pattern matching result */
  changed = 0;
  if ( result ) {
    if ( (pattern->state < 0) ||
	 (memcmp(&(pattern->matched), &(pattern->matched0), sizeof(frame_geometry_t))) )
      changed = +1;
  }
  else {
    if ( pattern->state > 0 )
      changed = -1;
  }

  pattern->matched0 = pattern->matched;
  //fprintf(stderr, "-- match_process_reply(id='%s', result=%d) => changed=%d\n", pattern->id, result, changed);

  if ( changed ) {
    pattern->state = changed;

    if ( ((pattern->mode & PATTERN_MODE_APPEAR)    && (changed > 0)) ||
	 ((pattern->mode & PATTERN_MODE_DISAPPEAR) && (changed < 0)) ) {
      int id_len = strlen(pattern->id);
      int size = id_len + 256;
      char str[size];
      int len = 0;

      size -= 3;  /* Keep room for extra separators */
      len += snprintf(str+len, size-len, "%s ", pattern->id);

      if ( changed > 0 ) {
	/* Update pattern state in the display tool */
	frame_display_pattern_show(match->display, pattern);

	/* Report pattern apparition */
	len += snprintf(str+len, size-len, "APPEAR");
	if ( ! (pattern->mode & PATTERN_MODE_BRIEF) ) {
	  str[len++] = ' ';
	  len += match_show_matched(pattern, str+len, size-len);
	}
      }
      else {
	/* Update pattern state in the display tool */
	frame_display_pattern_hide(match->display, pattern);

	/* Report pattern disparition */
	len += snprintf(str+len, size-len, "DISAPPEAR");
      }

      str[len++] = '\n';
      str[len] = '\0';

      if ( match_link_dump(pattern->id, tstamp, str) == 0 ) {
	char *hdr = log_hdr(tstamp, MATCH_TAG);
	printf("%s%s", hdr, str);
      }
    }
  }

  /* If pattern is marked for REQUEST, re-issue a pattern matching request */
  if ( pattern->flag & PATTERN_FLAG_REQUEST ) {
    pattern->flag &= ~PATTERN_FLAG_REQUEST;
    dprintf("match_process_reply(%s) -> !REQUEST : flag=%d\n", pattern->id, pattern->flag);
    match_process_request(pattern);
  }
  else {
    frame_pattern_done((frame_t *) pattern->frame, pattern);
  }
}


void match_process(pattern_t *pattern, frame_geometry_t *updated)
{
  match_t *match = pattern->ctx;
  frame_geometry_t g;
  int x0, y0;

  x0 = pattern->frame->g0.x + pattern->window.x;
  y0 = pattern->frame->g0.y + pattern->window.y;

  if ( updated == NULL ) {
    capture_get_window(match->display->root->capture, &g);
    updated = &g;
  }

  /* Do not process the match window if it does not intersect the updated area */
  //fprintf(stderr, "-- Processing %s within %ux%u+%d+%d\n", pattern->id, updated->width, updated->height, updated->x, updated->y);
  if ( (updated->x + updated->width) < x0 )
    return;
  if ( (updated->y + updated->height) < y0 )
    return;
  if ( (x0 + pattern->window.width) < updated->x )
    return;
  if ( (y0 + pattern->window.height) < updated->y )
    return;
  //fprintf(stderr, "   => pattern intersects this area\n");

  /* If pattern matching is busy, mark pending request */
  if ( pattern->flag & PATTERN_FLAG_BUSY ) {
    pattern->flag |= PATTERN_FLAG_REQUEST;
    dprintf("match_process(%s) -> REQUEST : flag=%d\n", pattern->id, pattern->flag);
    return;
  }

  match_process_request(pattern);
}


/*==================================================================*/
/* Pattern reset and refresh                                        */
/*==================================================================*/

static void match_reset_item(pattern_t *pattern, match_t *match)
{
  /* Print operation */
  printf("%s%s RESET\n", log_hdr_(MATCH_TAG), pattern->id);

  /* Perform pattern match clear */
  pattern->state = 0;
  pattern->matched = pattern->matched0 = frame_geometry_null;
  pattern->dt = 0;

  /* Refresh display tool matching indicators */
  frame_display_pattern_show(match->display, pattern);

  /* Request a new pattern matching on the whole frame */
  match_process(pattern, NULL);
}


int match_reset(match_t *match, char *id)
{
  if ( id == NULL ) {
    g_list_foreach(match->list, (GFunc) match_reset_item, match);
  }
  else {
    pattern_t *pattern = match_find(match, id);

    if ( pattern == NULL )
      return -1;

    match_reset_item(pattern, match);
  }

  return 0;
}
