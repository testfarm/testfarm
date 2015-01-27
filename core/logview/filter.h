/****************************************************************************/
/* TestFarm                                                                 */
/* Log Viewer - Filter list selectors                                       */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 07-DEC-1999                                                    */
/****************************************************************************/

/*
 * $Revision: 42 $
 * $Date: 2006-06-03 15:30:01 +0200 (sam., 03 juin 2006) $
 */

#ifndef __FILTER_H__
#define __FILTER_H__

typedef struct filter_s filter_t;
typedef void filter_event_t(filter_t *fl, void *arg, void *ptr);

typedef struct {
  char *id;
  void *ptr;
  int selected;
} filter_item_t;

struct filter_s {
  filter_item_t **items;
  int nitems;

  GtkWidget *window;
  GtkWidget *vbox;
  GtkWidget *scrolled;
  GtkWidget *clist;
  GtkWidget *button_hbox;
  GtkWidget *button_select;
  GtkWidget *button_unselect;

  filter_event_t *destroyed;
  void *destroyed_arg;
  filter_event_t *select;
  void *select_arg;
  filter_event_t *unselect;
  void *unselect_arg;
};


extern filter_item_t *filter_retrieve(filter_t *fl, char *id);
extern filter_item_t *filter_add(filter_t *fl, char *id, void *ptr);
extern filter_t *filter_init(void);
extern filter_item_t *filter_feed(filter_t *filter, char *id, void *ptr);
extern void filter_done(filter_t *fl);
extern void filter_window(filter_t *fl, char *title);
extern void filter_show(filter_t *fl);
extern void filter_hide(filter_t *fl);
extern void filter_connect_destroyed(filter_t *fl, filter_event_t *handler, void *arg);
extern void filter_connect_select(filter_t *fl, filter_event_t *handler, void *arg);
extern void filter_connect_unselect(filter_t *fl, filter_event_t *handler, void *arg);

#endif /* __FILTER_H__ */
