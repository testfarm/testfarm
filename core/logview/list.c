/****************************************************************************/
/* TestFarm                                                                 */
/* Log Viewer - Log list management                                         */
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <malloc.h>
#include <errno.h>
#include <gtk/gtk.h>

#include "useful.h"
#include "filter.h"
#include "list.h"


/*================================================================*/
/* List rows actions                                              */
/*================================================================*/

static int get_last_row(GtkTreeModel *model, GtkTreeIter *iter)
{
  int n;

  n = gtk_tree_model_iter_n_children(model, NULL);
  if ( n <= 0 )
    return FALSE;

  return gtk_tree_model_iter_nth_child(model, iter, NULL, n-1);
}


static void scroll_to_row(GtkTreeView *tree, GtkTreeIter *iter)
{
  GtkTreeModel *model = gtk_tree_view_get_model(tree);
  GtkTreePath *path;

  path = gtk_tree_model_get_path(model, iter);
  gtk_tree_view_scroll_to_cell(tree, path, NULL, TRUE, 0.5, 0);
  gtk_tree_path_free(path);
}


static void scroll_to_last_row(GtkTreeView *tree)
{
  GtkTreeModel *model = gtk_tree_view_get_model(tree);
  GtkTreeIter iter;

  if ( get_last_row(model, &iter) )
    scroll_to_row(tree, &iter);
}


static int row_is_visible(GtkTreeView *tree, GtkTreeIter *iter)
{
  GtkTreeModel *model = gtk_tree_view_get_model(tree);
  GtkTreePath *path;
  GdkRectangle rect;
  GdkRectangle visible;

  path = gtk_tree_model_get_path(model, iter);
  gtk_tree_view_get_cell_area(tree, path, gtk_tree_view_get_column(tree, 0), &rect);
  gtk_tree_path_free(path);

  gtk_tree_view_get_visible_rect(tree, &visible);
  //fprintf(stderr, "-- visible.height=%d rect.y=%d => %s\n", visible.height, rect.y, ((rect.y > 0) && ((rect.y - 1) <= visible.height)) ? "visible":"hidden");

 return (rect.y > 0) && ((rect.y - 1) <= visible.height);
}


static int last_row_is_visible(GtkTreeView *tree)
{
  GtkTreeModel *model = gtk_tree_view_get_model(tree);
  GtkTreeIter iter;

  if ( ! get_last_row(model, &iter) )
    return 0;

  return row_is_visible(tree, &iter);
}


/*================================================================*/
/* Log file access                                                */
/*================================================================*/

static void list_file_close(list_t *list)
{
  if ( list->f != NULL )
    fclose(list->f);
  list->f = NULL;
}

static int list_file_open(list_t *list)
{
  list_file_close(list);

  if ( (list->f = fopen(list->filename, "r")) == NULL ) {
    //fprintf(stderr, "Cannot open log file %s: %s\n", list->filename, strerror(errno));
    return -1;
  }

  return 0;
}


/*================================================================*/
/* Time stamps management                                         */
/*================================================================*/

static int list_dt_strtime(char *buf, long long t)
{
  long long ms = t / 1000;
  int len;

  if ( ms >= 10000 ) {
    unsigned long s = ms / 1000;
    len = sprintf(buf, "%lu.%03ds", s, (int) (ms % 1000));
  }
  else {
    len = sprintf(buf, "%lld.%03dms", ms, (int) (t % 1000));
  }

  return len;
}

static void list_dt_compute(list_t *list, list_item_t *cur, list_item_t *prev)
{
  long long global_dt;
  long long local_dt;
  char buf[80];
  char *p;

  /* Compute global dt */
  global_dt = -1;
  if ( (prev->global_ts >= 0) && (cur->global_ts >= prev->global_ts) )
    global_dt = cur->global_ts - prev->global_ts;

  /* Compute local dt if relevant */
  local_dt = -1;
  if ( prev->periph == cur->periph ) {
    /* Compute local dt */
    if ( (prev->local_ts >= 0) && (cur->local_ts >= prev->local_ts) )
      local_dt = cur->local_ts - prev->local_ts;
  }

  /* Global time */
  if ( global_dt >= 0 ) {
    int n = prev->index + 1;

    p = buf;
    if ( n != cur->index )
      p += sprintf(p, "[%d] ", n);
    p += list_dt_strtime(p, global_dt);
    cur->global_dt = strdup(buf);
  }

  /* Local time (if relevant) */
  if ( local_dt >= 0 ) {
    p = buf;
    p += list_dt_strtime(p, local_dt);
    cur->local_dt = strdup(buf);
  }
}


static void list_dt_clear(list_item_t *item)
{
  if ( item->global_dt != NULL )
    free(item->global_dt);
  item->global_dt = NULL;

  if ( item->local_dt != NULL )
    free(item->local_dt);
  item->local_dt = NULL;
}


