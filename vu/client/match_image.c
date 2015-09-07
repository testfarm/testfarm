/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* Image Pattern matching                                                   */
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
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <glib.h>

#include "fuzz.h"
#include "pattern.h"
#include "options.h"
#include "error.h"
#include "frame_display.h"
#include "match.h"
#include "match_image.h"


#define dprintf(args...) //fprintf(stderr, "[MATCH_IMAGE_DEBUG] " args);


#define MATCH_IMAGE_MAX_THREADS 4

extern void match_image_save_map(pattern_t *pattern, unsigned short *pmap);


typedef struct {
  unsigned int index;
  pattern_t *pattern;
  unsigned int model_width, model_height; /* Clipped Model size */

  unsigned char *image_buf;               /* Image RGB buffer pointer */
  unsigned int image_stride;

  unsigned int scan_x;
  unsigned int scan_xmax, scan_ymax;      /* Scan area limits */

  unsigned long map_size;
  unsigned short *map_buf;

  int match_x, match_y;                   /* Match position */
  unsigned long match_badpixels;          /* Match quality */
  unsigned long match_potential;
  gboolean aborted;
} match_job_t;


static match_job_t **match_image_jobs = NULL;
static guint match_image_njobs = 0;

static GThreadPool *match_image_thread_pool = NULL;
static int match_image_reply_pipe[2] = {-1,-1};
static GIOChannel *match_image_reply_channel = NULL;
static guint match_image_reply_tag = 0;


static inline void match_image_line(unsigned char *xbuf,    /* Image ptr */
				    unsigned char *pbuf,    /* Model ptr */
				    unsigned int width,     /* Line width */
				    color_rgb_t fuzz_color, /* Color fuzzing conditions */
				    unsigned short *pmap,   /* Result map ptr */
				    unsigned long *badpixels,
				    unsigned long *potential)
{
  unsigned int xi;

  for (xi = 0; xi < width; xi++) {
    unsigned short r;

    /* Check pixel color is acceptable */
    r = fuzz_weight(fuzz_color, xbuf, pbuf);
    if ( r ) {
      (*badpixels)++;
    }

    *(pmap++) = r;
    (*potential) += r;

    xbuf += FRAME_RGB_BPP;
    pbuf += FRAME_RGB_BPP;
  }
}


static inline void match_image_line_masked(unsigned char *xbuf,    /* Image ptr */
					   unsigned char *pbuf,    /* Model ptr */
					   unsigned int width,     /* Line width */
					   unsigned char *pmask,   /* Mask ptr */
					   color_rgb_t fuzz_color, /* Color fuzzing conditions */
					   unsigned short *pmap,   /* Result map ptr */
					   unsigned long *badpixels,
					   unsigned long *potential)
{
  unsigned int xi;

  for (xi = 0; xi < width; xi++) {
    unsigned short r = 0;

    if ( *pmask ) {
      /* Check pixel color is acceptable */
      r = fuzz_weight(fuzz_color, xbuf, pbuf);
      if ( r ) {
	(*badpixels)++;
      }
    }

    *(pmap++) = r;
    (*potential) += r;

    xbuf += FRAME_RGB_BPP;
    pbuf += FRAME_RGB_BPP;

    pmask++;
  }
}


