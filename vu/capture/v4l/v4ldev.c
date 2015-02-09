/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* V4L2 Capture Device                                                      */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 04-MAY-2007                                                    */
/****************************************************************************/

/*
 * $Revision: 1233 $
 * $Date: 2012-04-08 18:01:14 +0200 (dim., 08 avril 2012) $
 */

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

#include <linux/videodev2.h>

#include "jpegdecode.h"
#include "yuvdecode.h"
#include "v4ldev.h"


#undef __DEBUG__
#define dprintf(args...) //fprintf(stderr, "[V4L_DEV] " args);

static const char *device_default = "/dev/video";


#define CLEAR(x) memset (&(x), 0, sizeof (x))

#define eprintf(args...) fprintf(stderr, "testfarm-vu (v4l): " args)


static int xioctl(int fd, int request, void *arg)
{
  int ret;

  do {
    ret = ioctl (fd, request, arg);
  } while ( (ret == -1) && (errno == EINTR));

  return ret;
}


static int device_open_dev(char *devname)
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

  fd = open(devname, O_RDWR | O_NONBLOCK, 0);
  if ( fd == -1 ) {
    eprintf("%s: Cannot open video device: %s\n",
	    devname, strerror (errno));
    return -1;
  }

  return fd;
}


void device_close(capture_dev_t *_dev)
{
  v4l_device_t *dev = (v4l_device_t *) _dev;

  /* Stop I/O events handling */
  if ( dev->tag > 0 ) {
    g_source_remove(dev->tag);
    dev->tag = 0;
  }

  if ( dev->channel != NULL ) {
    g_io_channel_unref(dev->channel);
    dev->channel = NULL;
  }

  if ( dev->decoder != NULL ) {
    if ( dev->decoder->destroy != NULL ) {
      dev->decoder->destroy(dev->decoder);
    }
    dev->decoder = NULL;
  }

  if ( dev->fd >= 0 ) {
    if ( close(dev->fd) == -1 )
      eprintf("%s: Failed to close video device: %s",
	      dev->h.name, strerror(errno));
    dev->fd = -1;
  }

  if ( dev->buf != NULL ) {
    free(dev->buf);
    dev->buf = NULL;
  }

  if ( dev->hwinfo != NULL ) {
    free(dev->hwinfo);
    dev->hwinfo = NULL;
  }

  capture_dev_clear(CAPTURE_DEV(dev));

  free(dev);
}


static int device_init_input(v4l_device_t *dev, int input_index)
{
  int ret = 0;

  /* Switch input */
  if ( input_index >= 0 ) {
    dprintf("%s: Switching input %d\n", dev->h.name, input_index);

    if ( xioctl(dev->fd, VIDIOC_S_INPUT, &input_index) == -1 ) {
      eprintf("%s: Cannot select video input %d: %s\n",
	      dev->h.name, input_index, strerror(errno));
      ret = -1;
    }
  }
  else {
    if ( xioctl(dev->fd, VIDIOC_G_INPUT, &input_index) == -1 ) {
      eprintf("%s: Cannot get video input number: %s\n",
	      dev->h.name, strerror(errno));
      ret = -1;
    }
  }

  if ( ret == 0 )
    dev->input = input_index;

  return ret;
}


static void device_show_input(v4l_device_t *dev, char *hdr)
{
  char *name = "*unknown*";
  struct v4l2_input input;

  input.index = dev->input;
  if ( xioctl(dev->fd, VIDIOC_ENUMINPUT, &input) == 0 ) {
    name = (char *) input.name;
  }

  printf("%s  Video input: %d - %s\n", hdr, dev->input, name);
}


