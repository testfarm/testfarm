/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* Display control buffer allocation                                        */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 07-NOV-2007                                                    */
/****************************************************************************/

/*
 * $Revision: 1016 $
 * $Date: 2008-08-13 16:16:39 +0200 (mer., 13 août 2008) $
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