static void match_image_process(match_job_t *job)
{
  pattern_t *pattern = job->pattern;
  unsigned char *ibuf = job->image_buf + (job->scan_x * FRAME_RGB_BPP);
  unsigned int ibuf_stride = job->image_stride;
  unsigned int width = pattern->d.image.width;
  unsigned char *pmask = pattern->d.image.mask.buf;
  unsigned char *pbuf;
  unsigned short *pmap;
  unsigned int pbuf_stride;
  unsigned int xlen;
  unsigned int y, x;
  color_rgb_t fuzz_color;
  int mx, my;               /* Match position */
  unsigned long badpixels;  /* Match quality */
  unsigned long potential;

  dprintf("match_image_process pattern=%s job=%u scan=%u..%u\n", pattern->id, job->index,
	  job->scan_x, job->scan_x+job->scan_xmax);

  /* The first image line should match */
  pbuf = pattern->d.image.buf;
  pbuf_stride = FRAME_RGB_BPP * width;

  fuzz_color[0] = pattern->d.image.fuzz.color[0];
  fuzz_color[1] = pattern->d.image.fuzz.color[1];
  fuzz_color[2] = pattern->d.image.fuzz.color[2];

  xlen = job->model_width;

  /* Clear match result */
  mx = my = -1;
  badpixels = 0;
  potential = 0;

  pmap = job->map_buf;

  if ( pmask != NULL ) {
    for (y = 0; (y <= job->scan_ymax) && (mx < 0); y++) {
      unsigned char *ybuf = ibuf;

      for (x = 0; (x <= job->scan_xmax) && (mx < 0); x++) {
	unsigned char *xbuf = ybuf;
	unsigned char *pbuf3 = pbuf;
	unsigned char *pmask3 = pmask;
	unsigned short *pmap3 = pmap;
	int yi;

	/* Try matching at this position */
	mx = x;
	my = y;
	badpixels = 0;
	potential = 0;

	/* Check image area against the model */
	for (yi = 0; yi < job->model_height; yi++) {
	  match_image_line_masked(xbuf, pbuf3, xlen, pmask3, fuzz_color,
				  pmap3, &badpixels, &potential);
	  if ( (badpixels > pattern->d.image.badpixels_max) ||
	       (potential > pattern->d.image.potential_max) ) {
	    mx = my = -1;
	    break;
	  }

	  xbuf += ibuf_stride;
	  pbuf3 += pbuf_stride;
	  pmask3 += width;
	  pmap3 += width;
	}

	/* Stop searching if job was aborted */
	if ( job->aborted ) {
	  dprintf("match_image_process ABORTED job=%u\n", job->index);
	  return;
	}

	ybuf += FRAME_RGB_BPP;
      }

      ibuf += ibuf_stride;
    }
  }
  else {
    for (y = 0; (y <= job->scan_ymax) && (mx < 0); y++) {
      unsigned char *ybuf = ibuf;

      for (x = 0; (x <= job->scan_xmax) && (mx < 0); x++) {
	unsigned char *xbuf = ybuf;
	unsigned char *pbuf3 = pbuf;
	unsigned short *pmap3 = pmap;
	int yi;

	/* Try matching at this position */
	mx = x;
	my = y;
	badpixels = 0;
	potential = 0;

	/* Check image area against the model */
	for (yi = 0; yi < job->model_height; yi++) {
	  match_image_line(xbuf, pbuf3, xlen, fuzz_color,
			   pmap3, &badpixels, &potential);
	  if ( (badpixels > pattern->d.image.badpixels_max) ||
	       (potential > pattern->d.image.potential_max) ) {
	    mx = my = -1;
	    break;
	  }

	  xbuf += ibuf_stride;
	  pbuf3 += pbuf_stride;
	  pmap3 += width;
	}

	/* Stop searching if job was aborted */
	if ( job->aborted ) {
	  dprintf("match_image_process ABORTED job=%u\n", job->index);
	  return;
	}

	ybuf += FRAME_RGB_BPP;
      }

      ibuf += ibuf_stride;
    }
  }

  job->match_x = mx;
  job->match_y = my;
  job->match_badpixels = badpixels;
  job->match_potential = potential;

  dprintf("match_image_process DONE job=%u match_x=%d match_y=%d\n", job->index, job->match_x, job->match_y);
}


static void match_image_reply(match_job_t *job)
{
  pattern_t *pattern = job->pattern;
  gboolean last_job = TRUE;
  unsigned int i;

  dprintf("match_image_reply pattern=%s job=%u\n", pattern->id, job->index);

  /* Mark current job as completed */
  job->pattern = NULL;

  /* Determine whether we are in the last job */
  for (i = 0; i < match_image_njobs; i++) {
    match_job_t *job2 = match_image_jobs[i];
    if ( job2->pattern == pattern ) {
      last_job = FALSE;
    }
  }

  if ( (! job->aborted) && (job->match_x >= 0) ) {
    /* Abort other jobs that work on the same request */
    for (i = 0; i < match_image_njobs; i++) {
      match_job_t *job2 = match_image_jobs[i];
      if ( job2->pattern == pattern ) {
        job2->aborted = TRUE;
      }
    }

    /* Return where the pattern is matched */
    pattern->matched.x = pattern->window.x + job->match_x + job->scan_x;
    pattern->matched.y = pattern->window.y + job->match_y;
    pattern->matched.width = job->model_width;
    pattern->matched.height = job->model_height;
    pattern->d.image.badpixels = job->match_badpixels;
    pattern->d.image.potential = job->match_potential;

    /* Record result map if the 'map' option is enabled */
    if ( pattern->d.image.opt & PATTERN_IMAGE_OPT_MAP ) {
      match_image_save_map(pattern, job->map_buf);
    }
  }

  dprintf("  => %s - last_job=%d\n",
	  job->aborted ? "ABORTED" : ((job->match_x >= 0) ? "MATCHED" : "NOT MATCHED"),
	  last_job);

  /* Call match result processing if last job */
  if ( last_job ) {
    dprintf("  => REPLY matched.x=%d matched.y=%d\n", pattern->matched.x, pattern->matched.y);
    match_process_reply(pattern, (pattern->matched.x >= 0));
  }

  dprintf("match_image_reply DONE job=%u\n", job->index);
}


static void match_image_reply_write(match_job_t *job)
{
  write(match_image_reply_pipe[1], &(job->index), sizeof(job->index));
}


