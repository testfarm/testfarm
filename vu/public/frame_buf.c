/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* Display control buffer allocation                                        */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 07-NOV-2007                                                    */
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
#include <errno.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include "frame_buf.h"

#define eprintf(args...) fprintf(stderr, "testfarm-vu (frame_buf): " args)


frame_buf_t *frame_buf_alloc(unsigned int width, unsigned int height, int *_shmid)
{
  unsigned int rgbsize;
  unsigned int shmsize;
  int shmid;
  frame_buf_t *fb;

  /* Alloc frame control buffer */
  rgbsize =  frame_rgb_bufsize(width, height);
  shmsize = sizeof(frame_buf_t) + rgbsize + FRAME_RGB_MMTAIL;
  if ( (shmid = shmget(IPC_PRIVATE, shmsize, IPC_CREAT | IPC_EXCL | SHM_R | SHM_W)) == -1 ) {
    eprintf("shmget: %s\n", strerror(errno));
    return NULL;
  }
  if ( (fb = shmat(shmid, NULL, 0)) == (void *) -1 ) {
    eprintf("shmat(%d): %s\n", shmid, strerror(errno));
    return NULL;
  }

  /* Mark shared memory buffer for automatic deletion
     when all processes have detached */
  if ( shmctl(shmid, IPC_RMID, NULL) == -1 ) {
    eprintf("shmctl(%d): %s\n", shmid, strerror(errno));
    return NULL;
  }

  /* Setup frame control buffer */
  if ( fb == NULL ) {
    eprintf("Frame Control buffer allocation failed (null pointer returned)\n");
    return NULL;
  }

  frame_rgb_init(&fb->rgb, width, height);

  if ( _shmid != NULL ) {
    *_shmid = shmid;
  }

  return fb;
}


frame_buf_t *frame_buf_map(int shmid, int readonly)
{
  int flg = readonly ? SHM_RDONLY : 0;
  frame_buf_t *fb;

  if ( (fb = shmat(shmid, NULL, flg)) == (void *) -1 ) {
    eprintf("shmat(%d): %s\n", shmid, strerror(errno));
    return NULL;
  }

  return fb;
}


void frame_buf_free(frame_buf_t *fb)
{
  if ( fb != NULL ) {
    if ( shmdt(fb) == -1 )
      eprintf("shmdt: %s\n", strerror(errno));
  }
}