static void list_dt_show(list_t *list, list_item_t *item)
{
  GtkTreeIter iter;

  if ( item->path == NULL )
    return;
  if ( ! gtk_tree_model_get_iter(GTK_TREE_MODEL(list->store), &iter, item->path) )
    return;

  gtk_list_store_set(list->store, &iter,
		     LIST_GLOBAL_DT, item->global_dt,
		     LIST_LOCAL_DT, item->local_dt,
		     -1);
}


static void list_dt_update(list_t *list, GtkTreeIter *iter, int selected)
{
  list_item_t *cur;
  list_item_t *prev;
  int i;

  /* Get item attached to current row */
  gtk_tree_model_get(GTK_TREE_MODEL(list->store), iter,
		     LIST_ITEM, &cur,
		     -1);

  /* Retrieve previous selected item */
  prev = NULL;
  for (i = cur->index - 1; (i >= 0) && (prev == NULL); i--) {
    prev = list->items[i];
    if ( prev->global_dt == NULL )
      prev = NULL;
  }
  if ( prev == NULL )
    prev = list->items[0];

  /* Update current item */
  list_dt_clear(cur);
  if ( selected )
    list_dt_compute(list, cur, prev);
  list_dt_show(list, cur);

  /* Retrieve next selected item */
  i = cur->index + 1;
  while ( (i < list->nitems) && (list->items[i]->global_dt == NULL) )
    i++;

  /* Update next selected item */
  if ( i < list->nitems ) {
    list_item_t *next = list->items[i];
    list_dt_clear(next);
    list_dt_compute(list, next, (cur->global_dt != NULL) ? cur : prev);
    list_dt_show(list, next);
  }
}


/*================================================================*/
/* Items selection                                                */
/*================================================================*/

static gboolean list_select(GtkTreeSelection *selection, GtkTreeModel *model,
			    GtkTreePath *path, gboolean path_currently_selected,
			    list_t *list)
{
  int new_state = path_currently_selected ? 0:1;

  if ( ! list->freeze ) {
    GtkTreeIter iter;
    gtk_tree_model_get_iter(model, &iter, path);
    list_dt_update(list, &iter, new_state);
  }

  return TRUE;
}


void list_select_clear(list_t *list)
{
  gtk_tree_selection_unselect_all(list->selection);
}


/*================================================================*/
/* List feeding                                                   */
/*================================================================*/

static char *list_background(list_t *list, list_item_t *item)
{
  char *background = NULL;

  /* ENGINE log items are shown with red foreground */
  if ( item->flags & LIST_FLAG_ENGINE ) {
    if ( item->flags & LIST_FLAG_WITHIN_CASE )
      background = "#EFEFE0";

    if ( item->flags & LIST_FLAG_WAIT ) {
      if ( item->flags & LIST_FLAG_CONTINUE )
	background = "#FFC";
      else if ( item->flags & LIST_FLAG_TIMEOUT )
	background = "#FC9";
    }
    else if ( item->flags & LIST_FLAG_VERDICT ) {
      if ( item->flags & LIST_FLAG_PASSED )
	background = "#6F9";
      else if ( item->flags & LIST_FLAG_FAILED )
	background = "#F66";
      else if ( item->flags & LIST_FLAG_INCONCLUSIVE )
	background = "#F6CB69";
    }
  }
  else {
    if ( item->flags & LIST_FLAG_WITHIN_CASE )
      background = "#EFEFE0";
  }

  return background;
}


static char *list_field(char **s)
{
  char *field = *s;
  char *p;

  if ( field == NULL )
    return "?";

  p = strskip_chars(field);
  if ( *p != NUL ) {
    *(p++) = NUL;
    p = strskip_spaces(p);
  }

  *s = p;

  return field;
}


static long long list_field_tstamp(char **s)
{
  long long ts = -1;
  char *p = list_field(s);

  if ( (*p != '\0') && (*p != '*') )
    sscanf(p, "%lld", &ts);

  return ts;
}


