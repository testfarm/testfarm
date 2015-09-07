/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* Graphical Front-end                                                      */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 05-JAN-2004                                                    */
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

#ifndef __TVU_DISPLAY_H__
#define __TVU_DISPLAY_H__

#include <gtk/gtk.h>

#include "capture_cap.h"
#include "frame_geometry.h"
#include "frame_ctl.h"
#include "frame_hdr.h"
#include "display_pad.h"


#define DISPLAY_BPP 3


typedef struct {
  frame_geometry_t g0;
  unsigned char *buf;
} display_layer_t;

typedef struct {
  frame_hdr_t root;           /* Root frame descriptor */
  char *name;                 /* Display name */
  int available;              /* Client availability flag */
  int nlayers;
  frame_hdr_t **layers;
  unsigned char *layout;
} display_desktop_t;


typedef struct {
  display_desktop_t desktop;

  char *sockname;             /* Control socket name */
  int sock;                   /* Control socket fd */
  int sock_tag;               /* Control socket GDK input tag */
  unsigned char *rsp_buf;
  int rsp_size;
  int rsp_idx;

  capture_cap_t cap;          /* Frame device capabilities */
  unsigned char *rgb;         /* RGB display buffer */
  unsigned int rgb_width;     /* RGB display width */
  unsigned int rgb_height;    /* RGB display height */
  frame_geometry_t rgb_window; /* RGB active frame geometry */

  display_pad_t pad;

  int sel_x, sel_y;
  frame_geometry_t sel_window;
} display_t;

extern display_t *display_current;

extern int display_init(GtkWindow *window);
extern void display_done(void);

extern display_t *display_alloc(char *name);
extern void display_free(display_t *d);

extern void display_error(char *fmt, ...);

extern int display_connect(display_t *d);
extern void display_disconnect(display_t *d);
extern void display_setup(display_t *d);

extern void display_refresh(display_t *d, frame_geometry_t *g);
extern void display_selection_show(display_t *d);

#endif  /* __TVU_DISPLAY_H__ */