static int device_init_format(v4l_device_t *dev)
{
  struct v4l2_cropcap cropcap;
  struct v4l2_fmtdesc fmtdesc;
  struct v4l2_format format;
  char *fmtstr = NULL;
  int fmtstrlen = 0;

  /* Get frame size */
  cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  if ( xioctl(dev->fd, VIDIOC_CROPCAP, &cropcap) == 0 ) {
    dev->h.width = cropcap.defrect.width;
    dev->h.height = cropcap.defrect.height;
  }
  else {
    format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if ( xioctl(dev->fd, VIDIOC_G_FMT, &format) == -1 ) {
      eprintf("%s: Cannot get video device format: %s\n",
	      dev->h.name, strerror(errno));
      return -1;
    }

    dprintf("%s: VIDIOC_G_FMT -> width=%d height=%d pixelformat=%c%c%c%c field=%d bytesperline=%d sizeimage=%d\n",
	    dev->h.name, format.fmt.pix.width, format.fmt.pix.height,
	    (format.fmt.pix.pixelformat >> 0) & 0xFF,
	    (format.fmt.pix.pixelformat >> 8) & 0xFF,
	    (format.fmt.pix.pixelformat >> 16) & 0xFF,
	    (format.fmt.pix.pixelformat >> 24) & 0xFF,
	    format.fmt.pix.field,
	    format.fmt.pix.bytesperline,
	    format.fmt.pix.sizeimage);

    dev->h.width = format.fmt.pix.width;
    dev->h.height = format.fmt.pix.height;
  }

  /* Choose an acceptable pixel format */
  format.fmt.pix.pixelformat = 0;
  fmtdesc.index = 0;
  fmtdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  while ( xioctl(dev->fd, VIDIOC_ENUM_FMT, &fmtdesc) == 0 ) {
    int size = 10 + sizeof(fmtdesc.description);

    fmtstr = realloc(fmtstr, fmtstrlen + size);

    if ( fmtdesc.index > 0 )
      fmtstrlen += sprintf(fmtstr+fmtstrlen, ", ");
    fmtstrlen += sprintf(fmtstr+fmtstrlen, "%c%c%c%c (%s)",
	    (fmtdesc.pixelformat >> 0) & 0xFF,
	    (fmtdesc.pixelformat >> 8) & 0xFF,
	    (fmtdesc.pixelformat >> 16) & 0xFF,
	    (fmtdesc.pixelformat >> 24) & 0xFF,
	    fmtdesc.description);

    if ( (fmtdesc.pixelformat == V4L2_PIX_FMT_RGB24) ||
	 (fmtdesc.pixelformat == V4L2_PIX_FMT_BGR24) ||
	 (fmtdesc.pixelformat == V4L2_PIX_FMT_RGB565) ||
	 (fmtdesc.pixelformat == V4L2_PIX_FMT_YVU420) ||
	 (fmtdesc.pixelformat == V4L2_PIX_FMT_YUV420) ||
	 (fmtdesc.pixelformat == V4L2_PIX_FMT_JPEG) ) {
      if ( format.fmt.pix.pixelformat == 0 )
	format.fmt.pix.pixelformat = fmtdesc.pixelformat;
    }

    fmtdesc.index++;
  }

  if ( format.fmt.pix.pixelformat == 0 ) {
    eprintf("%s: Video device does not support pixel formats RGB24/BGR24/RGB565/YUV420/JPEG\n",
	     dev->h.name);
    if ( fmtstr != NULL ) {
      eprintf("%s: Video device only supports formats %s\n",
	      dev->h.name, fmtstr);
    }
  }

  if ( fmtstr != NULL )
    free(fmtstr);

  if ( format.fmt.pix.pixelformat == 0 )
    return -1;

  format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  format.fmt.pix.width = dev->h.width;
  format.fmt.pix.height = dev->h.height;
  format.fmt.pix.field = V4L2_FIELD_NONE;      /* Try progressive field format first */

  if ( xioctl(dev->fd, VIDIOC_S_FMT, &format) == -1 ) {
    format.fmt.pix.field = V4L2_FIELD_ANY;     /* Then ask default field format */
  }

  if ( xioctl(dev->fd, VIDIOC_S_FMT, &format) == -1 ) {
    eprintf("%s: Cannot set video device format: %s\n",
	    dev->h.name, strerror(errno));
    return -1;
  }

  dev->pixelformat[0] = (format.fmt.pix.pixelformat >> 0)  & 0xFF;
  dev->pixelformat[1] = (format.fmt.pix.pixelformat >> 8)  & 0xFF;
  dev->pixelformat[2] = (format.fmt.pix.pixelformat >> 16) & 0xFF;
  dev->pixelformat[3] = (format.fmt.pix.pixelformat >> 24) & 0xFF;
  dev->pixelformat[4] = '\0';
  dev->imagesize = format.fmt.pix.sizeimage;

  switch ( format.fmt.pix.pixelformat ) {
  case V4L2_PIX_FMT_RGB24:
    dev->h.pixfmt = CAPTURE_PIXFMT_RGB24;
    break;
  case V4L2_PIX_FMT_BGR24:
    dev->h.pixfmt = CAPTURE_PIXFMT_BGR24;
    break;
  case V4L2_PIX_FMT_YVU420:
  case V4L2_PIX_FMT_YUV420:
    dev->h.pixfmt = CAPTURE_PIXFMT_RGB24;
    dev->decoder = yuv_create(format.fmt.pix.pixelformat, dev->h.width, dev->h.height);
    dev->streaming = 0;  /* Streamed YUV decoding not yet implemented */
    break;
  case V4L2_PIX_FMT_JPEG:
    dev->h.pixfmt = CAPTURE_PIXFMT_RGB24;
    dev->decoder = jd_create(format.fmt.pix.sizeimage);
    dev->streaming = 0;  /* Streamed JPEG decoding not yet implemented */
    break;
  case V4L2_PIX_FMT_RGB565:
    dev->h.pixfmt = CAPTURE_PIXFMT_RGB565;
    dev->h.pixsize = 2;
    break;
  default:
    /* Nothing */
    break;
  }

  /* Set field order (progressive or interlaced) */
  if ( (format.fmt.pix.field != V4L2_FIELD_NONE) &&
       (format.fmt.pix.field != V4L2_FIELD_INTERLACED) ) {
    eprintf("%s: Progressive or interlaced field formats not supported\n",
	    dev->h.name);
    return -1;
  }

  return 0;
}