static void list_feed(list_t *list, list_item_t *item)
{
  char *buf = NULL;
  int size = 0;
  char lineno[20];
  char *date, *time, *info;
  char global_ts[20] = "*";
  char local_ts[20] = "*";
  GtkTreeIter iter;
  char *background;
  char *foreground;

  /* Seek log line */
  if ( fseek(list->f, item->offset, SEEK_SET) == 0 )
    fgets2(list->f, &buf, &size);
  else
    fprintf(stderr, "Cannot seek log line %s:%d: %s\n", list->filename, item->index+1, strerror(errno));

  snprintf(lineno, sizeof(lineno), "%d", item->index+1);

  /* Truncate info line to a reasonable value */
  if ( size > 512 ) {
    buf[509] = '.';
    buf[510] = '.';
    buf[511] = '.';
    buf[512] = '\0';
  }

  info = strxml(buf);

  date = list_field(&info); /* Get date */
  time = list_field(&info); /* Get time */
  list_field(&info);        /* Skip global TS */
  list_field(&info);        /* Skip interface id */
  list_field(&info);        /* Skip local TS */
  list_field(&info);        /* Skip tag */

  if ( item->global_ts >= 0 )
    snprintf(global_ts, sizeof(global_ts), "%lld.%06lld", item->global_ts/1000000, item->global_ts%1000000);

  if ( item->local_ts >= 0 )
    snprintf(local_ts, sizeof(local_ts), "%lld.%06lld", item->local_ts/1000000, item->local_ts%1000000);

  gtk_list_store_append(list->store, &iter);
  item->path = gtk_tree_model_get_path(GTK_TREE_MODEL(list->store), &iter);
  gtk_list_store_set(list->store, &iter,
		     LIST_LINENO,    lineno,
		     LIST_DATE,      date,
		     LIST_TIME,      time,
		     LIST_GLOBAL_TS, global_ts,
		     LIST_PERIPH,    item->periph,
		     LIST_LOCAL_TS,  local_ts,
		     LIST_TAG,       item->tag,
		     LIST_INFO,      info,
		     LIST_GLOBAL_DT, item->global_dt,
		     LIST_LOCAL_DT,  item->local_dt,
		     LIST_ITEM,      item,
		     -1);

  background = list_background(list, item);
  foreground = (item->flags & LIST_FLAG_ENGINE) ? "#A00" : NULL;

  gtk_list_store_set(list->store, &iter,
		     LIST_BACKGROUND, background,
		     LIST_FOREGROUND, foreground,
		     -1);

  if ( buf != NULL )
    free(buf);
}


/*================================================================*/
/* Filtering events handling                                      */
/*================================================================*/

static void list_filter_rows_realize(list_t *list);

static void event_periph_select(filter_t *filter, list_t *list, list_item_t *item)
{
  char *id = item->periph;
  int i;

  /* Process filtering */
  for (i = 0; i < list->nitems; i++) {
    list_item_t *item = list->items[i];

    if ( item->periph == id ) {
      item->flags &= ~LIST_FLAG_FILTER_PERIPH;
    }
  }

  /* Refresh list display */
  list_filter_rows_realize(list);
}


static void event_periph_unselect(filter_t *filter, list_t *list, list_item_t *item)
{
  char *id = item->periph;
  int i;

  /* Process filtering */
  for (i = 0; i < list->nitems; i++) {
    list_item_t *item = list->items[i];

    if ( item->periph == id ) {
      item->flags |= LIST_FLAG_FILTER_PERIPH;
      list_dt_clear(item);
    }
  }

  /* Refresh list display */
  list_filter_rows_realize(list);
}


static void event_tag_select(filter_t *filter, list_t *list, list_item_t *item)
{
  char *id = item->tag;
  int i;

  /* Process filtering */
  for (i = 0; i < list->nitems; i++) {
    list_item_t *item = list->items[i];

    if ( item->tag == id ) {
      item->flags &= ~LIST_FLAG_FILTER_TAG;
    }
  }

  /* Refresh list display */
  list_filter_rows_realize(list);
}


static void event_tag_unselect(filter_t *filter, list_t *list, list_item_t *item)
{
  char *id = item->tag;
  int i;

  /* Process filtering */
  for (i = 0; i < list->nitems; i++) {
    list_item_t *item = list->items[i];

    if ( item->tag == id ) {
      item->flags |= LIST_FLAG_FILTER_TAG;
      list_dt_clear(item);
    }
  }

  /* Refresh list display */
  list_filter_rows_realize(list);
}


static void event_case_select(filter_t *filter, list_t *list, list_item_t *item)
{
  int i;

  /* Clear filter masks for the Test Case */
  for (i = item->index; i < list->nitems; i++) {
    list_item_t *cur = list->items[i];

    cur->flags &= ~LIST_FLAG_FILTER_CASE;

    if ( cur->flags & LIST_FLAG_DONE )
      break;
  }

  /* Refresh list display */
  list_filter_rows_realize(list);
}


static void event_case_unselect(filter_t *filter, list_t *list, list_item_t *item)
{
  int i;

  /* Process filtering */
  for (i = item->index; i < list->nitems; i++) {
    list_item_t *cur = list->items[i];

    cur->flags |= LIST_FLAG_FILTER_CASE;
    list_dt_clear(cur);

    if ( cur->flags & LIST_FLAG_DONE )
      break;
  }

  /* Refresh list display */
  list_filter_rows_realize(list);
}


