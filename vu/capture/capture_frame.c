/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* Capture Device - Frame management                                        */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 26-APR-2007                                                    */
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
#include <malloc.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <glib.h>

#include "capture_frame.h"


#define dprintf(args...) //fprintf(stderr, "[CAPTURE_FRAME] " args);

#define eprintf(args...) fprintf(stderr, "testfarm-vu (capture): " args)

#define REFRESH_FULL 0x80


typedef struct {
  int index_in;
  int index_out;
  frame_geometry_t g;
} frame_process_msg_t;


#define R 0
#define G 1
#define B 2

static const unsigned char frame_bayer_pattern[4] = {
  R, G,
  G, B
};


static void frame_process_update(capture_frame_t *capture, unsigned char *buf, frame_geometry_t *_sg,
				 frame_geometry_t *_tg)
{
  capture_dev_t *dev = capture->dev;
  capture_pixfmt_t pixfmt = dev->pixfmt;
  unsigned int pixsize = dev->pixsize;
  frame_geometry_t sg, tg;
  unsigned char *source;
  unsigned int source_rowstride;
  unsigned int source_ofs;
  unsigned char *target;
  unsigned int target_rowstride, target_bpp;
  unsigned int target_ofs;
  unsigned int xstride, ystride;
  unsigned int xbayer, ybayer;
  int xi, yi;

  if ( capture->fb == NULL )
    return;

  sg = *_sg;
  source_rowstride = pixsize * dev->width;
  source_ofs = (source_rowstride * sg.y) + (pixsize * sg.x);
  source = buf + source_ofs;

  xbayer = sg.x % 2;
  ybayer = 2 * (sg.y % 2);

  dprintf("      UPDATE %ux%u+%u+%u (ofs=%d)\n", sg.width, sg.height, sg.x, sg.y, source_ofs);

  target_rowstride = capture->fb->rgb.rowstride;
  target_bpp = capture->fb->rgb.bpp;

  if ( capture->rotate != 0 ) {
    tg.width = sg.height;
    tg.height = sg.width;
    if ( capture->rotate > 0 ) {
      tg.x = dev->height - sg.y - sg.height;
      tg.y = sg.x;
      target_ofs = (target_rowstride * tg.y) + (target_bpp * (tg.x + tg.width - 1));
      xstride = target_rowstride;
      ystride = -target_bpp;
    }
    else {
      tg.x = sg.y;
      tg.y = dev->width - sg.x - sg.width;
      target_ofs = (target_rowstride * (tg.y + tg.height - 1)) + (target_bpp * tg.x);
      xstride = -target_rowstride;
      ystride = +target_bpp;
    }
  }
  else {
    tg = sg;

    xstride = +target_bpp;
    ystride = target_rowstride;

    target_ofs = source_ofs;
  }

  //fprintf(stderr, "      => %ux%u+%u+%u (ofs=%d)\n", tg.width, tg.height, tg.x, tg.y, target_ofs);
  target = capture->fb->rgb.buf + target_ofs;
  *_tg = tg;

  for (yi = 0; yi < sg.height; yi++) {
    unsigned char *psource = source;
    unsigned char *ptarget = target;
    unsigned int bayer = ybayer + xbayer;

    for (xi = 0; xi < sg.width; xi++) {
      switch ( pixfmt ) {
      case CAPTURE_PIXFMT_BGR24:
	ptarget[R] = psource[2];
	ptarget[G] = psource[1];
	ptarget[B] = psource[0];
	break;
      case CAPTURE_PIXFMT_MONO8:
	ptarget[R] = *psource;
	ptarget[G] = *psource;
	ptarget[B] = *psource;
	break;
      case CAPTURE_PIXFMT_BAYER8:
	{
	  unsigned int c = frame_bayer_pattern[bayer];
	  ptarget[R] = 0;
	  ptarget[G] = 0;
	  ptarget[B] = 0;
	  ptarget[c] = *psource;
	  bayer ^= 1;
	}
	break;
      case CAPTURE_PIXFMT_RGB565:
	      ptarget[B] = (psource[0] & 0x1F) << 3;
	      ptarget[G] = (((psource[0] >> 5) & 0x07) | ((psource[1] & 0x07) << 3)) << 2;
	      ptarget[R] = ((psource[1] >> 3) & 0x1F) << 3;
	break;
      default:
	ptarget[R] = psource[0];
	ptarget[G] = psource[1];
	ptarget[B] = psource[2];
      }

      psource += pixsize;
      ptarget += xstride;
    }

    source += source_rowstride;
    target += ystride;
    ybayer ^= 2;
  }
}


