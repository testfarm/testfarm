/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* Display Tool - desktops management                                       */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 15-NOV-2007                                                    */
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

#ifndef __TVU_DISPLAY_DESKTOP_H__
#define __TVU_DISPLAY_DESKTOP_H__

#include "color.h"
#include "display.h"

#define DISPLAY_DESKTOP_COLOR_S "#0000FF"
#define DISPLAY_DESKTOP_COLOR {0,0,255}
extern color_rgb_t display_desktop_color;

typedef enum {
  DISPLAY_DESKTOP_DISCONNECTED,
  DISPLAY_DESKTOP_CONNECTING,
  DISPLAY_DESKTOP_CONNECTED,
} display_desktop_state_t;

extern display_t *display_current;

extern void display_desktop_init(GtkWindow *window);
extern void display_desktop_done(void);

extern void display_desktop_available(display_t *d, int available);
extern void display_desktop_show(display_t *d, display_desktop_state_t state);

extern int display_desktop_init_root(display_desktop_t *desktop, int shmid);
extern void display_desktop_add_frame(char *id, int shmid, frame_geometry_t *g0, int parent_shmid);
extern void display_desktop_remove_frame(int shmid);
extern frame_hdr_t *display_desktop_get_frame(int shmid);
extern frame_hdr_t *display_desktop_get_current_frame(void);

#endif /* __TVU_DISPLAY_DESKTOP_H__ */
