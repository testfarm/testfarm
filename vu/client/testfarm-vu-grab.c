/**********************************************************************/
/* TestFarm Virtual User                                              */
/* TVU Grab Tool                                                      */
/**********************************************************************/
/* Author: Sylvain Giroudon                                           */
/* Creation: 14-JUN-2006                                              */
/**********************************************************************/

/*
 * $Revision: 980 $
 * $Date: 2008-03-04 17:54:24 +0100 (mar., 04 mars 2008) $
 */

#include <stdio.h>
#include <stdlib.h>

#include "frame_buf.h"
#include "frame_geometry.h"
#include "png_file.h"

#define NAME "testfarm-vu-grab"

int main(int argc, char *argv[])
{
  int shmid;
  frame_buf_t *fb;
  char *fname;

  if ( argc < 2 ) {
    fprintf(stderr, "TestFarm Virtual User Grab Tool - (c) Basil Dev 2006-2007\n");
    fprintf(stderr, "Usage: " NAME " <shmid> [<fname>]\n");
    exit(2);
  }

  shmid = atoi(argv[1]);

  if ( (fb = frame_buf_map(shmid, 1)) == NULL ) {
    fprintf(stderr, NAME ": Cannot map RGB buffer with shmid=%d\n", shmid);
    exit(1);
  }

  fname = png_filename(argv[2]);
  printf("%s\n", fname);

  png_save(&(fb->rgb), NULL, fname);

  frame_buf_free(fb);

  return 0;
}