static void frame_process_compare(capture_frame_t *capture, frame_process_msg_t *msg)
{
  capture_dev_t *dev = capture->dev;
  unsigned int pixsize = dev->pixsize;
  unsigned int rowstride = pixsize * dev->width;
  unsigned char *buf1 = dev->bufs[msg->index_in].base + capture->window_ofs;
  unsigned char *buf2 = dev->bufs[msg->index_out].base + capture->window_ofs;
  int xi, yi;
  int x0 = -1;
  int y0 = -1;
  int x1 = -1;
  int y1 = -1;

  for (yi = 0; yi < capture->window.height; yi++) {
    unsigned char *ptr1 = buf1;
    unsigned char *ptr2 = buf2;
    int xi0 = -1;
    int xi1 = -1;

    for (xi = 0; xi < capture->window.width; xi++) {
      int i;

      for (i = 0; i < pixsize; i++) {
	if ( ptr1[i] ^ ptr2[i] ) {
	  if ( xi0 < 0 )
	    xi0 = xi;
	  xi1 = xi;
	  break;
	}
      }

      ptr1 += pixsize;
      ptr2 += pixsize;
    }

    if ( xi0 >= 0 ) {
      if ( (x0 < 0) || (x0 > xi0) )
	x0 = xi0;
      if ( x1 < xi1 )
	x1 = xi1;
      if ( y0 < 0 )
	y0 = yi;
      y1 = yi;
    }

    buf1 += rowstride;
    buf2 += rowstride;
  }

  dprintf("      COMPARE (%d,%d) (%d,%d)\n", x0, y0, x1, y1);

  if ( x0 >= 0 ) {
    msg->g.x = capture->window.x + x0;
    msg->g.y = capture->window.y + y0;
    msg->g.width = x1 - x0 + 1;
    msg->g.height = y1 - y0 + 1;
  }
  else {
    msg->g.x = 0;
    msg->g.y = 0;
    msg->g.width = 0;
    msg->g.height = 0;
  }
}


static gpointer frame_process_thread(capture_frame_t *capture)
{
  int running = 1;
  frame_process_msg_t msg;

  msg.index_in = -1;
  msg.index_out = -1;
  msg.g.x = 0;
  msg.g.y = 0;
  msg.g.width = 0;
  msg.g.height = 0;

  while ( running ) {
    unsigned char c;
    int ret;

    ret = read(capture->proc_in[0], &c, 1);

    if ( ret == 1 ) {
      unsigned char index = c & ~REFRESH_FULL;

      if ( index < capture->dev->nbufs ) {  /* Paranoia */
	msg.index_in = index;

	dprintf("----- PROCESS prev=%d cur=%d\n", msg.index_out, msg.index_in);

	if ( (msg.index_out < 0) ||       /* No previous buffer => full refresh */
	     (msg.index_out == index) ||  /* Buffer index rollup => full refresh */
	     (c & REFRESH_FULL) ) {       /* Full refresh requested */
	  msg.g.x = 0;
	  msg.g.y = 0;
	  msg.g.width = capture->dev->width;
	  msg.g.height = capture->dev->height;
	}
	else {
	  frame_process_compare(capture, &msg);
	}

	ret = write(capture->proc_out[1], &msg, sizeof(msg));
	if ( ret == -1 ) {
	  eprintf("Frame processing output queue: write: %s\n", strerror(errno));
	  running = 0;
	}

	msg.index_out = msg.index_in;
      }
      else {
	eprintf("Frame processing input queue: read: got an illegal buffer index (%d)\n", c);
	running = 0;
      }
    }

    /* Input pipe closed ! */
    else if ( ret == 0 ) {
      running = 0;
    }

    /* Input pipe I/O error */
    else if ( ret == -1 ) {
      if ( (errno != EAGAIN) && (errno != EINTR) ) {
	eprintf("Frame processing input queue: read: %s\n", strerror(errno));
	running = 0;
      }
    }
  }

  close(capture->proc_out[1]);
  capture->proc_out[1] = -1;

  return NULL;
}