static void event_destroyed(filter_t *filter, void *arg, void *ptr)
{
  list_filter_t *fl = (list_filter_t *) arg;
  if ( fl->destroyed != NULL )
    fl->destroyed(fl->arg);
}


static void list_destroyed_xxx(list_filter_t *fl, list_destroyed_t *destroyed, void *arg)
{
  fl->destroyed = destroyed;
  fl->arg = arg;
  if ( fl->filter != NULL )
    filter_connect_destroyed(fl->filter, event_destroyed, fl);
}


void list_destroyed_periph(list_t *list, list_destroyed_t *destroyed, void *arg)
{
  list_destroyed_xxx(&(list->fl_periph), destroyed, arg);
}


void list_destroyed_tag(list_t *list, list_destroyed_t *destroyed, void *arg)
{
  list_destroyed_xxx(&(list->fl_tag), destroyed, arg);
}


void list_destroyed_case(list_t *list, list_destroyed_t *destroyed, void *arg)
{
  list_destroyed_xxx(&(list->fl_case), destroyed, arg);
}


/*================================================================*/
/* List item retrieval                                            */
/*================================================================*/

void list_moveto_case(list_t *list, char *name)
{
  filter_item_t *fi;
  list_item_t *item = NULL;
  int i = 0;

  /* Retrieve first line of Test Case */
  fi = filter_retrieve(list->fl_case.filter, name);
  if ( fi != NULL )
    item = fi->ptr;

  /* Retrieve first non-filtered line of this test case */
  while ( (item != NULL) && (item->flags & LIST_FLAG_FILTER) ) {
    if ( item->flags & LIST_FLAG_DONE ) {
      item = NULL;
    }
    else {
      i++;
      item = list->items[i];
    }
  }

  /* Jump to the line if not filtered */
  if ( (item != NULL) && (item->path != NULL) ) {
    gtk_tree_view_scroll_to_cell(list->tree, item->path, NULL, FALSE, 0, 0);
  }
}


/*================================================================*/
/* Content search                                                 */
/*================================================================*/

static char *strstr2(char *str, char *text, int case_sensitive)
{
  if ( ! case_sensitive ) {
    char str_buf[strlen(str)+1];
    char *p = str_buf;

    while ( *str != '\0' )
      *(p++) = toupper(*(str++));
    return strstr(str_buf, text);
  }
  else {
    return strstr(str, text);
  }
}


static void list_search_clear(list_t *list)
{
  if ( list->search_path != NULL )
    gtk_tree_path_free(list->search_path);
  list->search_path = NULL;
}


list_item_t *list_search_text(list_t *list, char *text, int from_start, int info_only, int case_sensitive)
{
  list_item_t *cur = NULL;
  gboolean good = FALSE;
  GtkTreeIter iter;
  char text_buf[strlen(text)+1];
  int column0;
  int found;

  //fprintf(stderr, "-- SEARCH text='%s' info=%d\n", text, info_only);

  /* Find where the search should start from */
  if ( list->search_path != NULL ) {
    /* Get path of previous successful search */
    gtk_tree_model_get_iter(GTK_TREE_MODEL(list->store), &iter, list->search_path);

    /* Remove search result highlighting */
    gtk_list_store_set(list->store, &iter,
		       LIST_LINENO_BACKGROUND, NULL,
		       -1);

    list_search_clear(list);
 
    /* Get next row. If end reached, wrap to first row */
    good = gtk_tree_model_iter_next(GTK_TREE_MODEL(list->store), &iter);
    if ( ! good )
      from_start = 1;
  }
  else {
    from_start = 1;
  }

  if ( from_start ) {
    good = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(list->store), &iter);
  }

  /* Return now if no valid start row found */
  if ( ! good )
    return NULL;

  /* Make all chars capitals if search is not case sensitive */
  if ( ! case_sensitive ) {
    strcpy(text_buf, text);
    text = strupper(text_buf);
  }

  /* Search for text in Log list */
  column0 = info_only ? LIST_INFO : LIST_DATE;
  found = 0;
  while ( good && (!found) ) {
    int column;
    char *str;

    str = gtk_tree_model_get_string_from_iter(GTK_TREE_MODEL(list->store), &iter);
    g_free(str);

    for (column = column0; (column <= LIST_INFO) && (!found); column++) {
      gtk_tree_model_get(GTK_TREE_MODEL(list->store), &iter,
			 column, &str,
			 -1);
      if ( strstr2(str, text, case_sensitive) != NULL )
	found = 1;
    }

    if ( ! found )
      good = gtk_tree_model_iter_next(GTK_TREE_MODEL(list->store), &iter);
  }

  if ( found ) {
    gtk_list_store_set(list->store, &iter,
		       LIST_LINENO_BACKGROUND, "black",
		       -1);

    if ( ! row_is_visible(list->tree, &iter) )
      scroll_to_row(list->tree, &iter);

    list->search_path = gtk_tree_model_get_path(GTK_TREE_MODEL(list->store), &iter);
  }

  return cur;
}