static int device_init_caps(v4l_device_t *dev)
{
  struct v4l2_capability cap;
  int len;

  /* Get video device capabilities */
  if ( xioctl(dev->fd, VIDIOC_QUERYCAP, &cap) == -1 ) {
    if ( errno == EINVAL ) {
      dprintf("%s: Not a V4L2-compliant video device\n",
	      dev->h.name);
    }
    else {
      eprintf("%s: Cannot get video device capabilities: %s\n",
	      dev->h.name, strerror(errno));
    }

    return -1;
  }

  if ( !(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) ) {
    eprintf("%s: Not a video capture device\n",
	    dev->h.name);
    return -1;
  }

  if ( cap.capabilities & V4L2_CAP_STREAMING ) {
    dev->streaming = 1;
  }
  else if ( !(cap.capabilities & V4L2_CAP_READWRITE) ) {
    eprintf("%s: No support for read/write or streaming I/O\n",
	    dev->h.name);
    return -1;
  }

  len = strlen((char *) cap.card) + strlen((char *) cap.bus_info) + 4;
  dev->hwinfo = (char *) malloc(len+1);
  snprintf(dev->hwinfo, len, "%s (%s)", cap.card, cap.bus_info);

  return 0;
}


static int device_qbuf_streaming(v4l_device_t *dev, int index)
{
  struct v4l2_buffer buf;

  CLEAR (buf);
  buf.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  buf.memory = V4L2_MEMORY_MMAP;
  buf.index  = index;

  if ( xioctl(dev->fd, VIDIOC_QBUF, &buf) == -1 ) {
    eprintf("%s: ioctl(VIDIOC_QBUF,buf%d): %s\n",
	    dev->h.name, index, strerror(errno));
    return -1;
  }

  return 0;
}