static gboolean frame_process_out_read(GIOChannel *source, GIOCondition condition, capture_frame_t *capture)
{
  frame_process_msg_t msg;
  int ret;

  if ( condition & G_IO_HUP ) {
    eprintf("Frame processing output queue abnormally closed\n");
    exit(EXIT_FAILURE);
    return FALSE;
  }

  ret = read(capture->proc_out[0], &msg, sizeof(msg));
  if ( ret == sizeof(msg) ) {
    if ( capture->processing > 0 )
      capture->processing--;

    /* Give old frame buffer back to video device */
    if ( msg.index_out >= 0 ) {
      capture->dev->bufs[msg.index_out].busy = 0;
      device_qbuf(capture->dev, msg.index_out);
    }

    /* Update frame buffer */
    if ( msg.g.width > 0 ) {
      frame_geometry_t tg;

      frame_process_update(capture, capture->dev->bufs[msg.index_in].base, &msg.g, &tg);

      /* Call frame update handler */
      if ( capture->update ) {
	capture->update(&tg, capture->data);
      }
    }
  }
  else if ( ret == -1 ) {
    if ( (errno != EAGAIN) && (errno != EINTR) ) {
      eprintf("Frame processing output queue: read: %s\n", strerror(errno));
      return FALSE;
    }
  }
  else if ( ret == 0 ) {
    eprintf("Frame processing output queue: closed\n");
    return FALSE;
  }
  else {
    eprintf("Frame processing output queue: read broken message\n");
    return FALSE;
  }

  return TRUE;
}


static int frame_process_queue(capture_frame_t *capture)
{
  unsigned char index_in;
  unsigned char msg_index_in;
  int ret;

  /* Check and setup buffer index */
  if ( capture->dev->buf_ready < 0 ) {
    eprintf("Frame processing input queue: illegal buffer index\n");
    return -1;
  }
  index_in = capture->dev->buf_ready;

  if ( capture->processing > 1 ) {
    dprintf("----- OVERLOAD index=%u\n", index_in);
    /* Signal capture processing overload to upper layer */
    if ( capture->update )
      capture->update(NULL, capture->data);

    return 0;
  }

  /* Raise Full-Refresh flag if requested */
  msg_index_in = index_in;
  if ( capture->refresh_full ) {
    msg_index_in |= REFRESH_FULL;
    capture->refresh_full = 0;
  }

  ret = write(capture->proc_in[1], &msg_index_in, 1);

  if ( ret == -1 ) {
    eprintf("Frame processing input queue: write: %s\n", strerror(errno));
    return -1;
  }

  if ( ret == 1 ) {
    capture->dev->bufs[index_in].busy = 1;
    capture->dev->buf_ready = -1;
    capture->refresh_requested = 0;
    capture->processing++;
  }

  return 0;
}


static gboolean frame_process_refresh(capture_frame_t *capture)
{
  /* Read now for non-streamed capture device */
  device_read(capture->dev);

  if ( capture->dev->buf_ready >= 0 )
    frame_process_queue(capture);
  else
    capture->refresh_requested = 1;

  return TRUE;
}


static int frame_process_init(capture_frame_t *capture)
{
  GError *error = NULL;

  /* Create frame processing queues */
  if ( pipe(capture->proc_in) == -1 ) {
    eprintf("Cannot create frame processing input queue: %s\n", strerror(errno));
    return -1;
  }

  fcntl(capture->proc_in[0], F_SETFD, FD_CLOEXEC);
  fcntl(capture->proc_in[1], F_SETFD, FD_CLOEXEC);
  fcntl(capture->proc_in[1], F_SETFL, O_NONBLOCK);

  if ( pipe(capture->proc_out) == -1 ) {
    eprintf("Cannot create frame processing output queue: %s\n", strerror(errno));
    return -1;
  }

  fcntl(capture->proc_out[0], F_SETFD, FD_CLOEXEC);
  fcntl(capture->proc_out[0], F_SETFL, O_NONBLOCK);
  fcntl(capture->proc_out[1], F_SETFD, FD_CLOEXEC);

  capture->proc_out_channel = g_io_channel_unix_new(capture->proc_out[0]);
  capture->proc_out_tag = g_io_add_watch(capture->proc_out_channel, G_IO_IN | G_IO_HUP,
					 (GIOFunc) frame_process_out_read, capture);

  /* Start frame processing thread */
  capture->proc_thread = g_thread_create((GThreadFunc) frame_process_thread, capture, TRUE, &error);
  if ( capture->proc_thread == NULL ) {
    eprintf("Cannot create frame processing thread: %s\n", error->message);
    return -1;
  }

  return 0;
}


