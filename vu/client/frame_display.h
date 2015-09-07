/****************************************************************************/
/* TestFarm VNC Interface                                                   */
/* Incoming Display Frame Management                                        */
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

#ifndef __FRAME_DISPLAY_H__
#define __FRAME_DISPLAY_H__

#include <glib.h>

#include "shell.h"

#include "pattern.h"
#include "frame_ctl_msg.h"
#include "frame.h"


typedef void frame_display_pattern_operation(void *data, frame_ctl_cmd *cmd);


typedef struct {
  frame_t *root;                       /* Root frame descriptor */

  shell_t *shell;                      /* Shell commmand interpreter */

  frame_display_pattern_operation *pattern_set; /* Pattern mngt handlers */
  frame_display_pattern_operation *pattern_get;
  void *pattern_data;

  char *sockname;                      /* Display socket name */

  int csock;                           /* Display connection socket */
  GIOChannel *csock_channel;
  guint csock_tag;

  int dsock;                           /* Display data socket */
  GIOChannel *dsock_channel;
  guint dsock_tag;

  unsigned char *dbuf;                 /* Data buffer */
  int dsize;                           /* Allocated data buffer size */
} frame_display_t;


extern frame_display_t *frame_display_alloc(char *name, frame_t *root);
extern void frame_display_destroy(frame_display_t *display);
extern void frame_display_disable(frame_display_t *display);
extern int frame_display_connected(frame_display_t *display);

extern void frame_display_set_shell(frame_display_t *display, shell_t *shell);
extern void frame_display_set_pattern_operations(frame_display_t *display,
						 frame_display_pattern_operation *declare,
						 frame_display_pattern_operation *reap,
						 void *pattern_data);

extern int frame_display_geometry(frame_display_t *display, frame_geometry_t *g);
extern void frame_display_refresh(frame_display_t *display, frame_t *frame, frame_geometry_t *g);
extern void frame_display_period(frame_display_t *display, unsigned long delay);
extern void frame_display_source_done(frame_display_t *display);

extern void frame_display_pattern_add(frame_display_t *display, pattern_t *pattern);
extern void frame_display_pattern_remove(frame_display_t *display, char *id);
extern void frame_display_pattern_show(frame_display_t *display, pattern_t *pattern);
extern void frame_display_pattern_hide(frame_display_t *display, pattern_t *pattern);
extern void frame_display_pad_show(frame_display_t *display, frame_geometry_t *gtab, int nmemb);
extern void frame_display_frame_add(frame_display_t *display, frame_t *frame);
extern void frame_display_frame_remove(frame_display_t *display, frame_t *frame);

#endif /* __FRAME_DISPLAY_H__ */
