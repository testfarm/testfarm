/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* Graphical Front-end                                                      */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 05-JAN-2004                                                    */
/****************************************************************************/

/*
 * $Revision: 1015 $
 * $Date: 2008-08-13 15:38:03 +0200 (mer., 13 août 2008) $
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