/*================================================================*/
/* List filtering                                                 */
/*================================================================*/

static void list_filter_init(list_t *list)
{
  /* Init filter lists */
  list->fl_periph.filter = filter_init();
  list->fl_tag.filter = filter_init();
  list->fl_case.filter = filter_init();

  /* Connect filtering event handlers */
  filter_connect_select(list->fl_periph.filter, (filter_event_t *) event_periph_select, list);
  filter_connect_unselect(list->fl_periph.filter, (filter_event_t *) event_periph_unselect, list);
  filter_connect_destroyed(list->fl_periph.filter, event_destroyed, &(list->fl_periph));

  filter_connect_select(list->fl_tag.filter, (filter_event_t *) event_tag_select, list);
  filter_connect_unselect(list->fl_tag.filter, (filter_event_t *) event_tag_unselect, list);
  filter_connect_destroyed(list->fl_tag.filter, event_destroyed, &(list->fl_tag));

  filter_connect_select(list->fl_case.filter, (filter_event_t *) event_case_select, list);
  filter_connect_unselect(list->fl_case.filter, (filter_event_t *) event_case_unselect, list);
  filter_connect_destroyed(list->fl_case.filter, event_destroyed, &(list->fl_case));
}


static void list_filter_done(list_t *list)
{
  if ( list->fl_periph.filter != NULL ) {
    filter_done(list->fl_periph.filter);
    list->fl_periph.filter = NULL;
  }

  if ( list->fl_tag.filter != NULL ) {
    filter_done(list->fl_tag.filter);
    list->fl_tag.filter = NULL;
  }

  if ( list->fl_case.filter != NULL ) {
    filter_done(list->fl_case.filter);
    list->fl_case.filter = NULL;
  }
}


static void list_filter_rows_realize(list_t *list)
{
  list_item_t *prev;
  int i;

  if ( list_file_open(list) )
    return;

  /* Clear last search result */
  list_search_clear(list);

  /* Disable selection update */
  list->freeze = 1;

  /* Clear list */
  gtk_list_store_clear(list->store);

  prev = list->items[0];
  for (i = 0; i < list->nitems; i++) {
    list_item_t *item = list->items[i];

    if ( item->path != NULL )
      gtk_tree_path_free(item->path);
    item->path = NULL;

    if ( (item->flags & LIST_FLAG_FILTER) == 0 ) {
      list_feed(list, item);

      if ( item->global_dt != NULL ) {
	gtk_tree_selection_select_path(list->selection, item->path);

        list_dt_clear(item);
        list_dt_compute(list, item, prev);
        list_dt_show(list, item);
        prev = item;
      }
    }
  }

  /* Re-enable selection update */
  list->freeze = 0;

  list_file_close(list);
}


/*================================================================*/
/* Filtering windows                                              */
/*================================================================*/

static void list_filter_xxx(list_t *list, list_filter_t *fl, char *title, int status)
{
  if ( fl->filter != NULL ) {
    filter_window(fl->filter, title);

    if ( status )
      filter_show(fl->filter);
    else
      filter_hide(fl->filter);
  }
  else {
    if ( fl->destroyed != NULL )
      fl->destroyed(fl->arg);
  }
}


void list_filter_periph(list_t *list, int status)
{
  list_filter_xxx(list, &(list->fl_periph), "Interfaces", status);
}


void list_filter_tag(list_t *list, int status)
{
  list_filter_xxx(list, &(list->fl_tag), "Tags", status);
}


void list_filter_case(list_t *list, int status)
{
  list_filter_xxx(list, &(list->fl_case), "Test cases", status);
}


/*================================================================*/
/* List loading                                                   */
/*================================================================*/

