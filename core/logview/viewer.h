/****************************************************************************/
/* TestFarm                                                                 */
/* Log Viewer - Main window                                                 */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 07-DEC-1999                                                    */
/****************************************************************************/

/*
 * $Revision: 262 $
 * $Date: 2006-10-11 16:17:19 +0200 (mer., 11 oct. 2006) $
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
