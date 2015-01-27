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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <gtk/gtk.h>

#include "filter.h"

#define FILTER_MIN_WIDTH 180
#define FILTER_MIN_HEIGHT 160

filter_item_t *filter_retrieve(filter_t *fl, char *id)
{
  int i;

 if ( fl == NULL )
   return NULL;
  if ( fl->items == NULL )
    return NULL;

  for (i = 0; i < fl->nitems; i++) {
    filter_item_t *item = fl->items[i];
    if ( strcmp(item->id, id) == 0 )
      return item;
  }

  return NULL;
}


filter_item_t *filter_add(filter_t *fl, char *id, void *ptr)
{
  filter_item_t *item;

  fl->items = (filter_item_t **) realloc(fl->items, sizeof(filter_item_t *) * (fl->nitems + 1));
  item = (filter_item_t *) malloc(sizeof(filter_item_t));
  fl->items[(fl->nitems)++] = item;
  item->id = strdup(id);
  item->ptr = ptr;
  item->selected = 1;

  return item;
}


static int filter_sort_op(filter_item_t **i1, filter_item_t **i2)
{
  return strcmp((*i1)->id, (*i2)->id);
}


static void filter_feed_list(filter_t *filter)
{
  int i;

  /* Clear the list */
  gtk_clist_clear((GtkCList *) filter->clist);

  /* Feed the list */
  for (i = 0; i < filter->nitems; i++) {
    filter_item_t *item = filter->items[i];
    gtk_clist_append((GtkCList *) filter->clist, &(item->id));
    if ( item->selected )
      gtk_clist_select_row((GtkCList *) filter->clist, i, 0);
  }

  /*Set window size to fit the list content */
  gtk_clist_columns_autosize((GtkCList *) filter->clist);
  i = 60 + gtk_clist_optimal_column_width((GtkCList *) filter->clist, 0);
  if ( i < FILTER_MIN_WIDTH )
    i = FILTER_MIN_WIDTH;
  gtk_widget_set_usize(GTK_WIDGET(filter->window), i, FILTER_MIN_HEIGHT);
}


filter_item_t *filter_feed(filter_t *filter, char *id, void *ptr)
{
  filter_item_t *item;

  /* Accept new elements only */
  item = filter_retrieve(filter, id);
  if ( item != NULL )
    return item;

  /* Add new element to the list */
  item = filter_add(filter, id, ptr);

  /* Sort filter list */
  qsort(filter->items, filter->nitems, sizeof(filter_item_t *), (void *) filter_sort_op);

  /* Update window */
  if ( filter->window != NULL ) {
    gtk_clist_freeze((GtkCList *) filter->clist);
    filter_feed_list(filter);
    gtk_clist_thaw((GtkCList *) filter->clist);
  }

  return item;
}


filter_t *filter_init(void)
{
  filter_t *fl;

  /* Alloc filter list descriptor */
  fl = (filter_t *) malloc(sizeof(filter_t));
  fl->items = NULL;
  fl->nitems = 0;
  fl->window = NULL;
  fl->vbox = NULL;
  fl->scrolled = NULL;
  fl->clist = NULL;
  fl->button_hbox = NULL;
  fl->button_select = NULL;
  fl->button_unselect = NULL;
  fl->destroyed = NULL;
  fl->select = NULL;
  fl->unselect = NULL;

  return fl;
}


void filter_done(filter_t *fl)
{
  int i;

  if ( fl == NULL )
    return;

  /* Destroy window */
  if ( fl->window != NULL ) {
    gtk_widget_destroy(fl->button_select);
    gtk_widget_destroy(fl->button_unselect);
    gtk_widget_destroy(fl->button_hbox);
    gtk_widget_destroy(fl->clist);
    gtk_widget_destroy(fl->scrolled);
    gtk_widget_destroy(fl->vbox);
    gtk_widget_destroy(fl->window);
  }

  /* Free data */
  for (i = 0; i < fl->nitems; i++) {
    free(fl->items[i]->id);
    free(fl->items[i]);
  }
  free(fl->items);

  /* Free descriptor */
  free(fl);
}


static void filter_event_select(GtkWidget *list, gint row, gint column,
                                GdkEventButton *event, gpointer data )
{
  filter_t *fl = (filter_t *) data;

  if ( fl->select != NULL )
    fl->select(fl, fl->select_arg, fl->items[row]->ptr);
}


static void filter_event_unselect(GtkWidget *list, gint row, gint column,
                                  GdkEventButton *event, gpointer data )
{
  filter_t *fl = (filter_t *) data;

  if ( fl->unselect != NULL )
    fl->unselect(fl, fl->unselect_arg, fl->items[row]->ptr);
}