static void list_load_data(list_t *list, int offset, char *line)
{
  list_item_t *item;
  filter_item_t *fi;
  int within_case;
  int from_engine = 0;
  char *field = line;
  char *s;

  /* Determine initial WITHIN_CASE flag */
  within_case = 0;
  if ( list->nitems > 0 ) {
    unsigned int flags = list->items[list->nitems-1]->flags;
    within_case = (flags & LIST_FLAG_WITHIN_CASE) && !(flags & LIST_FLAG_DONE);
  }

  /* Create new Log item */
  list->items = (list_item_t **) realloc(list->items, sizeof(list_item_t *) * ((list->nitems)+2));
  item = (list_item_t *) malloc(sizeof(list_item_t));
  list->items[list->nitems] = item;

  item->index = list->nitems;
  item->offset = offset;
  item->path = NULL;
  (list->nitems)++;

  /* Skip HR date */
  field = strskip_spaces(strskip_chars(field));

  /* Skip HR time */
  field = strskip_spaces(strskip_chars(field));

  /* Get global time stamp */
  item->global_ts = list_field_tstamp(&field);

  /* Get interface id */
  item->periph = "";
  s = list_field(&field);
  if ( *s != '\0' ) {
    fi = filter_feed(list->fl_periph.filter, s, item);
    item->periph = fi->id;
  }

  /* Get local time stamp */
  item->local_ts = list_field_tstamp(&field);

  /* Get tag */
  item->tag = "";
  s = list_field(&field);
  if ( *s != '\0' ) {
    fi = filter_feed(list->fl_tag.filter, s, item);
    item->tag = fi->id;
  }

  item->global_dt = NULL;
  item->local_dt = NULL;

  /* Set item flags */
  item->flags = 0;
  if ( strcmp(item->periph, "ENGINE") == 0 ) {
    from_engine = 1;
    item->flags |= LIST_FLAG_ENGINE;
  }

  /* Manage Test Case begining */
  if ( from_engine && (strcmp(item->tag, "CASE") == 0) ) {
    fi = filter_feed(list->fl_case.filter, field, item);
    within_case = 1;
    item->flags |= LIST_FLAG_CASE;
  }

  /* Set Test Case name */
  if ( within_case )
    item->flags |= LIST_FLAG_WITHIN_CASE;

  /* Set flags for Test Engine events colorization */
  if ( from_engine ) {
    if ( strcmp(item->tag, "WAIT") == 0 ) {
      if ( strncmp(field, "CONTINUE ", 9) == 0 )
        item->flags |= LIST_FLAG_CONTINUE;
      else if ( strncmp(field, "TIMEOUT ", 8) == 0 )
        item->flags |= LIST_FLAG_TIMEOUT;
    }
    else if ( strcmp(item->tag, "VERDICT") == 0 ) {
      char *p = strskip_chars(field);
      char c = *p;

      *p = NUL;

      if ( strcmp(field, "PASSED") == 0 )
        item->flags |= LIST_FLAG_PASSED;
      else if ( strcmp(field, "FAILED") == 0 )
        item->flags |= LIST_FLAG_FAILED;
      else if ( strcmp(field, "INCONCLUSIVE") == 0 )
        item->flags |= LIST_FLAG_INCONCLUSIVE;

      *p = c;
    }
  }

  /* Manage Test Case end */
  if ( from_engine && (strcmp(item->tag, "DONE") == 0) ) {
    item->flags |= LIST_FLAG_DONE;
  }
}


static void list_load_feed(list_t *list, int from)
{
  int i;

  /* Feed and display the list */
  for (i = from; i < list->nitems; i++)
    list_feed(list, list->items[i]);
}


static void list_load_fetch(list_t *list)
{
  char *buf = NULL;
  int size = 0;
  int n = list->nitems;

  if ( ftell(list->f) == 0 ) {
    int is_xml = 0;
    int log_found = 0;

    /* Reach the LOG section (if any) */
    while ( fgets2(list->f, &buf, &size) >= 0 ) {
      if ( buf != NULL ) {
	if ( strcmp(buf, "<RESULT>") == 0 ) {
	  is_xml = 1;
	}
	else if ( is_xml && strcmp(buf, "<LOG>") == 0 ) {
	  log_found = 1;
	  break;
	}
      }
    }

    if ( ! log_found ) {
      rewind(list->f);

      /* Abort if log not found in XML file */
      if ( is_xml )
	return;
    }
  }

  /* Parse log file */
  while ( !feof(list->f) ) {
    int offset = ftell(list->f);
    int len = fgets2(list->f, &buf, &size);
    char *line;

    /* Abort on i/o error */
    if ( len < 0 )
      break;

    /* Ignore empty lines */
    if ( (len == 0) || (buf == NULL) )
      continue;
    line = strskip_spaces(buf);
    strxml(line);

    /* Ignore XML-related lines */
    if ( line[0] == '<' ) {
      if ( strcmp(line, "</LOG>") == 0 )
        break;
      else
        continue;
    }

    list_load_data(list, offset, line);
  }

  /* Update follow index */
  list->filesize = ftell(list->f);

  /* Put item list termination mark */
  if ( list->items != NULL )
    list->items[list->nitems] = NULL;

  /* Free fetch buffer */
  if ( buf != NULL )
    free(buf);

  /* Feed and display the list */
  list_load_feed(list, n);
}


