/**********************************************************************/
/* TestFarm Virtual User                                              */
/* TVU Grab Tool                                                      */
/**********************************************************************/
/* Author: Sylvain Giroudon                                           */
/* Creation: 14-JUN-2006                                              */
/**********************************************************************/

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
    fprintf(stderr, "TestFarm Virtual User Grab Tool - (c) TestFarm.org 2006-2015\n");
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
