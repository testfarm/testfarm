/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* File Input pseudo-device                                                 */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 18-JUL-2007                                                    */
/****************************************************************************/

/*
 * $Revision: 1099 $
 * $Date: 2010-01-03 23:08:32 +0100 (dim., 03 janv. 2010) $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <errno.h>
#include <sys/stat.h>

#include "filelist.h"

#include "ppm.h"
#include "png_file.h"
#include "jpeg.h"
#include "filedev.h"


static const char *device_default = "frame.ppm.gz";

#define eprintf(args...) fprintf(stderr, "testfarm-vu (file): " args)
#define dprintf(args...) //fprintf(stderr, "[FILE_DEV] " args);

enum {
  FMT_PPM=0,
  FMT_PNG,
  FMT_JPEG,
  FMT_N
};

static char *fmt_str[FMT_N] = {"PPM", "PNG", "JPEG"};


/* Capture device attributes:
   ID       DESCRIPTION                          VALUES
   -------  -----------------------------------  ------------------------------------------------
   Loop     Restart file sequence when finished  =0: disabled
                                                 >0: loop delay in milliseconds
   Sync     Synchronous sequence mode            =0: real time (synchronized by file time stamps)
                                                 =1: synchronized by 'capture refresh' settings
   Current  Current frame file                   File name (read only)
   Restart  Restart sequence                     (command)
*/
static unsigned long attr_loop = 0;
static int attr_sync = 0;


void device_close(capture_dev_t *_dev)
{
  file_device_t *dev = FILE_DEV(_dev);

  if ( dev == NULL )
    return;

  if ( dev->timeout_tag > 0 )
    g_source_remove(dev->timeout_tag);
  dev->timeout_tag = 0;

  if ( dev->files != NULL )
    free(dev->files);
  dev->files = NULL;
  dev->nfiles = 0;

  capture_dev_clear(_dev);

  free(dev);
}


static void sequence_init(file_device_t *dev, GList *flist)
{
  GList *l;
  unsigned long tstamp;
  int i;

  dev->nfiles = g_list_length(flist);
  dev->files = calloc(dev->nfiles, sizeof(file_device_frame_t));

  tstamp = 0;
  i = 0;
  l = flist;
  while ( (l != NULL) && (i < dev->nfiles) ) {
    char *s1, *s2;

    s1 = strchr(l->data, '.');
    if ( s1 != NULL ) {
      s1++;

      s2 = strchr(s1, '.');
      if ( s2 != NULL ) {
	tstamp = strtoul(s1, NULL, 10);
      }
    }

    dprintf("INIT '%s' => %lu\n", (char *) l->data, tstamp);
    dev->files[i].fname = l->data;
    dev->files[i].tstamp = tstamp;
    i++;

    l = g_list_next(l);
  }
}


static gboolean sequence_next(file_device_t *dev)
{
  int max = dev->nfiles - 1;

  if ( dev->current < max ) {
    (dev->current)++;
    dprintf("NEXT => %d\n", dev->current);
  }
  else if ( attr_loop ) {
    dev->current = 0;
  }

  if ( ! attr_sync ) {
    long dt = 0;

    if ( dev->current < max ) {
      dt = dev->files[dev->current+1].tstamp - dev->files[dev->current].tstamp;
      if ( dt < 10 )
	dt = 10;
    }
    else if ( attr_loop ) {
      dt = attr_loop;
    }

    if ( dt > 0 ) {
      dev->timeout_tag = g_timeout_add(dt, (GSourceFunc) sequence_next, dev);
      dprintf("       dt=%ld\n", dt);
    }
  }

  return FALSE;
}


static void sequence_restart(file_device_t *dev)
{
  if ( attr_sync ) {
    dev->current = 0;
  }
  else {
    dev->current = -1;
    sequence_next(dev);
  }
}


