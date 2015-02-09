/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* V4L1 Capture Device                                                      */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 28-FEB-2008                                                    */
/****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#include <libv4l1-videodev.h>
#include <linux/videodev2.h>

#include "yuvdecode.h"
#include "v4l1dev.h"


#define dprintf(args...) //fprintf(stderr, "[V4L1_DEV] " args);

#define eprintf(args...) fprintf(stderr, "testfarm-vu (v4l1): " args)

static const char *device_default = "/dev/video";


static int xioctl(int fd, int request, void *arg)
{
  int ret;

  do {
    ret = ioctl (fd, request, arg);
  } while ( (ret == -1) && (errno == EINTR));

  return ret;
}


/*===========================================================*/
/* Open/Close capture device                                 */
/*===========================================================*/

static int device_init_dev(char *devname)
{
  struct stat st;
  int fd;

  if ( stat(devname, &st) == -1 ) {
    eprintf("%s: Cannot identify video device: %s\n",
	    devname, strerror(errno));
    return -1;
  }

  if ( ! S_ISCHR(st.st_mode) ) {
    eprintf("%s: Not a device node\n", devname);
    return -1;
  }

  fd = open(devname, O_RDWR, 0);
  if ( fd == -1 ) {
    eprintf("%s: Cannot open video device: %s\n",
	    devname, strerror (errno));
    return -1;
  }

  return fd;
}


static int device_init_input(v4l_device_t *dev, int input_index)
{
  struct video_channel input;

  /* V4L1 API does not allow to retrieve selected input channel,
     so we select input 0 by default */
  if ( input_index < 0 )
    input_index = 0;

  /* Switch input */
  dprintf("%s: Switching input %d\n", dev->h.name, input_index);

  if ( xioctl(dev->fd, VIDIOCSCHAN, &input_index) == -1 ) {
    eprintf("%s: Cannot select video input %d: %s\n",
	    dev->h.name, input_index, strerror(errno));
    return -1;
  }

  dev->input = input_index;

  input.channel = input_index;
  if ( xioctl(dev->fd, VIDIOCGCHAN, &input) == 0 )
    dev->input_str = strdup(input.name);
  else
    dev->input_str = strdup("");

  return 0;
}


static int device_init_format(v4l_device_t *dev)
{
  struct video_window window;
  struct video_picture picture;
  int palette;
  unsigned long pixelformat2 = 0;

  /* Get frame size */
  if ( xioctl(dev->fd, VIDIOCGWIN, &window) == -1 ) {
    eprintf("%s: Cannot get video device window: %s\n",
	    dev->h.name, strerror(errno));
    return -1;
  }

  dprintf("%s: VIDIOCGWIN -> width=%d height=%d\n", dev->h.name, window.width, window.height);
  dev->h.width = window.width;
  dev->h.height = window.height;

  /* Choose an acceptable pixel format */
  if ( xioctl(dev->fd, VIDIOCGPICT, &picture) == -1 ) {
    eprintf("%s: Cannot get video device picture format: %s\n",
	    dev->h.name, strerror(errno));
    return -1;
  }

  dprintf("%s: VIDIOCGPICT -> palette=%d currently selected\n", dev->h.name, picture.palette);
  palette = picture.palette;

  picture.palette = VIDEO_PALETTE_RGB24;
  if ( xioctl(dev->fd, VIDIOCSPICT, &picture) == 0 ) {
    palette = picture.palette;
  }
  else {
    dprintf("%s: VIDIOCSPICT -> palette=%d rejected\n", dev->h.name, picture.palette);
  }

  dev->pixelformat = palette;
  dev->imagesize = dev->h.width * dev->h.height * picture.depth / 8;
  dprintf("%s: Using palette=%d - imagesize=%lu\n", dev->h.name, palette, dev->imagesize);

  switch ( palette ) {
  case VIDEO_PALETTE_RGB24:
    dev->h.pixfmt = CAPTURE_PIXFMT_BGR24;  /* V4L1 RGB is actually BGR */
    pixelformat2 = V4L2_PIX_FMT_BGR24;
    break;
  case VIDEO_PALETTE_YUV420:
    dev->h.pixfmt = CAPTURE_PIXFMT_RGB24;
    pixelformat2 = V4L2_PIX_FMT_YUV420;
    dev->decoder = yuv_create(pixelformat2, dev->h.width, dev->h.height);
    break;
  default:
    eprintf("%s: Video device does not support pixel formats RGB24/YUV420\n",
	    dev->h.name);
    eprintf("%s: Video device only supports format #%d\n",
	    dev->h.name, palette);
    return -1;
    break;
  }

  /* Set pixel format string */
  dev->pixelformat_str[0] = (pixelformat2 >> 0)  & 0xFF;
  dev->pixelformat_str[1] = (pixelformat2 >> 8)  & 0xFF;
  dev->pixelformat_str[2] = (pixelformat2 >> 16) & 0xFF;
  dev->pixelformat_str[3] = (pixelformat2 >> 24) & 0xFF;
  dev->pixelformat_str[4] = '\0';

  /* Check for streaming capabilitiy */
  if ( xioctl(dev->fd, VIDIOCGMBUF, &(dev->mbuf)) == 0 ) {
    dprintf("%s: VIDIOCGMBUF -> size=%d frames=%d\n",
	    dev->h.name, dev->mbuf.size, dev->mbuf.frames);
    if ( dev->mbuf.frames > 0 ) {
      dev->streaming = 1;
      dprintf("%s: Streaming mode enabled\n", dev->h.name);
    }
  }

  return 0;
}


