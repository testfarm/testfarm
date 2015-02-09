/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* Capture Device - Equipment Interface                                     */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 04-DEC-2007                                                    */
/****************************************************************************/

/*
 * $Revision: 897 $
 * $Date: 2008-01-14 11:01:41 +0100 (lun., 14 janv. 2008) $
 */

#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <glib.h>

#include "capture_device.h"

void capture_dev_set(capture_dev_t *dev, char *name)
{
  dev->name = strdup(name);
  dev->basename = g_path_get_basename(name);
  dev->pixfmt = CAPTURE_PIXFMT_RGB24;
  dev->pixsize = 3;   /* RGB24 mode by default */
  dev->nbufs = 0;
  dev->bufs = NULL;
  dev->buf_ready = -1;
}


void capture_dev_clear(capture_dev_t *dev)
{
  if ( dev->basename != NULL ) {
    free(dev->basename);
    dev->basename = NULL;
  }

  if ( dev->name != NULL ) {
    free(dev->name);
    dev->name = NULL;
  }

  dev->nbufs = 0;
  dev->bufs = NULL;
  dev->buf_ready = -1;  
}