static void filter_event_select_all(GtkWidget *widget, gpointer data)
{
  filter_t *fl = (filter_t *) data;

  gtk_clist_freeze(GTK_CLIST(fl->clist));
  gtk_clist_select_all(GTK_CLIST(fl->clist));
  gtk_clist_thaw(GTK_CLIST(fl->clist));
}


static void filter_event_unselect_all(GtkWidget *widget, gpointer data)
{
  filter_t *fl = (filter_t *) data;

  gtk_clist_freeze(GTK_CLIST(fl->clist));
  gtk_clist_unselect_all(GTK_CLIST(fl->clist));
  gtk_clist_thaw(GTK_CLIST(fl->clist));
}


static void filter_event_destroyed(GtkWidget *window, gpointer data)
{
  filter_t *fl = (filter_t *) data;

  fl->window = NULL;

  if ( fl->destroyed != NULL )
    fl->destroyed(fl, fl->destroyed_arg, NULL);
}


void filter_window(filter_t *fl, char *title)
{
  if ( fl->window != NULL )
    return;

  /* Create window */
  fl->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  /*gtk_widget_set_usize(GTK_WIDGET(fl->window), 150, 300);*/
  gtk_window_set_title(GTK_WINDOW(fl->window), title);
  gtk_signal_connect(GTK_OBJECT(fl->window), "destroy",
                     GTK_SIGNAL_FUNC(filter_event_destroyed), (gpointer) fl);

  /* A vertical box container to put everything in */
  fl->vbox = gtk_vbox_new(FALSE, 5);
  gtk_container_border_width(GTK_CONTAINER(fl->vbox), 5);
  gtk_container_add(GTK_CONTAINER(fl->window), fl->vbox);
  gtk_widget_show(fl->vbox);

  /* Create scrolled window for list display */
  fl->scrolled = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (fl->scrolled),
                                  GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);
  gtk_box_pack_start(GTK_BOX(fl->vbox), fl->scrolled, TRUE, TRUE, 0);
  gtk_widget_show(fl->scrolled);

  /* Create the list */
  fl->clist = gtk_clist_new(1);
  gtk_clist_set_selection_mode(GTK_CLIST(fl->clist), GTK_SELECTION_MULTIPLE);
  gtk_clist_set_shadow_type(GTK_CLIST(fl->clist), GTK_SHADOW_OUT);
  gtk_container_add(GTK_CONTAINER(fl->scrolled), (GtkWidget *) fl->clist);
  gtk_widget_show(fl->clist);

  /* Create control buttons in a button box */
  fl->button_hbox = gtk_hbox_new(FALSE, 0);
  gtk_box_pack_start(GTK_BOX(fl->vbox), fl->button_hbox, FALSE, TRUE, 0);
  gtk_widget_show(fl->button_hbox);

  fl->button_select = gtk_button_new_with_label("Select all");
  gtk_container_set_border_width(GTK_CONTAINER(fl->button_select), 5);
  gtk_box_pack_start(GTK_BOX(fl->button_hbox), fl->button_select, TRUE, TRUE, 0);
  gtk_widget_show(fl->button_select);

  fl->button_unselect = gtk_button_new_with_label("Unselect all");
  gtk_container_set_border_width(GTK_CONTAINER(fl->button_unselect), 5);
  gtk_box_pack_start(GTK_BOX(fl->button_hbox), fl->button_unselect, TRUE, TRUE, 0);
  gtk_widget_show(fl->button_unselect);

  /* Fill up the list */
  filter_feed_list(fl);

  /* Connect list event handlers */
  gtk_signal_connect(GTK_OBJECT(fl->clist), "select_row",
                     GTK_SIGNAL_FUNC(filter_event_select), (gpointer) fl);
  gtk_signal_connect(GTK_OBJECT(fl->clist), "unselect_row",
                     GTK_SIGNAL_FUNC(filter_event_unselect), (gpointer) fl);
  gtk_signal_connect(GTK_OBJECT(fl->button_select), "clicked",
                     GTK_SIGNAL_FUNC(filter_event_select_all),
                     (gpointer) fl);
  gtk_signal_connect(GTK_OBJECT(fl->button_unselect), "clicked",
                     GTK_SIGNAL_FUNC(filter_event_unselect_all),
                     (gpointer) fl);
}


void filter_show(filter_t *fl)
{
  gtk_widget_show(fl->window);
}


void filter_hide(filter_t *fl)
{
  gtk_widget_hide(fl->window);
}


void filter_connect_destroyed(filter_t *fl, filter_event_t *handler, void *arg)
{
  fl->destroyed = handler;
  fl->destroyed_arg = arg;
}


void filter_connect_select(filter_t *fl, filter_event_t *handler, void *arg)
{
  fl->select = handler;
  fl->select_arg = arg;
}


void filter_connect_unselect(filter_t *fl, filter_event_t *handler, void *arg)
{
  fl->unselect = handler;
  fl->unselect_arg = arg;
}