static int device_init_caps(v4l_device_t *dev)
{
  struct video_capability cap;

  /* Get video device capabilities */
  if ( xioctl(dev->fd, VIDIOCGCAP, &cap) == -1 ) {
    if ( errno == EINVAL ) {
      dprintf("%s: Not a V4L1-compliant video device\n",
	      dev->h.name);
    }
    else {
      eprintf("%s: Cannot get video device capabilities: %s\n",
	      dev->h.name, strerror(errno));
    }

    return -1;
  }

  if ( !(cap.type & VID_TYPE_CAPTURE) ) {
    eprintf("%s: Not a video capture device\n",
	    dev->h.name);
    return -1;
  }

  /* Streaming capability will be probed after image format is set */

  dev->hwinfo = strdup(cap.name);

  return 0;
}


capture_dev_t *device_open(char *devname, capture_func callback_func, void *callback_ctx)
{
  int input_index = -1;
  v4l_device_t *dev;
  int fd;

  /* Get optionnal device name and/or input number */
  if ( (devname != NULL) && (*devname != '\0') ) {
    char *s_input;

    devname = strdup(devname);
    s_input = strrchr(devname, ':');
    if ( s_input != NULL ) {
      *(s_input++) = '\0';
      input_index = atoi(s_input);
    }

    if ( *devname == '\0' ) {
      free(devname);
      devname = NULL;
    }
  }

  if ( (devname == NULL) || (*devname == '\0') ) {
    devname = strdup(device_default);
  }

  fd = device_init_dev(devname);
  if ( fd >= 0 ) {
    dev = (v4l_device_t *) malloc(sizeof(v4l_device_t));
    memset(dev, 0, sizeof(v4l_device_t));

    capture_dev_set(CAPTURE_DEV(dev), devname);
  }

  free(devname);

  if ( fd < 0 )
    return NULL;

  dev->fd = fd;
  dev->read_in[0] = -1;
  dev->read_in[1] = -1;
  dev->read_out[0] = -1;
  dev->read_out[1] = -1;

  /* Clear timing measurement */
  fps_init(&dev->fps);

  /* Init capture capabilities */
  if ( device_init_caps(dev) ) {
    device_close(CAPTURE_DEV(dev));
    return NULL;
  }

  /* Switch video input */
  if ( device_init_input(dev, input_index) ) {
    device_close(CAPTURE_DEV(dev));
    return NULL;
  }

  /* Init pixel encoding */
  if ( device_init_format(dev) ) {
    device_close(CAPTURE_DEV(dev));
    return NULL;
  }

  /* Setup I/O events callbacks */
  dev->callback_func = callback_func;
  dev->callback_ctx = callback_ctx;

  return CAPTURE_DEV(dev);
}


void device_close(capture_dev_t *_dev)
{
  v4l_device_t *dev = (v4l_device_t *) _dev;

  /* Destroy image decoder */
  if ( dev->decoder != NULL ) {
    if ( dev->decoder->destroy != NULL ) {
      dev->decoder->destroy(dev->decoder);
    }
    dev->decoder = NULL;
  }

  /* Close capture device */
  if ( dev->fd >= 0 ) {
    if ( close(dev->fd) == -1 )
      eprintf("%s: Failed to close video device: %s",
	      dev->h.name, strerror(errno));
    dev->fd = -1;
  }

  /* Free various information data */
  if ( dev->hwinfo != NULL ) {
    free(dev->hwinfo);
    dev->hwinfo = NULL;
  }

  if ( dev->input_str != NULL ) {
    free(dev->input_str);
    dev->input_str = NULL;
  }

  capture_dev_clear(CAPTURE_DEV(dev));

  free(dev);
}