capture_dev_t *device_open(char *devname, capture_func callback_func, void *callback_ctx)
{
  GList *flist = NULL;
  file_device_t *dev;
  char *fname;

  /* Collect image files */
  if ( (devname == NULL) || (*devname == '\0') )
    devname = (char *) device_default;

  if ( g_file_test(devname, G_FILE_TEST_EXISTS) ) {
    flist = g_list_append(flist, devname);
  }
  else {
    int len = strlen(devname);
    char regex[len+32];

    snprintf(regex, sizeof(regex), "^%s\\.\\d+\\.", devname);
    flist = filelist(regex);

    if ( flist == NULL ) {
      eprintf("No file found matching '%s.*'\n", devname);
      return NULL;
    }

    /* Sort file list in alphanumeric order */
    flist = g_list_sort(flist, (GCompareFunc) strcmp);
  }

  /* Alloc device descriptor */
  dev = (file_device_t *) malloc(sizeof(file_device_t));
  memset(dev, 0, sizeof(file_device_t));

  /* Set file name */
  capture_dev_set(CAPTURE_DEV(dev), devname);

  sequence_init(dev, flist);

  g_list_free(flist);

  /* Alloc incoming frame buffers */
  dev->h.nbufs = 2;
  dev->h.bufs = calloc(dev->h.nbufs, sizeof(*(dev->h.bufs)));

  /* Guess image size and format  */
  fname = dev->files[0].fname;
  if ( jpeg_filename_suffix(fname) ) {
    if ( jpeg_size(fname, &dev->h.width, &dev->h.height) ) {
      device_close(CAPTURE_DEV(dev));
      return NULL;
    }

    dev->fmt = FMT_JPEG;
  }
  else if ( png_filename_suffix(fname) ) {
    if ( png_size(fname, &dev->h.width, &dev->h.height) ) {
      device_close(CAPTURE_DEV(dev));
      return NULL;
    }

    dev->fmt = FMT_PNG;
  }
  else {
    if ( ppm_size(fname, &dev->h.width, &dev->h.height) ) {
      device_close(CAPTURE_DEV(dev));
      return NULL;
    }

    dev->fmt = FMT_PPM;
  }

  /* Clear file sequence index */
  dev->current = -1;

  return CAPTURE_DEV(dev);
}


int device_qbuf(capture_dev_t *dev, int index)
{
  if ( dev->bufs[index].base != NULL ) {
    free(dev->bufs[index].base);
    dev->bufs[index].base = NULL;
  }

  return 0;
}


int device_attach(capture_dev_t *dev)
{
  return 0;
}


void device_detach(capture_dev_t *dev)
{
  int i;

  for (i = 0; i < dev->nbufs; i++) {
    device_qbuf(dev, i);
  }
}


static int device_changed(file_device_t *dev)
{
  char *fname;
  struct stat file_stat;
  int changed = 0;

  if ( dev->current < 0 )
    return 0;

  fname = dev->files[dev->current].fname;
  if ( stat(fname, &file_stat) == 0 ) {
    changed = (dev->ino != file_stat.st_ino) || (dev->mtime != file_stat.st_mtime);

    dev->ino = file_stat.st_ino;
    dev->mtime = file_stat.st_mtime;
  }

  return changed;
}


static unsigned char *device_remap(file_device_t *dev,
				   unsigned int width, unsigned int height, unsigned char *buf)
{
  int new_rowsize = FRAME_RGB_BPP * dev->h.width;
  int new_height = dev->h.height;
  int new_bufsize = new_rowsize * new_height;
  int rowsize = FRAME_RGB_BPP * width;
  int min_rowsize = (rowsize < new_rowsize) ? rowsize : new_rowsize;
  int min_height = (height < new_height) ? height : new_height;
  int rem_rowsize = new_rowsize - min_rowsize;
  unsigned char *new_buf, *new_p;
  unsigned char *p;
  int y;

  new_buf = new_p = (unsigned char *) malloc(new_bufsize);
  p = buf;

  for (y = 0; y < min_height; y++) {
    memcpy(new_p, p, min_rowsize);
    if ( rem_rowsize > 0 )
      memset(new_p+min_rowsize, 0, rem_rowsize);

    p += rowsize;
    new_p += new_rowsize;
  }

  if ( y < new_height ) {
    int rem_size = new_rowsize * (new_height - y);
    memset(new_p, 0, rem_size);
  }

  free(buf);

  return new_buf;
}