static gboolean match_image_reply_read(GIOChannel *source, GIOCondition condition, gpointer user_data)
{
  if ( condition & G_IO_IN ) {
    int ret;
    unsigned int index;
    match_job_t *job;

    ret = read(match_image_reply_pipe[0], &index, sizeof(index));

    if ( ret == sizeof(index) ) {
      if ( index < match_image_njobs ) {
	job = match_image_jobs[index];
	match_image_reply(job);
      }
      else {
	eprintf("*PANIC* Illegal image matching job index received from reply pipe\n");
      }
    }
    else if ( ret == -1 ) {
      if ( (errno != EAGAIN) && (errno != EINTR) ) {
	eprintf("*PANIC* I/O error from reply pipe: %s\n", strerror(errno));
	condition |= G_IO_HUP;
      }
    }
    else if ( ret > 0 ) {
      eprintf("*PANIC* Broken image matching job index received from reply pipe\n");
    }
  }

  if ( condition & G_IO_HUP ) {
    return FALSE;
  }

  return TRUE;
}


static match_job_t *match_image_get_job(pattern_t *pattern)
{
  match_job_t *job = NULL;
  unsigned int index;

  for (index = 0; (index < match_image_njobs) && (job == NULL); index++) {
    job = match_image_jobs[index];
    if ( job->pattern != NULL )
      job = NULL;
  }

  if ( job == NULL ) {
    unsigned int njobs0 = match_image_njobs;

    match_image_njobs += 4;
    match_image_jobs = realloc(match_image_jobs, match_image_njobs * sizeof(*match_image_jobs));

    for (index = njobs0; index < match_image_njobs; index++) {
      job = (match_job_t *) malloc(sizeof(match_job_t));
      job->index = index;
      job->pattern = NULL;
      job->map_size = 0;
      job->map_buf = NULL;
      match_image_jobs[index] = job;
    }

    job = match_image_jobs[njobs0];
  }

  /* Set match source */
  job->pattern = pattern;

  if ( pattern != NULL ) {
    unsigned long size = pattern->d.image.width * pattern->d.image.height * sizeof(*(job->map_buf));

    if ( size > job->map_size ) {
      if ( job->map_buf != NULL )
	free(job->map_buf);
      job->map_size = size;
      job->map_buf = malloc(size);
    }
  }

  /* Clear match result */
  job->match_x = -1;
  job->match_y = -1;
  job->match_badpixels = 0;
  job->match_potential = 0;
  job->aborted = FALSE;

  return job;
}


static void match_image_dup_job(match_job_t *job, match_job_t *job0)
{
  /* Duplicate input parameters */
  job->pattern = job0->pattern;
  job->model_width = job0->model_width;
  job->model_height = job0->model_height;
  job->image_buf = job0->image_buf;
  job->image_stride = job0->image_stride;
  job->scan_x = job0->scan_x;
  job->scan_xmax = job0->scan_xmax;
  job->scan_ymax = job0->scan_ymax;

  /* Clear match result */
  job->match_x = -1;
  job->match_y = -1;
  job->match_badpixels = 0;
  job->match_potential = 0;
  job->aborted = FALSE;
}


int match_image_request(pattern_t *pattern)
{
  frame_rgb_t *rgb;
  frame_geometry_t w;   /* Clipped window */
  match_job_t *job;
  unsigned int scan_width;

  dprintf("match_image_request pattern=%s\n", pattern->id);

  /* Get a job descriptor */
  job = match_image_get_job(pattern);

  /* Get pattern window */
  w = pattern->window;
  pattern->matched.x = -1;
  pattern->matched.y = -1;

  /* Clip model size to the match window */
  job->model_width = pattern->d.image.width;
  if ( job->model_width > w.width )
    job->model_width = w.width;
  job->model_height = pattern->d.image.height;
  if ( job->model_height > w.height )
    job->model_height = w.height;

  /* Compute global scan domain */
  job->scan_x = 0;
  job->scan_xmax = w.width - job->model_width;
  job->scan_ymax = w.height - job->model_height;
  //fprintf(stderr, "   Clipping window:%dx%d+%d+%d image:%dx%d scan:%dx%d\n", w.width, w.height, w.x, w.y, job.model_width, job.model_height, job.scan_xmax, job.scan_ymax);

  /* Set source image */
  rgb = &(pattern->frame->fb->rgb);
  job->image_stride = rgb->rowstride;
  job->image_buf = rgb->buf + (w.y * rgb->rowstride) + (w.x * rgb->bpp);

  /* Process image pattern matching:
     Launch a threaded job if scan area is at least 3 pixels wide */
  scan_width = job->scan_xmax + 1;
  if ( scan_width >= 3 ) {
    unsigned int scan_stride;
    unsigned int scan_x;

    /* Use up to 2 concurent threads by default */
    scan_stride = scan_width / 2;

    if ( opt_quad ) {
      /* Use up to 4 concurent threads if scan slices are wide enough */
      if ( scan_stride >= 16 )
	scan_stride /= 2;
    }

    dprintf("  => PUSH job=%d scan_stride=%u scan_width=%u\n", job->index, scan_stride, scan_width);
    job->scan_xmax = scan_stride;
    g_thread_pool_push(match_image_thread_pool, job, NULL);

    scan_x = scan_stride + 1;
    while ( scan_x < scan_width ) {
      match_job_t *job2 = match_image_get_job(pattern);
      match_image_dup_job(job2, job);

      job2->scan_x = scan_x;

      if ( (scan_x + scan_stride) > scan_width )
	scan_stride = scan_width - scan_x;
      job2->scan_xmax = scan_stride - 1;

      dprintf("  => PUSH job=%d scan_x=%u\n", job2->index, scan_x);
      g_thread_pool_push(match_image_thread_pool, job2, NULL);

      scan_x += scan_stride;
    }
  }
  else {
    dprintf("  => CALL job=%d\n", job->index);
    match_image_process(job);
    match_image_reply(job);
  }

  return 0;
}