int list_load(list_t *list, char *filename)
{
  /* Clear previous data */
  list_unload(list);

  /* Open log file */
  list->filename = strdup(filename);
  if ( list_file_open(list) )
    return -1;

  /* Init Interfaces/Tags/Cases filtering lists */
  list_filter_init(list);

  /* Fetch log file */
  list_load_fetch(list);

  /* Close log file */
  list_file_close(list);

  return 0;
}


void list_unload(list_t *list)
{
  /* Close log file stream */
  list_file_close(list);

  /* Clear current list */
  gtk_list_store_clear(list->store);

  /* Free result log storage */
  if ( list->items != NULL ) {
    int i;

    for (i = 0; i < list->nitems; i++) {
      list_item_t *item = list->items[i];
      list_dt_clear(item);
      free(item);
    }

    free(list->items);
    list->items = NULL;
  }

  list->nitems = 0;

  list_search_clear(list);

  /* Clear file specs */
  if ( list->filename != NULL )
    free(list->filename);
  list->filename = NULL;
  list->filesize = 0;

  /* Free filter lists */
  list_filter_done(list);
}


int list_follow(list_t *list, int moveto)
{
  int scroll;

  /* Check file name */
  if ( list->filename == NULL )
    return 0;

  /* Open log file */
  if ( list_file_open(list) )
    return -1;

  /* Reach last storage point */
  if ( fseek(list->f, list->filesize, SEEK_SET) == -1 ) {
    fprintf(stderr, "Cannot seek log file %s: %s\n", list->filename, strerror(errno));
    list_file_close(list);
    return -1;
  }

  /* Get last row visibility */
  scroll = last_row_is_visible(list->tree);

  /* Fetch log file */
  list_load_fetch(list);

  /* Close log file */
  list_file_close(list);

  /* Spot the end of the log if the former last row was visible */
  if ( (list->nitems > 0) && (moveto || scroll) ) {
    /* Flush GTK processing queue */
    while (gtk_events_pending() )
      gtk_main_iteration ();

    scroll_to_last_row(list->tree);
  }

  return 0;
}


/*================================================================*/
/* List creation / destruction                                    */
/*================================================================*/