static int device_qbuf_readwrite(v4l_device_t *dev, int index)
{
  if ( index >= dev->h.nbufs ) {
    eprintf("%s: Illegal buffer index %d\n", dev->h.name, index);
    return -1;
  }

  dev->h.bufs[index].bytesused = 0;

  return 0;
}


int device_qbuf(capture_dev_t *dev, int index)
{
  return V4L_DEV(dev)->streaming ?
    device_qbuf_streaming((v4l_device_t *) dev, index) :
    device_qbuf_readwrite((v4l_device_t *) dev, index);
}


static int device_streamon(v4l_device_t *dev)
{
  enum v4l2_buf_type type;

  type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  if ( xioctl (dev->fd, VIDIOC_STREAMON, &type) == -1 ) {
    eprintf("%s: ioctl(VIDIOC_STREAMON): %s\n",
	    dev->h.name, strerror(errno));
    return -1;
  }

  return 0;
}


static int device_streamoff(v4l_device_t *dev)
{
  enum v4l2_buf_type type;

  if ( dev->fd < 0 )
    return 0;

  type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  if ( xioctl (dev->fd, VIDIOC_STREAMOFF, &type) == -1 ) {
    eprintf("%s: ioctl(VIDIOC_STREAMOFF): %s\n",
	    dev->h.name, strerror(errno));
    return -1;
  }

  return 0;
}


static int device_attach_streaming(v4l_device_t *dev)
{
  struct v4l2_requestbuffers req;
  int i;

  CLEAR(req);
  req.count = 4;
  req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  req.memory = V4L2_MEMORY_MMAP;
  if ( xioctl(dev->fd, VIDIOC_REQBUFS, &req) == -1 ) {
    eprintf("%s: ioctl(VIDIOC_REQBUFS): %s\n",
	    dev->h.name, strerror(errno));
    return -1;
  }

  if ( req.count < 2 ) {
    eprintf("%s: Insufficient buffer memory\n",
	    dev->h.name);
    return -1;
  }

  dev->h.nbufs = req.count;
  dev->h.bufs = calloc(dev->h.nbufs, sizeof(*(dev->h.bufs)));
  dev->h.buf_ready = -1;

  /* Map capture buffers */
  for (i = 0; i < dev->h.nbufs; i++) {
    struct v4l2_buffer buf;

    CLEAR (buf);
    buf.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    buf.index  = i;

    if ( xioctl(dev->fd, VIDIOC_QUERYBUF, &buf) == -1 ) {
      eprintf("%s: ioctl(VIDIOC_QUERYBUF,buf%d): %s\n",
	      dev->h.name, i, strerror(errno));
      return -1;
    }

    dprintf("%s: Streaming buf%d: offset=%d length=%d\n", dev->h.name, i, buf.m.offset, buf.length);
    dev->h.bufs[i].length = buf.length;
    dev->h.bufs[i].base = mmap(NULL /* start anywhere */,
			       buf.length,
			       PROT_READ | PROT_WRITE,
			       MAP_SHARED,
			       dev->fd, buf.m.offset);
    if ( dev->h.bufs[i].base == MAP_FAILED ) {
      eprintf("%s: mmap(buf%d,%d): %s\n",
	      dev->h.name, i, buf.length, strerror(errno));
      return -1;
    }
  }

  /* Enqueue capture buffers */
  for (i = 0; i < dev->h.nbufs; i++) {
    if ( device_qbuf_streaming(dev, i) == -1 )
      return -1;
  }

  /* Start streaming */
  return device_streamon(dev);
}


static int device_attach_readwrite(v4l_device_t *dev)
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


int device_attach(capture_dev_t *dev)
{
  return V4L_DEV(dev)->streaming ?
    device_attach_streaming((v4l_device_t *) dev) :
    device_attach_readwrite((v4l_device_t *) dev);
}


