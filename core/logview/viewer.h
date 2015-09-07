/****************************************************************************/
/* TestFarm                                                                 */
/* Log Viewer - Main window                                                 */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 07-DEC-1999                                                    */
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

#ifndef __VIEWER_H__
#define __VIEWER_H__

#include <fam.h>
#include <gtk/gtk.h>

#include "list.h"

typedef void viewer_destroyed_t(void *arg);

typedef struct {
  char *title;
  char *filename;

  FAMConnection fc;
  int fc_tag;
  FAMRequest fc_req;
  int fc_req_ok;

  GdkCursor *cursor_watch;
  GtkWidget *window;
  GtkWidget *filew;
  GtkWidget *button_box;

  GtkWidget *search_combo, *search_info, *search_case;
  char *search_history;
  GList *search_list;

  list_t *list;
  int loaded;

  viewer_destroyed_t *destroyed;
  void *destroyed_arg;
} viewer_t;

extern viewer_t *viewer_init(char *title);
extern void viewer_done(viewer_t *viewer);
extern void viewer_destroyed(viewer_t *viewer, viewer_destroyed_t *destroyed, void *arg);

extern void viewer_load(viewer_t *viewer, char *filename);
extern int viewer_follow(viewer_t *viewer, int moveto);
extern void viewer_clear(viewer_t *viewer);

#endif /* __VIEWER_H__ */