static void list_init_treeview(list_t *list, GtkWidget *widget)
{
  GtkTreeViewColumn *column;
  GtkCellRenderer *renderer;
  GtkCellRenderer *renderer_dt;
  int n;

  /* Create a list model */
  list->store = gtk_list_store_new(LIST_NFIELDS,
				   G_TYPE_STRING,     /* LINENO */
				   G_TYPE_STRING,     /* DATE */
				   G_TYPE_STRING,     /* TIME */
				   G_TYPE_STRING,     /* GLOBAL TS */
				   G_TYPE_STRING,     /* INTERFACE */
				   G_TYPE_STRING,     /* LOCAL TS */
				   G_TYPE_STRING,     /* TAG */
				   G_TYPE_STRING,     /* INFO */
				   G_TYPE_STRING,     /* GLOBAL DT */
				   G_TYPE_STRING,     /* LOCAL DT */
				   G_TYPE_STRING,     /* LINENO_BACKGROUND */
				   G_TYPE_STRING,     /* BACKGROUND */
				   G_TYPE_STRING,     /* FOREGROUND */
				   G_TYPE_POINTER);   /* ITEM */

  /* Create a view */
  list->tree = GTK_TREE_VIEW(gtk_tree_view_new_with_model(GTK_TREE_MODEL(list->store)));
  gtk_container_add(GTK_CONTAINER(list->scrolled), GTK_WIDGET(list->tree));
  //gtk_tree_view_set_enable_search(list->tree, TRUE);
  gtk_widget_show(GTK_WIDGET(list->tree));

  /* The view now holds a reference. We can get rid of our own reference */
  g_object_unref(G_OBJECT(list->store));

  /* Setup tree selection handler */
  list->selection = gtk_tree_view_get_selection(list->tree);
  gtk_tree_selection_set_mode(list->selection, GTK_SELECTION_MULTIPLE);
  gtk_tree_selection_set_select_function(list->selection,
					 (GtkTreeSelectionFunc) list_select, list, NULL);

  /* Col #0: Line number */
  renderer = gtk_cell_renderer_text_new();
  gtk_object_set(GTK_OBJECT(renderer), "font", "Sans Italic 7", NULL);
  gtk_object_set(GTK_OBJECT(renderer), "ypad", 0, NULL);
  gtk_object_set(GTK_OBJECT(renderer), "foreground", "#777", NULL);
  gtk_tree_view_insert_column_with_attributes(list->tree, -1, "", renderer,
					      "text", LIST_LINENO,
					      "background", LIST_LINENO_BACKGROUND,
					      NULL);
  
  /* Col #1: Date */
  renderer = gtk_cell_renderer_text_new();
  gtk_object_set(GTK_OBJECT(renderer), "font", "Monospace 9", NULL);
  gtk_object_set(GTK_OBJECT(renderer), "ypad", 0, NULL);
  n = gtk_tree_view_insert_column_with_attributes(list->tree, -1, "Date", renderer,
						  "text", LIST_DATE,
						  "background", LIST_BACKGROUND,
						  "foreground", LIST_FOREGROUND,
						  NULL);
  //column = gtk_tree_view_get_column(list->tree, n-1);
  //gtk_tree_view_column_set_visible(column, FALSE);

  /* Col #2: Time */
  gtk_tree_view_insert_column_with_attributes(list->tree, -1, "Time", renderer,
					      "text", LIST_TIME,
					      "background", LIST_BACKGROUND,
					      "foreground", LIST_FOREGROUND,
					      NULL);

  /* Col #3: Global TS */
  gtk_tree_view_insert_column_with_attributes(list->tree, -1, "Global TS", renderer,
					      "text", LIST_GLOBAL_TS,
					      "background", LIST_BACKGROUND,
					      "foreground", LIST_FOREGROUND,
					      NULL);

  /* Col #4: Global dT */
  renderer_dt = gtk_cell_renderer_text_new();
  gtk_object_set(GTK_OBJECT(renderer_dt), "font", "Sans Italic 8", NULL);
  gtk_object_set(GTK_OBJECT(renderer_dt), "ypad", 0, NULL);
  gtk_tree_view_insert_column_with_attributes(list->tree, -1, "Global dT", renderer_dt,
					      "text", LIST_GLOBAL_DT,
					      "background", LIST_BACKGROUND,
					      "foreground", LIST_FOREGROUND,
					      NULL);

  /* Col #5: Interface */
  gtk_tree_view_insert_column_with_attributes(list->tree, -1, "Interface", renderer,
					      "text", LIST_PERIPH,
					      "background", LIST_BACKGROUND,
					      "foreground", LIST_FOREGROUND,
					      NULL);

  /* Col #6: Local TS */
  gtk_tree_view_insert_column_with_attributes(list->tree, -1, "Local TS", renderer,
					      "text", LIST_LOCAL_TS,
					      "background", LIST_BACKGROUND,
					      "foreground", LIST_FOREGROUND,
					      NULL);

  /* Col #7: Local dT */
  gtk_tree_view_insert_column_with_attributes(list->tree, -1, "Local dT", renderer_dt,
					      "text", LIST_LOCAL_DT,
					      "background", LIST_BACKGROUND,
					      "foreground", LIST_FOREGROUND,
					      NULL);

  /* Col #8: Tag */
  gtk_tree_view_insert_column_with_attributes(list->tree, -1, "Tag", renderer,
					      "text", LIST_TAG,
					      "background", LIST_BACKGROUND,
					      "foreground", LIST_FOREGROUND,
					      NULL);

  /* Col #9: Info */
  n = gtk_tree_view_insert_column_with_attributes(list->tree, -1, "Info", renderer,
						  "text", LIST_INFO,
						  "background", LIST_BACKGROUND,
						  "foreground", LIST_FOREGROUND,
						  NULL);
  column = gtk_tree_view_get_column(list->tree, n-1);
  gtk_tree_view_column_set_expand(column, TRUE);
}


list_t *list_init(GtkWidget *widget)
{
  list_t *list;

  /* Allocate list display object */
  list = (list_t *) malloc(sizeof(list_t));

  /* Create a scrolled window to pack the list widget into */
  list->scrolled = gtk_scrolled_window_new(NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW(list->scrolled),
                                  GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);
  gtk_box_pack_start(GTK_BOX(widget), list->scrolled, TRUE, TRUE, 0);
  gtk_widget_show(list->scrolled);

  /* Create the list widget */
  list_init_treeview(list, widget);

  /* Init file specs */
  list->filename = NULL;
  list->filesize = 0;
  list->f = NULL;

  /* Init the list storage */
  list->items = NULL;
  list->nitems = 0;
  list->freeze = 0;

  /* Init Interfaces/Tags/Cases filtering lists */
  list->fl_periph.filter = NULL;
  list->fl_periph.destroyed = NULL;
  list->fl_periph.arg = NULL;
  list->fl_tag.filter = NULL;
  list->fl_tag.destroyed = NULL;
  list->fl_tag.arg = NULL;
  list->fl_case.filter = NULL;
  list->fl_case.destroyed = NULL;
  list->fl_case.arg = NULL;

  /* Init text search index */
  list->search_path = NULL;

  return list;
}


void list_done(list_t *list)
{
  if ( list == NULL )
    return;

  list_unload(list);

  gtk_widget_destroy(GTK_WIDGET(list->tree));
  gtk_widget_destroy(list->scrolled);

  free(list);
}