/*===========================================================*/
/* Manage incoming frames                                    */
/*===========================================================*/

static int device_qbuf_streaming(v4l_device_t *dev, int index)
{
  struct video_mmap mm;

  mm.frame = index;
  mm.width = dev->h.width;
  mm.height = dev->h.height;
  mm.format = dev->pixelformat;

  dprintf("ioctl(VIDIOCMCAPTURE,%d)\n", index);

  if ( xioctl(dev->fd, VIDIOCMCAPTURE, &mm) == -1 ) {
    eprintf("%s: ioctl(VIDIOCMCAPTURE,%d): %s\n",
	    dev->h.name, index, strerror(errno));
    return -1;
  }

  return 0;
}


int device_qbuf(capture_dev_t *_dev, int index)
{
  v4l_device_t *dev = V4L_DEV(_dev);

  if ( index >= dev->h.nbufs ) {
    eprintf("%s: Illegal buffer index %d\n", dev->h.name, index);
    return -1;
  }

  if ( write(dev->read_in[1], &index, 1) != 1 ) {
    eprintf("%s: Read input pipe: write error: %s\n", dev->h.name, strerror(errno));
    exit(EXIT_FAILURE);
  }

  return 0;
}


static void device_read_thread(v4l_device_t *dev)
{
  int working;
  int frame;

  dprintf("Entering read thread\n");

  /* Queue all available frame buffers */
  if ( dev->streaming ) {
    for (frame = 0; frame < dev->mbuf.frames; frame++)
      device_qbuf_streaming(dev, frame);
  }

  frame = 0;
  working = 1;

  while ( working ) {
    unsigned char index;
    int ret = 0;

    /* Get an available output buffer */
    if ( dev->read_in[1] != -1 ) {
      ret = read(dev->read_in[0], &index, 1);
    }

    if ( ret == 1 ) {
      unsigned char *ibuf = NULL;
      int isize = 0;

      dprintf("Read thread input: buf%d\n", index);
      dev->h.bufs[index].bytesused = 0;

      /* Read input buffer from capture device */
      if ( dev->streaming ) {
	dprintf("ioctl(VIDIOCSYNC,%d)\n", frame);
	if ( xioctl(dev->fd, VIDIOCSYNC, &frame) == -1 ) {
	  eprintf("%s: ioctl(VIDIOCSYNC,%d): %s\n", dev->h.name, frame, strerror(errno));
	  exit(EXIT_FAILURE);
	}

	ibuf = dev->buf + dev->mbuf.offsets[frame];
	isize = dev->imagesize;
      }
      else {
	ibuf = dev->buf;
	isize = read(dev->fd, ibuf, dev->imagesize);

	if ( isize < 0 ) {
	  if ( (errno != EAGAIN) && (errno != EINTR) ) {
	    eprintf("%s: read(%lu): %s\n", dev->h.name, dev->imagesize, strerror(errno));
	    exit(EXIT_FAILURE);
	  }
	  else {
	    isize = 0;
	  }
	}
      }

      /* Copy input buffer to output buffer */
      if ( isize > 0 ) {
	int length = 0;

	dprintf("Got %d bytes from capture device\n", isize);

	if ( dev->decoder != NULL ) {
	  if ( dev->decoder->process != NULL ) {
	    length = dev->decoder->process(dev->decoder,
					   ibuf, isize,
					   dev->h.bufs[index].base, dev->h.bufs[index].length);
	  }
	}
	else {
	  length = isize;
	  if ( length > dev->h.bufs[index].length )
	    length = dev->h.bufs[index].length;
	  memcpy(dev->h.bufs[index].base, ibuf, length);
	}

	dev->h.bufs[index].bytesused = length;
      }

      /* Send filled output buffer back for processing */
      if ( dev->streaming ) {
	device_qbuf_streaming(dev, frame);

	frame++;
	if ( frame >= dev->mbuf.frames )
	  frame = 0;
      }

      if ( dev->read_out[1] != -1 ) {
	dprintf("Read thread output: buf%d\n", index);
	if ( write(dev->read_out[1], &index, 1) != 1 ) {
	  eprintf("%s: Read output pipe: write error: %s\n", dev->h.name, strerror(errno));
	  exit(EXIT_FAILURE);
	}
      }
    }
    else if ( ret == 0 ) {
      working = 0;
    }
    else if ( ret == -1 ) {
      if ( (errno != EAGAIN) && (errno != EINTR) ) {
	eprintf("Read input pipe: read error: %s\n", strerror(errno));
	working = 0;
      }
    }
  }

  dprintf("Leaving read thread\n");
}


