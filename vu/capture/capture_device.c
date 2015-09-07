/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* Capture Device - Equipment Interface                                     */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 04-DEC-2007                                                    */
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
