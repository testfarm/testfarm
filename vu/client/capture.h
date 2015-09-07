/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* Capture Device Management                                                */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 25-APR-2007                                                    */
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

#ifndef __TVU_CAPTURE_H__
#define __TVU_CAPTURE_H__

#include "capture_interface.h"

#define CAPTURE_TAG "CAPTURE"

extern int capture_init(char *method);

extern capture_t *capture_open(char *device, capture_opt_t opt, capture_update_fn *update, void *data);
extern void capture_close(capture_t *capture);

extern int capture_set_window(capture_t *capture, frame_geometry_t *g);
extern int capture_get_window(capture_t *capture, frame_geometry_t *g);
extern long capture_set_period(capture_t *capture, long delay);
extern long capture_get_period(capture_t *capture);
extern int capture_refresh(capture_t *capture);

extern void capture_action_key(capture_t *capture, int down, unsigned long key);
extern void capture_action_pointer(capture_t *capture, unsigned char buttons, unsigned int x, unsigned int y);
extern void capture_action_scroll(capture_t *capture, unsigned char direction);

extern int capture_attr_set(capture_t *capture, char *key, char *value);
extern capture_attr_t *capture_attr_get(capture_t *capture, char *key, int *nmemb);

extern void capture_show_status(capture_t *capture, char *hdr);

#endif /* __TVU_CAPTURE_H__ */