static void device_update_fps(v4l_device_t *dev)
{
  struct timeval tv;
  unsigned long long tstamp;

  gettimeofday(&tv, NULL);
  tstamp = (((unsigned long long) tv.tv_sec) * 1000000ULL) + tv.tv_usec;
  fps_update(&dev->fps, tstamp);
}


static gboolean device_read_event(GIOChannel *source, GIOCondition condition, v4l_device_t *dev)
{
  unsigned char index;
  int ret;

  if ( condition & G_IO_HUP ) {
    eprintf("%s: Read output pipe abnormally closed\n", dev->h.name);
    exit(EXIT_FAILURE);
    return FALSE;
  }

  ret = read(dev->read_out[0], &index, 1);
  if ( ret == -1 ) {
    if ( (errno != EAGAIN) && (errno != EINTR) ) {
      eprintf("Read output pipe: read error: %s\n", strerror(errno));
      exit(EXIT_FAILURE);
      return FALSE;
    }
  }

  if ( ret == 0 ) {
    eprintf("Read output pipe is broken\n");
    exit(EXIT_FAILURE);
    return FALSE;
  }

  if ( index < dev->h.nbufs ) {
    /* Requeue previously 'ready' buffer if it was not processed */
    if ( dev->h.buf_ready >= 0 ) {
      device_qbuf(CAPTURE_DEV(dev), dev->h.buf_ready);
      dev->h.buf_ready = -1;
    }

    /* Consider newly filled buffer as 'ready' */
    dev->h.buf_ready = index;

    /* Update refresh rate statistics */
    device_update_fps(dev);
  }

  if ( dev->callback_func != NULL ) {
    dev->callback_func(dev->callback_ctx);
  }

  return TRUE;
}


/*===========================================================*/
/* Start / stop video streaming                              */
/*===========================================================*/

static int device_attach_streaming(v4l_device_t *dev)
{
  dprintf("Mapping streaming buffers: %d bytes\n", dev->mbuf.size);

  if ( dev->mbuf.frames < 2 ) {
    eprintf("%s: Insufficient buffer memory\n", dev->h.name);
    return -1;
  }

  dev->buf = mmap(NULL /* start anywhere */,
		  dev->mbuf.size,
		  PROT_READ | PROT_WRITE,
		  MAP_SHARED,
		  dev->fd, 0);
  if ( dev->buf == MAP_FAILED ) {
    eprintf("%s: mmap(%d): %s\n",
	    dev->h.name, dev->mbuf.size, strerror(errno));
    dev->buf = NULL;
    return -1;
  }

  return 0;
}


static int device_attach_readwrite(v4l_device_t *dev)
{
  dprintf("Allocating read buffer: %lu bytes\n", dev->imagesize);

  dev->buf = malloc(dev->imagesize);
  if ( dev->buf == NULL ) {
    eprintf("%s: malloc(%lu): %s\n",
	    dev->h.name, dev->imagesize, strerror(errno));
    return -1;
  }

  return 0;
}


static int device_attach_buffers(v4l_device_t *dev)
{
  int rgbsize;
  int i;

  dev->h.nbufs = 4;
  dev->h.bufs = calloc(dev->h.nbufs, sizeof(*(dev->h.bufs)));
  dev->h.buf_ready = -1;

  rgbsize = dev->h.pixsize * dev->h.width * dev->h.height;

  for (i = 0; i < dev->h.nbufs; i++) {
    capture_buf_t *buf = &(dev->h.bufs[i]);

    buf->base = malloc(rgbsize);
    buf->length = rgbsize;
  }

  return 0;
}