int match_image_info(pattern_t *pattern, char *str, int len)
{
  int ret = 0;

  if ( (pattern->d.image.badpixels_rate != 0) && (pattern->d.image.npixels > 0) ) {
    ret += snprintf(str+ret, len-ret, " badpixels=%lu%%",
		    (pattern->d.image.badpixels * 100) / pattern->d.image.npixels);
  }

  if ( (pattern->d.image.potential_rate != 100) && (pattern->d.image.potential_scale > 0) ) {
    ret += snprintf(str+ret, len-ret, " potential=%lu%%",
		    (pattern->d.image.potential * 100) / pattern->d.image.potential_scale);
  }

  return ret;
}


static void match_image_thread(match_job_t *job)
{
  dprintf("match_image_thread pattern=%s job=%u\n", job->pattern->id, job->index);
  match_image_process(job);
  match_image_reply_write(job);
}


int match_image_init(void)
{
  GError *error = NULL;

  /* Create reply pipe */
  if ( pipe(match_image_reply_pipe) == -1 ) {
    eprintf("Cannot create image matching reply pipe: %s\n", strerror(errno));
    return -1;
  }

  /* Prevent reply pipe from being inherited by child programs */
  fcntl(match_image_reply_pipe[0], F_SETFD, FD_CLOEXEC);
  fcntl(match_image_reply_pipe[1], F_SETFD, FD_CLOEXEC);

  /* Handle events from reply pipe */
  fcntl(match_image_reply_pipe[0], F_SETFL, O_NONBLOCK);
  match_image_reply_channel = g_io_channel_unix_new(match_image_reply_pipe[0]);
  match_image_reply_tag = g_io_add_watch(match_image_reply_channel, G_IO_IN | G_IO_HUP,
					 match_image_reply_read, NULL);

  /* Create initial job descriptors pool */
  match_image_get_job(NULL);

  /* Create thread pool */
  match_image_thread_pool = g_thread_pool_new((GFunc) match_image_thread, NULL,
					      MATCH_IMAGE_MAX_THREADS, TRUE,
					      &error);
  if ( match_image_thread_pool == NULL ) {
    eprintf("Cannot create image matching thread pool: %s\n", error ? error->message : "(unknown error)");
    match_image_done();
    return -1;
  }

  return 0;
}


void match_image_done(void)
{
  if ( match_image_thread_pool != NULL ) {
    g_thread_pool_free(match_image_thread_pool, TRUE, TRUE);
    match_image_thread_pool = NULL;
  }

  /* Destroy reply pipe event hendling */
  if ( match_image_reply_tag > 0 ) {
    g_source_remove(match_image_reply_tag);
    match_image_reply_tag = 0;
  }

  if ( match_image_reply_channel != NULL ) {
    g_io_channel_unref(match_image_reply_channel);
    match_image_reply_channel = NULL;
  }

  /* Destroy reply pipe endpoints */
  if ( match_image_reply_pipe[0] > 0 ) {
    close(match_image_reply_pipe[0]);
    match_image_reply_pipe[0] = -1;
  }

  if ( match_image_reply_pipe[1] > 0 ) {
    close(match_image_reply_pipe[1]);
    match_image_reply_pipe[1] = -1;
  }

  /* Free job descriptors pool */
  if ( match_image_jobs != NULL ) {
    unsigned int i;

    for (i = 0; i < match_image_njobs; i++) {
      match_job_t *job = match_image_jobs[i];
      if ( job->map_buf != NULL )
	free(job->map_buf);
      free(job);
    }

    free(match_image_jobs);
    match_image_jobs = NULL;
  }
  match_image_njobs = 0;
}
