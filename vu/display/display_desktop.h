/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* Display Tool - desktops management                                       */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 15-NOV-2007                                                    */
/****************************************************************************/

/*
 * $Revision: 817 $
 * $Date: 2007-11-27 12:37:15 +0100 (mar., 27 nov. 2007) $
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