int device_attach(capture_dev_t *_dev)
{
  v4l_device_t *dev = V4L_DEV(_dev);
  GError *error = NULL;
  int ret;
  int i;

  /* Init buffering */
  ret = dev->streaming ?
    device_attach_streaming(dev) :
    device_attach_readwrite(dev);

  if ( ret )
    return -1;

  if ( device_attach_buffers(dev) == -1 )
    return -1;

  /* Create read input pipe */
  if ( pipe(dev->read_in) == -1 ) {
    eprintf("%s: Cannot create read input pipe: %s\n", dev->h.name, strerror(errno));
    return -1;
  }

  fcntl(dev->read_in[0], F_SETFD, FD_CLOEXEC);
  fcntl(dev->read_in[1], F_SETFD, FD_CLOEXEC);
  fcntl(dev->read_in[1], F_SETFL, O_NONBLOCK);

  /* Create read output pipe */
  if ( pipe(dev->read_out) == -1 ) {
    eprintf("%s: Cannot create read output pipe: %s\n", dev->h.name, strerror(errno));
    return -1;
  }

  fcntl(dev->read_out[0], F_SETFD, FD_CLOEXEC);
  fcntl(dev->read_out[0], F_SETFL, O_NONBLOCK);
  fcntl(dev->read_out[1], F_SETFD, FD_CLOEXEC);

  dev->read_out_channel = g_io_channel_unix_new(dev->read_out[0]);
  dev->read_out_tag = g_io_add_watch(dev->read_out_channel, G_IO_IN | G_IO_HUP,
				     (GIOFunc) device_read_event, dev);

  /* Start read thread */
  dev->read_thread = g_thread_create((GThreadFunc) device_read_thread, dev, TRUE, &error);
  if ( dev->read_thread == NULL ) {
    eprintf("%s: Cannot create read thread: %s\n", dev->h.name, error->message);
    return -1;
  }

  /* Queue all output buffers to read thread */
  for (i = 0; i < dev->h.nbufs; i++)
    device_qbuf(CAPTURE_DEV(dev), i);

  return 0;
}


void device_detach(capture_dev_t *_dev)
{
  v4l_device_t *dev = V4L_DEV(_dev);
  int i;

  /* Throttle read output pipe */
  if ( dev->read_out_tag > 0 ) {
    g_source_remove(dev->read_out_tag);
    dev->read_out_tag = 0;
  }

  /* Destroy read input pipe */
  if ( dev->read_in[0] != -1 ) {
    close(dev->read_in[0]);
    dev->read_in[0] = -1;
  }

  if ( dev->read_in[1] != -1 ) {
    close(dev->read_in[1]);
    dev->read_in[1] = -1;
  }

  /* Wait for read thread termination */
  if ( dev->read_thread != NULL ) {
    g_thread_join(dev->read_thread);
    dev->read_thread = NULL;
  }

  /* Destroy read output pipe */
  if ( dev->read_out_channel != NULL ) {
    g_io_channel_unref(dev->read_out_channel);
    dev->read_out_channel = NULL;
  }

  if ( dev->read_out[0] != -1 ) {
    close(dev->read_out[0]);
    dev->read_out[0] = -1;
  }

  if ( dev->read_out[1] != -1 ) {
    close(dev->read_out[1]);
    dev->read_out[1] = -1;
  }

  /* Free input capture buffers */
  if ( dev->buf != NULL ) {
    if ( dev->streaming ) {
      if ( munmap(dev->buf, dev->mbuf.size) == -1 ) {
	eprintf("%s: munmap(%d): %s\n",
		dev->h.name, dev->mbuf.size, strerror(errno));
      }
    }
    else {
      free(dev->buf);
    }
  }

  dev->buf = NULL;

  /* Free output capture buffers */
  if ( dev->h.bufs != NULL ) {
    for (i = 0; i < dev->h.nbufs; i++) {
      free(dev->h.bufs[i].base);
      dev->h.bufs[i].base = NULL;
    }

    free(dev->h.bufs);
  }

  dev->h.nbufs = 0;
  dev->h.bufs = NULL;
  dev->h.buf_ready = -1;
}


int device_read(capture_dev_t *dev)
{
  /* Only streamed i/o */
  return 0;
}


/*===========================================================*/
/* Show device info                                          */
/*===========================================================*/

void device_show_status(capture_dev_t *_dev, char *hdr)
{
  v4l_device_t *dev = (v4l_device_t *) _dev;

  printf("%sV4L1 capture device:\n", hdr);
  printf("%s  Device name: %s\n", hdr, dev->h.name);
  printf("%s  Hardware info: %s\n", hdr, dev->hwinfo);
  printf("%s  Video input: %d - %s\n", hdr, dev->input, dev->input_str);
  printf("%s  Display size: width=%d height=%d\n", hdr,
	 dev->h.width, dev->h.height);
  printf("%s  Pixel format: %s\n", hdr, dev->pixelformat_str);
  printf("%s  Refresh rate: %lu fps\n", hdr, fps_compute(&dev->fps));
  printf("%s  Streaming: %s\n", hdr, dev->streaming ? "enabled":"disabled");
}


/*===========================================================*/
/* Device attributes management                              */
/*===========================================================*/

int device_attr_set(capture_dev_t *dev, char *key, char *value)
{
  return -1;
}


capture_attr_t *device_attr_get(capture_dev_t *dev, char *key, int *_nmemb)
{
  return NULL;
}
