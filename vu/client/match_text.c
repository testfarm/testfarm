/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* Text Pattern matching                                                    */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 27-JUN-2007                                                    */
/****************************************************************************/

/*
    This file is part of TestFarm,
    the Test Automation Tool for Embedded Software.
    Please visit http://www.testfarm.org.

    TestFarm is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    TestFarm is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with TestFarm.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <string.h>
#include <pcre.h>

#include "log.h"
#include "pattern.h"
#include "error.h"
#include "frame_display.h"
#include "match.h"
#include "match_link.h"
#include "ocr.h"
#include "match_text.h"


static pattern_t *match_text_lookup(match_t *match, unsigned long addr)
{
  GList *p;
  pattern_t *pattern;

  p = match->list;
  pattern = NULL;
  while ( p != NULL ) {
    if ( ((void *) addr) == p->data ) {
      pattern = (pattern_t *) p->data;
      break;
    }
    p = p->next;
  }

  return pattern;
}


static void match_text_reply(unsigned long addr, char *str, ocr_ctx_t *ctx, match_t *match)
{
  pattern_t *pattern;
  pattern_text_t *text;
  int n;

  //fprintf(stderr, "** match_text_reply(addr=0x%08lX str='%s')\n", addr, str);

  /* Lookup pattern descriptor from the match list */
  pattern = match_text_lookup(match, addr);
  if ( pattern == NULL )
    return;
  //fprintf(stderr, "   pattern id='%s' type=%d\n", pattern->id, pattern->type);

  if ( pattern->type != PATTERN_TYPE_TEXT ) {
    eprintf("OCR agent reported data from non-text pattern (%s/%d)\n", pattern->id, pattern->type);
    return;
  }
  text = &(pattern->d.text);

  /* Close OCR request if End-Of-Text appears */
  if ( str == NULL ) {
    //fprintf(stderr, "   >>>>> EOT\n");
    match_process_reply(pattern, (pattern->matched.width > 0));
    return;
  }

  /* Show incoming text if dump mode is enabled */
  if (pattern->mode & PATTERN_MODE_DUMP) {
    frame_geometry_t g;

    if ( ocr_get_text_geometry(ctx, -1, 0, &g) == 0 ) {
      tstamp_t tstamp = tstamp_get();
      int size = 32 + strlen(pattern->id) + strlen(str);
      char buf[size];

      g.x += pattern->window.x;
      g.y += pattern->window.y;
      snprintf(buf, size, "%s DUMP @%s %s\n",
	       pattern->id, frame_geometry_str(&g), str);

      if ( match_link_dump(pattern->id, tstamp, buf) == 0 ) {
	char *hdr = log_hdr(tstamp, MATCH_TAG);
	printf("%s%s", hdr, buf);
      }
    }
  }

  /* Do not perform matching if text already matched */
  if ( pattern->matched.width > 0 ) {
    //fprintf(stderr, " (already matched)\n");
    return;
  }

  /* Do nothing if compiled pattern regex is not present */
  if ( text->re == NULL )
    return;

  /* Alloc regex substring capture vector */
  if ( pattern->d.text.ovec == NULL )
    pattern->d.text.ovec = (int *) malloc(pattern->d.text.ovec_n * sizeof(int));

  /* Perform pattern matching */
  n = pcre_exec(text->re, text->hints,
		str, strlen(str), 0, 0,
		text->ovec, text->ovec_n);
  if ( n > 0 ) {
    frame_geometry_t *g0;
    int i;

    //fprintf(stderr, "   >>>>> MATCHED %d\n", n);

    /* If text has changed, signal its disparition */
    if ( (text->str != NULL) && strcmp(str, text->str) ) {
      match_process_reply(pattern, 0);
    }

    /* Store matched string */
    if ( text->str != NULL )
      free(text->str);
    text->str = strdup(str);

    /* Build table of captured string geometry */
    if ( text->mvec != NULL )
      free(text->mvec);
    text->mvec = (frame_geometry_t *) malloc(n * sizeof(frame_geometry_t));
    text->mvec_n = n;

    for (i = 0; i < n; i++) {
      int offset = text->ovec[2*i+0];
      int len = text->ovec[2*i+1] - offset;
      frame_geometry_t *g = &(text->mvec[i]);
	
      if ( ocr_get_text_geometry(ctx, offset, len, g) < 0 )
	*g = frame_geometry_null;
    }

    g0 = &(text->mvec[0]);
    if ( g0->width > 0 ) {
      /* Set matched window with absolute coordinates */
      //fprintf(stderr, "         geometry=%ux%u+%d+%d\n", g0->width, g0->height, g0->x, g0->y);
      pattern->matched.x = pattern->window.x + g0->x;
      pattern->matched.y = pattern->window.y + g0->y;
      pattern->matched.width = g0->width;
      pattern->matched.height = g0->height;
    }
  }
}


int match_text_request(pattern_t *pattern)
{
  frame_t *frame = (frame_t *) pattern->frame;
  match_t *match = pattern->ctx;
  int ret;

  ret = ocr_request(frame->ocr,
		    (unsigned long) pattern, &(pattern->window), pattern->mode & PATTERN_MODE_INVERSE,
		    (ocr_reply_t *) match_text_reply, match);

  return 0;
}


int match_text_info(pattern_t *pattern, char *str, int len)
{
  pattern_text_t *text;
  int len1;
  int i;

  if ( pattern->type != PATTERN_TYPE_TEXT )
    return 0;
  text = &(pattern->d.text);

  len1 = 0;
  for (i = 1; i < text->mvec_n; i++) {
    frame_geometry_t *g = &(text->mvec[i]);
    int s_offset = text->ovec[2*i+0];
    int s_len = text->ovec[2*i+1] - s_offset;
    char s_buf[2*s_len+1];
    char *p1, *p2;
    int j;

    p1 = text->str + s_offset;
    p2 = s_buf;
    for (j = 0; j < s_len; j++) {
      if ( *p1 == '\'' )
	*(p2++) = '\\';
      *(p2++) = *(p1++);
    }
    *p2 = '\0';

    len1 += snprintf(str+len1, len-len1, " @%ux%u+%d+%d='%s'",
		     g->width, g->height,
		     pattern->window.x + g->x, pattern->window.y + g->y,
		     s_buf);
  }

  return len1;
}