int device_read(capture_dev_t *_dev)
{
  file_device_t *dev = FILE_DEV(_dev);
  int index;
  int ret = -1;

  if ( (dev->current < 0) || attr_sync )
    sequence_next(dev);

  /* Nothing to do if file has not changed */
  if ( ! device_changed(dev) )
    return ret;

  /* Requeue previously 'ready' buffer if it was not processed */
  if ( dev->h.buf_ready >= 0 ) {
    device_qbuf(CAPTURE_DEV(dev), dev->h.buf_ready);
    dev->h.buf_ready = -1;
  }

  /* Choose a free buffer to fill */
  index = 0;
  while ( (index < dev->h.nbufs) &&
	  (dev->h.bufs[index].base != NULL) )
    index++;

  if ( index < dev->h.nbufs ) {
    char *fname = dev->files[dev->current].fname;
    unsigned char *buf = NULL;
    unsigned int width, height;

    dprintf("READ => %d\n", dev->current);

    switch ( dev->fmt ) {
    case FMT_PPM:  ppm_load(fname, &width, &height, &buf); break;
    case FMT_PNG:  png_load(fname, &width, &height, &buf); break;
    case FMT_JPEG: jpeg_load(fname, &width, &height, &buf); break;
    }

    if ( buf != NULL ) {
      /* Remap buffer so that it fits into the original frame size */
      if ( (width != dev->h.width) || (height != dev->h.height) ) {
	buf = device_remap(dev, width, height, buf);
      }

      dev->h.bufs[index].base = buf;
      dev->h.buf_ready = index;
      ret = 0;
    }
  }

  return ret;
}


/*===========================================================*/
/* Show device info                                          */
/*===========================================================*/

void device_show_status(capture_dev_t *_dev, char *hdr)
{
  file_device_t *dev = FILE_DEV(_dev);

  printf("%sFILE capture device:\n", hdr);
  printf("%s  Base name: %s", hdr, dev->h.name);
  if ( dev->nfiles > 1 ) {
    printf(" (%d files)", dev->nfiles);
  }
  printf("\n");
  printf("%s  Image format: %s\n", hdr, fmt_str[dev->fmt]);
  printf("%s  Image size: width=%d height=%d\n", hdr,
	 dev->h.width, dev->h.height);
}


/*===========================================================*/
/* Device attributes management                              */
/*===========================================================*/

int device_attr_set(capture_dev_t *_dev, char *key, char *value)
{
  file_device_t *dev = FILE_DEV(_dev);

  if ( strcmp(key, "Loop") == 0 ) {
    attr_loop = strtoul(value, NULL, 0);
  }
  else if ( strcmp(key, "Sync") == 0 ) {
    attr_sync = atoi(value) ? 1:0;
  }
  else if ( strcmp(key, "Restart") == 0 ) {
    sequence_restart(dev);
  }
  else {
    eprintf("Unknown attribute '%s'\n", key);
    return -1;
  }

  return 0;
}


#define ATTR_N 4

capture_attr_t *device_attr_get(capture_dev_t *_dev, char *key, int *_nmemb)
{
  file_device_t *dev = FILE_DEV(_dev);
  static char loop_value[32];
  static char sync_value[32];
  static capture_attr_t tab[ATTR_N+1] = {
    { key: "Loop",    value: loop_value },
    { key: "Sync",    value: sync_value },
    { key: "Current", value: NULL },
    { key: "Restart", value: NULL },
    { key: NULL,      value: NULL }
  };
  capture_attr_t *attr = NULL;
  int nmemb = 0;

  snprintf(loop_value, sizeof(loop_value), "%lu", attr_loop);
  snprintf(sync_value, sizeof(sync_value), "%d", attr_sync);
  tab[2].value = (dev->current >= 0) ? dev->files[dev->current].fname : "(none)";

  if ( key != NULL ) {
    for (attr = tab; attr->key != NULL; attr++) {
      if ( strcmp(key, attr->key) == 0 )
	break;
    }

    if ( attr->key != NULL ) {
      if ( strcmp(key, "Restart") == 0 ) {
	sequence_restart(dev);
      }
      nmemb = 1;
    }
    else {
      attr = NULL;
      eprintf("Unknown attribute '%s'\n", key);
    }
  }
  else {
    attr = tab;
    nmemb = ATTR_N;
  }

  if ( _nmemb != NULL )
    *_nmemb = nmemb;
  return attr;
}
