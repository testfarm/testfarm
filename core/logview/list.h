/****************************************************************************/
/* TestFarm                                                                 */
/* Log Viewer - Log list management                                         */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 07-DEC-1999                                                    */
/****************************************************************************/

/*
 * $Revision: 300 $
 * $Date: 2006-11-26 14:55:06 +0100 (dim., 26 nov. 2006) $
 */

#ifndef __LIST_H__
#define __LIST_H__

#include <gtk/gtk.h>
#include "filter.h"

enum {
  LIST_LINENO=0,

  LIST_DATE,
  LIST_TIME,
  LIST_GLOBAL_TS,
  LIST_PERIPH,
  LIST_LOCAL_TS,
  LIST_TAG,
  LIST_INFO,
  LIST_GLOBAL_DT,
  LIST_LOCAL_DT,

  LIST_LINENO_BACKGROUND,
  LIST_BACKGROUND,
  LIST_FOREGROUND,
  LIST_ITEM,

  LIST_NFIELDS
};


#define LIST_FLAG_ENGINE        0x0001
#define LIST_FLAG_CASE          0x0002
#define LIST_FLAG_WITHIN_CASE   0x0004
#define LIST_FLAG_DONE          0x0008
#define LIST_FLAG_WAIT          0x00F0
#define LIST_FLAG_CONTINUE      0x0010
#define LIST_FLAG_TIMEOUT       0x0020
#define LIST_FLAG_VERDICT       0x0F00
#define LIST_FLAG_PASSED        0x0100
#define LIST_FLAG_FAILED        0x0200
#define LIST_FLAG_INCONCLUSIVE  0x0400
#define LIST_FLAG_FILTER        0xF000
#define LIST_FLAG_FILTER_PERIPH 0x1000
#define LIST_FLAG_FILTER_TAG    0x2000
#define LIST_FLAG_FILTER_CASE   0x4000


typedef void list_destroyed_t(void *arg);

typedef struct {
  filter_t *filter;
  list_destroyed_t *destroyed;
  void *arg;
} list_filter_t;

typedef struct {
  int index;
  int offset;
  GtkTreePath *path;
  unsigned long flags;
  char *periph;
  char *tag;
  long long local_ts;
  long long global_ts;
  char *global_dt;
  char *local_dt;
} list_item_t;


typedef struct {
  list_item_t **items;
  int nitems;
  int freeze;

  GtkWidget *scrolled;
  GtkListStore *store;
  GtkTreeView *tree;
  GtkTreeSelection *selection;

  char *filename;
  long filesize;
  FILE *f;

  list_filter_t fl_periph;
  list_filter_t fl_tag;
  list_filter_t fl_case;

  GtkTreePath *search_path;
} list_t;


extern list_t *list_init(GtkWidget *widget);
extern void list_done(list_t *list);

extern int list_load(list_t *list, char *filename);
extern void list_unload(list_t *list);
extern int list_follow(list_t *list, int moveto);

extern void list_select_clear(list_t *list);

extern void list_moveto_case(list_t *list, char *name);

extern void list_filter_periph(list_t *list, int status);
extern void list_filter_tag(list_t *list, int status);
extern void list_filter_case(list_t *list, int status);
extern void list_destroyed_periph(list_t *list, list_destroyed_t *destroyed, void *arg);
extern void list_destroyed_tag(list_t *list, list_destroyed_t *destroyed, void *arg);
extern void list_destroyed_case(list_t *list, list_destroyed_t *destroyed, void *arg);

extern list_item_t *list_search_text(list_t *list, char *text,
                                     int from_start, int info_only, int case_sensitive);

#endif /* __LIST_H__ */