static void frame_process_done(capture_frame_t *capture)
{
  /* Close frame processing input queue */
  if ( capture->proc_in[1] != -1 ) {
    close(capture->proc_in[1]);
    capture->proc_in[1] = -1;
  }

  /* Wait for frame processing thread termination */
  if ( capture->proc_thread != NULL ) {
    g_thread_join(capture->proc_thread);
    capture->proc_thread = NULL;
  }

  /* Close remaining frame processing queues */
  if ( capture->proc_out_tag > 0 ) {
    g_source_remove(capture->proc_out_tag);
    capture->proc_out_tag = 0;
  }

  if ( capture->proc_out_channel != NULL ) {
    g_io_channel_unref(capture->proc_out_channel);
    capture->proc_out_channel = NULL;
  }

  if ( capture->proc_out[0] != -1 ) {
    close(capture->proc_out[0]);
    capture->proc_out[0] = -1;
  }

  if ( capture->proc_out[1] != -1 ) {
    close(capture->proc_out[1]);
    capture->proc_out[1] = -1;
  }

  if ( capture->proc_in[0] != -1 ) {
    close(capture->proc_in[0]);
    capture->proc_in[0] = -1;
  }
}


static long _frame_set_period(capture_t *_capture, long delay)
{
  capture_frame_t *capture = CAPTURE_FRAME(_capture);

  if ( delay <= 0 ) {
    capture->proc_period = 0;
  }
  else {
    if ( delay < CAPTURE_PERIOD_MIN )
      return -1;
    if ( delay > CAPTURE_PERIOD_MAX )
      return -1;

    capture->proc_period = delay;
  }

  if ( capture->proc_timeout_tag > 0 )
    g_source_remove(capture->proc_timeout_tag);

  if ( capture->proc_period == 0 )
    capture->proc_timeout_tag = 0;
  else
    capture->proc_timeout_tag = g_timeout_add(capture->proc_period, (GSourceFunc) frame_process_refresh, capture);

  return capture->proc_period;
}


static long _frame_get_period(capture_t *capture)
{
  return CAPTURE_FRAME(capture)->proc_period;
}


static void frame_read_event(capture_frame_t *capture)
{
  if ( (capture->dev->buf_ready >= 0) && capture->refresh_requested )
    frame_process_queue(capture);
}


static void _frame_close(capture_t *_capture)
{
  capture_frame_t *capture = CAPTURE_FRAME(_capture);

  if ( capture == NULL )
    return;

  /* Stop frame processing clock */
  _frame_set_period(_capture, 0);

  /* Stop frame processing thread */
  frame_process_done(capture);

  /* Close device */
  if ( capture->dev != NULL ) {
    /* Stop video streaming */
    device_detach(capture->dev);

    device_close(capture->dev);
    capture->dev = NULL;
  }

  free(capture);
}