void device_detach(capture_dev_t *_dev)
{
  v4l_device_t *dev = V4L_DEV(_dev);
  int i;

  if ( dev->streaming )
    device_streamoff(dev);

  if ( dev->h.bufs != NULL ) {
    for (i = 0; i < dev->h.nbufs; i++) {
      if ( dev->streaming ) {
	if ( munmap(dev->h.bufs[i].base, dev->h.bufs[i].length) == -1 ) {
	  eprintf("%s: munmap(buf%d): %s\n",
		  dev->h.name, i, strerror(errno));
	}
      }
      else {
	free(dev->h.bufs[i].base);
      }

      dev->h.bufs[i].base = NULL;
    }

    free(dev->h.bufs);
  }

  dev->h.nbufs = 0;
  dev->h.bufs = NULL;
  dev->h.buf_ready = -1;
}


static int device_checkup(v4l_device_t *dev)
{
  int i;

  dprintf("%s: CHECKUP ready=%d\n", dev->h.name, dev->h.buf_ready);

  for (i = 0; i < dev->h.nbufs; i++) {
    struct v4l2_buffer buf;

    CLEAR (buf);
    buf.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.index  = i;

    if ( xioctl(dev->fd, VIDIOC_QUERYBUF, &buf) == -1 ) {
      eprintf("%s: ioctl(VIDIOC_QUERYBUF,buf%d): %s\n",
	      dev->h.name, i, strerror(errno));
      return -1;
    }

#ifdef __DEBUG__
    printf("   buf%d: busy=%d", i, dev->bufs[i].busy);
    if ( buf.flags & V4L2_BUF_FLAG_MAPPED )
      printf(" MAPPED");
    if ( buf.flags & V4L2_BUF_FLAG_QUEUED )
      printf(" QUEUED");
    if ( buf.flags & V4L2_BUF_FLAG_DONE )
      printf(" DONE");
#endif

    if ( ( i != dev->h.buf_ready) &&
	 (! dev->h.bufs[i].busy) &&
	 (! (buf.flags & (V4L2_BUF_FLAG_QUEUED | V4L2_BUF_FLAG_DONE))) ) {
#ifdef __DEBUG__
      printf(" => QBUF");
#endif
      device_qbuf(CAPTURE_DEV(dev), i);
    }

#ifdef __DEBUG__
    printf("\n");
#endif
  }

  return 0;
}


static int device_read_streaming(v4l_device_t *dev)
{
  struct v4l2_buffer buf;
  unsigned long long tstamp;

  CLEAR (buf);
  buf.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  buf.memory = V4L2_MEMORY_MMAP;

  if ( xioctl(dev->fd, VIDIOC_DQBUF, &buf) == -1 ) {
    switch (errno) {
    case EAGAIN:
      return 0;

    case EIO:
      /* If an I/O error occured, ignore it and restart available streaming buffers */
      device_checkup(dev);
      return -1;

    default:
      eprintf("%s: ioctl(VIDIOC_DQBUF): %s\n", dev->h.name, strerror(errno));
      return -1;
    }
  }

  if ( buf.index >= dev->h.nbufs ) {
    eprintf("%s: Got illegal buffer index (%d)\n", dev->h.name, buf.index);
    return -2;
  }

  /* Requeue previously 'ready' buffer if it was not processed */
  if ( dev->h.buf_ready >= 0 )
    device_qbuf_streaming(dev, dev->h.buf_ready);

  /* Consider newly filled buffer as 'ready' */
  dev->h.buf_ready = buf.index;
  dev->h.bufs[buf.index].bytesused = buf.bytesused;

  /* Update refresh rate statistics */
  tstamp = (((unsigned long long) buf.timestamp.tv_sec) * 1000000ULL) + buf.timestamp.tv_usec;
  fps_update(&dev->fps, tstamp);

  return 0;
}