static capture_t *_frame_open(char *devname, capture_opt_t opt,
			      capture_update_fn *update, void *data)
{
  capture_frame_t *capture;
  int w, h;

  /* Allocate and clear frame device descriptor */
  capture = (capture_frame_t *) malloc(sizeof(capture_frame_t));
  memset(capture, 0, sizeof(capture_frame_t));

  /* Clear i/o hooks */
  capture->proc_period = 0;
  capture->proc_timeout_tag = 0;
  capture->proc_thread = NULL;
  capture->proc_in[0] = capture->proc_in[1] = -1;
  capture->proc_out[0] = capture->proc_out[1] = -1;
  capture->proc_out_channel = NULL;
  capture->proc_out_tag = 0;

  /* Init device */
  capture->dev = device_open(devname, (capture_func) frame_read_event, (void *) capture);
  if ( capture->dev == NULL )
    goto failed;

  /* Set rotating option */
  capture->rotate = 0;
  if ( opt & CAPTURE_OPT_ROTATE ) {
    if ( opt & CAPTURE_OPT_ROTATE_CCW )
      capture->rotate = -1;
    else
      capture->rotate = +1;
  }

  /* Set processing window */
  capture->window.x = 0;
  capture->window.y = 0;
  capture->window.width = capture->dev->width;
  capture->window.height = capture->dev->height;
  capture->window_ofs = 0;

  /* Clear processing flags */
  capture->refresh_full = 1;
  capture->refresh_requested = 1;
  capture->processing = 0;

  /* Init frame processing thread */
  if ( frame_process_init(capture) == -1 )
    goto failed;

  /* Allocate target frame buffer */
  if ( capture->rotate ) {
    w = capture->dev->height;
    h = capture->dev->width;
  }
  else {
    w = capture->dev->width;
    h = capture->dev->height;
  }

  capture->fb = frame_buf_alloc(w, h, &(capture->h.shmid));
  if ( capture->fb == NULL ) {
    eprintf("Failed to allocate frame buffer\n");
    goto failed;
  }

  /* Set name and capabilities */
  capture->h.cap |= CAPTURE_CAP_VIDEO;
  capture->h.name = capture->dev->basename;

  /* Hook frame update handler */
  capture->update = update;
  capture->data = data;

  /* Start video streaming */
  if ( device_attach(capture->dev) == -1 )
    goto failed;

  return (capture_t *) capture;

 failed:
  _frame_close((capture_t *) capture);
  return NULL;
}


static int _frame_set_window(capture_t *_capture, frame_geometry_t *tg)
{
  capture_frame_t *capture = CAPTURE_FRAME(_capture);
  frame_geometry_t sg;

  if ( capture->rotate != 0 ) {
    sg.width = tg->height;
    sg.height = tg->width;
    if ( capture->rotate > 0 ) {
      sg.x = tg->y;
      sg.y = capture->dev->height - tg->x - tg->width;
    }
    else {
      sg.x = capture->dev->width - tg->y - tg->height;
      sg.y = tg->x;
    }
  }
  else {
    sg = *tg;
  }

  capture->window_ofs = (capture->dev->pixsize * ((capture->dev->width * sg.y) + sg.x));
  capture->window = sg;

  return 0;
}


static int _frame_get_window(capture_t *_capture, frame_geometry_t *tg)
{
  capture_frame_t *capture = CAPTURE_FRAME(_capture);
  frame_geometry_t sg = capture->window;

  if ( capture->rotate != 0 ) {
    tg->width = sg.height;
    tg->height = sg.width;
    if ( capture->rotate > 0 ) {
      tg->x = capture->dev->height - sg.y - sg.height;
      tg->y = sg.x;
    }
    else {
      tg->x = sg.y;
      tg->y = capture->dev->width - sg.x - sg.width;
    }
  }
  else {
    *tg = sg;
  }

  return 0;
}


static int _frame_refresh(capture_t *capture)
{
  CAPTURE_FRAME(capture)->refresh_full = 1;
  frame_process_refresh(CAPTURE_FRAME(capture));
  return 0;
}


static int _frame_attr_set(capture_t *capture, char *key, char *value)
{
  if ( CAPTURE_FRAME(capture)->dev == NULL )
    return -1;
  return device_attr_set(CAPTURE_FRAME(capture)->dev, key, value);
}


static capture_attr_t *_frame_attr_get(capture_t *capture, char *key, int *nmemb)
{
  if ( CAPTURE_FRAME(capture)->dev == NULL )
    return NULL;
  return device_attr_get(CAPTURE_FRAME(capture)->dev, key, nmemb);
}


static void _frame_show_status(capture_t *capture, char *hdr)
{
  device_show_status(CAPTURE_FRAME(capture)->dev, hdr);
}


capture_interface_t _capture_interface = {
  open :           _frame_open,
  close :          _frame_close,
  set_window :     _frame_set_window,
  get_window :     _frame_get_window,
  set_period :     _frame_set_period,
  get_period :     _frame_get_period,

  refresh :        _frame_refresh,

  action_key :     NULL,
  action_pointer : NULL,
  action_scroll :  NULL,

  attr_set:        _frame_attr_set,
  attr_get:        _frame_attr_get,

  show_status :    _frame_show_status,
};