static int device_read_readwrite(v4l_device_t *dev)
{
  int ret = -1;
  int index;

  /* Choose a free buffer to fill */
  index = 0;
  while ( (index < dev->h.nbufs) &&
	  (dev->h.bufs[index].bytesused != 0) ) {
    index++;
  }

  /* Load buffer with incoming frame */
  if ( index < dev->h.nbufs ) {
    unsigned char *base;
    unsigned long length;

    if ( dev->decoder != NULL ) {
      if ( dev->buf == NULL )
	dev->buf = calloc(dev->imagesize, 1);
      base = dev->buf;
      length = dev->imagesize;
    }
    else {
      base = dev->h.bufs[index].base;
      length = dev->h.bufs[index].length;
    }

    ret = read(dev->fd, base, length);
    if ( ret < 0 ) {
      if ( (errno != EAGAIN) && (errno != EINTR) ) {
	eprintf("%s: read(%lu): %s\n", dev->h.name, length, strerror(errno));
      }
      else {
	ret = 0;
      }
    }

    if ( ret > 0 ) {
      struct timeval tv;
      unsigned long long tstamp;

      if ( (dev->decoder != NULL) && (dev->decoder->process != NULL) ) {
	ret = dev->decoder->process(dev->decoder,
				    base, ret,
				    dev->h.bufs[index].base, dev->h.bufs[index].length);
      }

      /* Requeue previously 'ready' buffer if it was not processed */
      if ( dev->h.buf_ready >= 0 )
	device_qbuf_readwrite(dev, dev->h.buf_ready);

      /* Consider newly filled buffer as 'ready' */
      dev->h.bufs[index].bytesused = ret;
      dev->h.buf_ready = index;

      /* Update refresh rate statistics */
      gettimeofday(&tv, NULL);
      tstamp = (((unsigned long long) tv.tv_sec) * 1000000ULL) + tv.tv_usec;
      fps_update(&dev->fps, tstamp);

      ret = 0;
    }
  }
  else {
    eprintf("%s: No more i/o buffers available\n", dev->h.name);
  }

  return ret;
}


static int device_read_async(v4l_device_t *dev)
{
  return dev->streaming ?
    device_read_streaming(dev) :
    device_read_readwrite(dev);
}


int device_read(capture_dev_t *dev)
{
  /* Only streamed i/o */
  return 0;
}


static gboolean device_read_event(GIOChannel *source, GIOCondition condition, v4l_device_t *dev)
{
  if ( condition & G_IO_HUP ) {
    eprintf("%s: Device abnormally closed\n", dev->h.name);
    exit(EXIT_FAILURE);
    return FALSE;
  }

  device_read_async(dev);

  if ( dev->callback_func != NULL ) {
    dev->callback_func(dev->callback_ctx);
  }

  return TRUE;
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

  fd = device_open_dev(devname);
  if ( fd >= 0 ) {
    dev = (v4l_device_t *) malloc(sizeof(v4l_device_t));
    memset(dev, 0, sizeof(v4l_device_t));

    capture_dev_set(CAPTURE_DEV(dev), devname);
  }

  free(devname);

  if ( fd < 0 )
    return NULL;

  dev->fd = fd;

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

  /* Setup I/O events handling (if needed) */
  dev->callback_func = callback_func;
  dev->callback_ctx = callback_ctx;
  dev->channel = g_io_channel_unix_new(dev->fd);
  dev->tag = g_io_add_watch(dev->channel, G_IO_IN | G_IO_HUP,
			    (GIOFunc) device_read_event, dev);

  return CAPTURE_DEV(dev);
}


/*===========================================================*/
/* Show device info                                          */
/*===========================================================*/

void device_show_status(capture_dev_t *_dev, char *hdr)
{
  v4l_device_t *dev = (v4l_device_t *) _dev;

  printf("%sV4L2 capture device:\n", hdr);
  printf("%s  Device name: %s\n", hdr, dev->h.name);
  printf("%s  Hardware info: %s\n", hdr, dev->hwinfo);
  device_show_input(dev, hdr);
  printf("%s  Display size: width=%d height=%d\n", hdr,
	 dev->h.width, dev->h.height);
  printf("%s  Pixel format: %s\n", hdr, dev->pixelformat);
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
