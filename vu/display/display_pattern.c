/****************************************************************************/
/* TestFarm Virtual User display tool                                       */
/* Pattern management                                                       */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 22-MAR-2007                                                    */
/****************************************************************************/

/*
 * $Revision: 1131 $
 * $Date: 2010-03-31 16:01:06 +0200 (mer., 31 mars 2010) $
 */

#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <gtk/gtk.h>

#include "interface.h"
#include "support.h"
#include "window_icon.h"

#include "frame_ctl.h"
#include "frame_ctl_msg.h"
#include "pattern.h"
#include "xpm.h"
#include "utils.h"
#include "editor.h"
#include "viewer.h"
#include "display.h"
#include "display_desktop.h"
#include "display_sel.h"
#include "display_pad.h"
#include "display_pattern.h"


/* Pattern List */
enum {
  PATTERN_LIST_ID=0,
  PATTERN_LIST_TYPE,
  PATTERN_LIST_SOURCE,
  PATTERN_LIST_FRAME,
  PATTERN_LIST_BACKGROUND,
  PATTERN_LIST_WINDOW_STR,
  PATTERN_LIST_OBJECT,
  PATTERN_LIST_NCOLS
};

static int display_pattern_sock = -1;

static GtkWidget *display_pattern_window = NULL;

/* Pattern list */
static GtkTreeView *display_pattern_treeview = NULL;
static GtkTreeSelection *display_pattern_selection = NULL;
static GtkTreeModel *display_pattern_model = NULL;

/* Pattern buttons */
static GtkWidget *display_pattern_remove = NULL;
static GtkWidget *display_pattern_new = NULL;
static GtkWidget *display_pattern_copy = NULL;
static GtkWidget *display_pattern_button_match = NULL;

static pattern_t *display_pattern_selected = NULL;
static int display_pattern_editable = 1;


static void display_pattern_set_sensitive(void)
{
  int connected = (display_pattern_sock > 0);
  int selected =  connected && (display_pattern_selected != NULL);

  gtk_widget_set_sensitive(GTK_WIDGET(display_pattern_treeview), connected);
  gtk_widget_set_sensitive(display_pattern_remove, selected && display_pattern_editable);
  gtk_widget_set_sensitive(display_pattern_new, connected & display_pattern_editable);
  gtk_widget_set_sensitive(display_pattern_button_match, selected && display_pattern_editable);
  gtk_widget_set_sensitive(display_pattern_copy, selected);
}


void display_pattern_set_selection(frame_geometry_t *g)
{
  if ( display_pattern_window == NULL )
    return;

  editor_set_selection(g);
}


void display_pattern_set_frame(frame_hdr_t *frame)
{
  editor_set_frame(frame);
}


void display_pattern_remove_frame(frame_hdr_t *frame)
{
  editor_remove_frame(frame);
}


void display_pattern_set_ctl(int sock)
{
  display_pattern_set_frame(display_desktop_get_current_frame());

  /* Request for all active patterns */
  display_pattern_sock = sock;
  frame_ctl_reap(sock);

  if ( display_pattern_window != NULL )
    display_pattern_set_sensitive();
}


static void display_pattern_draw_selection(int state)
{
  if ( display_pattern_selected != NULL ) {
    frame_geometry_t gg = display_pattern_selected->window;

    if ( display_pattern_selected->frame != NULL ) {
      gg.x += display_pattern_selected->frame->g0.x;
      gg.y += display_pattern_selected->frame->g0.y;
    }

    if ( state )
      display_sel_draw(DISPLAY_SEL_COLOR_RED, &gg);
    else
      display_sel_undraw(display_current, &gg);
  }
}

void display_pattern_show_selection(void)
{
  if ( display_pattern_window == NULL )
    return;
  display_pattern_draw_selection(1);
}


static void display_pattern_hide_selection(void)
{
  display_pattern_draw_selection(0);
}


static void display_pattern_unselect_guts(void)
{
  editor_show(NULL);
  viewer_show(NULL);

  display_pattern_hide_selection();
  display_selection_show(display_current);
  display_pattern_selected = NULL;

  display_pattern_set_sensitive();
}


static gboolean display_pattern_select(GtkTreeSelection *selection, GtkTreeModel *model,
				       GtkTreePath *path, gboolean path_currently_selected,
				       void *data)
{
  int path_newly_selected = path_currently_selected ? 0:1;
  gboolean ret = TRUE;
  GtkTreeIter iter;

  display_pattern_unselect_guts();

  if ( path_newly_selected ) {
    ret = gtk_tree_model_get_iter(display_pattern_model, &iter, path);
    if ( ret ) {
      gtk_tree_model_get(display_pattern_model, &iter,
			 PATTERN_LIST_OBJECT, &display_pattern_selected,
			 -1);
    }

    display_pattern_show_selection();
    display_pattern_set_sensitive();

    editor_show(display_pattern_selected);
    if ( (display_pattern_selected != NULL) &&
	 (display_pattern_selected->type == PATTERN_TYPE_IMAGE) )
      viewer_show(display_pattern_selected->source);
    else
      viewer_show(NULL);
  }

  return ret;
}


static gboolean display_pattern_clear_iter_(GtkTreeModel *model,
					    GtkTreePath *path, GtkTreeIter *piter,
					    void *arg)
{
  pattern_t *pattern0;

  /* Clear pattern selection */
  if ( gtk_tree_selection_iter_is_selected(display_pattern_selection, piter) ) {
    display_pattern_unselect_guts();
    gtk_tree_selection_unselect_iter(display_pattern_selection, piter);
  }

  /* Get pattern object */
  gtk_tree_model_get(display_pattern_model, piter,
		     PATTERN_LIST_OBJECT, &pattern0,
		     -1);
  gtk_list_store_set(GTK_LIST_STORE(display_pattern_model), piter,
		     PATTERN_LIST_OBJECT, NULL,
		     -1);

  /* Free pattern object if any */
  pattern_free(pattern0);

  return FALSE;
}


static void display_pattern_clear_iter(GtkTreeIter *piter)
{
  display_pattern_clear_iter_(display_pattern_model, NULL, piter, NULL);
}


void display_pattern_clear(void)
{
  if ( display_pattern_window == NULL )
    return;

  debug(DEBUG_PATTERN, "PATTERN CLEAR\n");

  display_pattern_unselect_guts();
  gtk_tree_model_foreach(display_pattern_model,
			 (GtkTreeModelForeachFunc) display_pattern_clear_iter_, NULL);
  gtk_list_store_clear(GTK_LIST_STORE(display_pattern_model));
}


static gboolean display_pattern_get_iter(char *id, GtkTreeIter *piter)
{
  int good;

  good = gtk_tree_model_get_iter_first(display_pattern_model, piter);
  while ( good ) {
    int found = 0;
    char *id2;

    gtk_tree_model_get(display_pattern_model, piter, PATTERN_LIST_ID, &id2, -1);
    if ( id2 != NULL ) {
      found = (strcmp(id, id2) == 0);
      g_free(id2);
    }

    if ( found )
      return TRUE;

    good = gtk_tree_model_iter_next(display_pattern_model, piter);
  }

  return FALSE;
}


static pattern_t *display_pattern_find(char *id, GtkTreeIter *piter)
{
  if ( display_pattern_get_iter(id, piter) ) {
    pattern_t *pattern;
    gtk_tree_model_get(display_pattern_model, piter, PATTERN_LIST_OBJECT, &pattern, -1);
    return pattern;
  }

  return NULL;
}


static void display_pattern_match_iter(GtkTreeIter *piter, int state)
{
  if ( state ) {
    gtk_list_store_set(GTK_LIST_STORE(display_pattern_model), piter,
		       PATTERN_LIST_BACKGROUND, "yellow",
		       -1);
  }
  else {
    gtk_list_store_set(GTK_LIST_STORE(display_pattern_model), piter,
		       PATTERN_LIST_BACKGROUND, NULL,
		       -1);
  }
}


void display_pattern_match(display_t *d, char *id, int state)
{
  pattern_t *pattern;
  GtkTreeIter iter;

  if ( display_pattern_sock < 0 )
    return;
  if ( display_pattern_window == NULL )
    return;

  debug(DEBUG_PATTERN, "PATTERN MATCH id=%s state=%d\n", id, state);

  pattern = display_pattern_find(id, &iter);
  if ( pattern == NULL ) {
    error("TVU client matched unknown pattern %s\n", id);
    return;
  }

  /* Update pattern match status */
  pattern->state = state;

  /* Show match status in editor box */
  if ( pattern == display_pattern_selected )
    editor_show_match(state);

  /* Show match status in pattern list */
  display_pattern_match_iter(&iter, state);
}


static void display_pattern_remove_iter(GtkTreeIter *piter)
{
  /* Clear pattern selection */
  if ( gtk_tree_selection_iter_is_selected(display_pattern_selection, piter) ) {
    display_pattern_unselect_guts();
    gtk_tree_selection_unselect_iter(display_pattern_selection, piter);
  }

  /* Clear pattern object */
  display_pattern_clear_iter(piter);

  /* Remove pattern from list */
  gtk_list_store_remove(GTK_LIST_STORE(display_pattern_model), piter);
}


static pattern_t *display_pattern_update_iter(pattern_t *pattern, GtkTreeIter *piter)
{
  gboolean selected = FALSE;
  pattern_t *pattern0 = NULL;
  GtkTreeIter iter;
  char *s_type;
  char s_window[20];
  char *source;

  /* Append new pattern to list if none exists */
  if ( piter == NULL ) {
    piter = &iter;
    gtk_list_store_append(GTK_LIST_STORE(display_pattern_model), piter);
    pattern0 = pattern_clone(pattern);
    gtk_list_store_set(GTK_LIST_STORE(display_pattern_model), piter,
		       PATTERN_LIST_OBJECT, pattern0,
		       -1);
  }
  else {
    /* Clear pattern selection if it needs to be updated */
    selected = gtk_tree_selection_iter_is_selected(display_pattern_selection, piter);
    if ( selected )
      display_pattern_hide_selection();

    /* Update pattern object */
    gtk_tree_model_get(display_pattern_model, piter,
		       PATTERN_LIST_OBJECT, &pattern0,
		       -1);
    pattern_copy(pattern0, pattern);
  }

  /* Set pattern content */
  snprintf(s_window, sizeof(s_window), "%ux%u%+d%+d",
	   pattern->window.width, pattern->window.height,
	   pattern->window.x, pattern->window.y);

  switch ( pattern->type ) {
  case PATTERN_TYPE_IMAGE:
    s_type = "Image";
    source = strfilename(pattern->source);
    break;
  case PATTERN_TYPE_TEXT:
    s_type = "Text";
    source = pattern->source;
    break;
  default:
    s_type = "???";
    source = "";
    break;
  }

  gtk_list_store_set(GTK_LIST_STORE(display_pattern_model), piter,
		     PATTERN_LIST_ID, pattern->id,
		     PATTERN_LIST_WINDOW_STR, s_window,
		     PATTERN_LIST_TYPE, s_type,
		     PATTERN_LIST_SOURCE, source,
		     -1);

  if ( pattern->frame != NULL ) {
    gtk_list_store_set(GTK_LIST_STORE(display_pattern_model), piter,
		       PATTERN_LIST_FRAME, pattern->frame->id,
		       -1);
  }

  /* Redraw pattern selection if needed */
  if ( selected ) {
    display_pattern_show_selection();
    display_selection_show(display_current);
    editor_show(pattern0);
  }

  return pattern0;
}


void display_pattern_update(char *id, frame_hdr_t *frame, char *source, frame_geometry_t *g, unsigned int mode, int type,
			    fuzz_t fuzz, unsigned char loss[2])
{
  GtkTreeIter iter;
  GtkTreeIter *piter = NULL;

  if ( display_pattern_sock < 0 )
    return;
  if ( display_pattern_window == NULL )
    return;

  debug(DEBUG_PATTERN, "PATTERN UPDATE id=%s frame=%s window=%s source='%s' mode=%02X\n",
	id, frame ? frame->id : "(null)", frame_geometry_str(g), source, mode);

  if ( display_pattern_find(id, &iter) != NULL )
    piter = &iter;

  if ( (source != NULL) && (source[0] != '\0') ) {
    pattern_t *pattern = pattern_alloc(id, frame, g, mode);
    char *err = NULL;

    if ( pattern == NULL ) {
      error("Pattern %s: Allocation failed\n", id);
      return;
    }

    if ( pattern_set_source(pattern, type, source, &err) ) {
      if ( err != NULL ) {
	error("Pattern %s: %s\n", id, err);
	free(err);
      }
      else {
	error("Pattern %s: Unable to set source %s\n", id, source);
      }
    }
    else {
      switch (type) {
      case PATTERN_TYPE_IMAGE:
	      pattern->d.image.fuzz = fuzz;
	      pattern->d.image.badpixels_rate = loss[0];
	      pattern->d.image.potential_rate = loss[1];
	      break;
      default:
	      break;
      }
      display_pattern_update_iter(pattern, piter);
    }

    pattern_free(pattern);
  }
  else {
    if ( piter != NULL )
      display_pattern_remove_iter(piter);
    else
      error("TVU client attempted to remove unknown pattern %s\n", id);
  }
}


static void display_pattern_copy_iter(GtkTreeIter *piter, char **pbuf)
{
  pattern_t *pattern;
  int size;
  int offset;
  char *buf;
  int len;

  gtk_tree_model_get(display_pattern_model, piter,
		     PATTERN_LIST_OBJECT, &pattern,
		     -1);

  size = strlen(pattern->id) + strlen(pattern->source) + 256;

  /* Grow buffer to fit new data */
  offset = (*pbuf != NULL) ? strlen(*pbuf) : 0;
  *pbuf = realloc(*pbuf, offset + size);

  buf = (*pbuf) + offset;
  len = snprintf(buf, size, "match add ");
  len += pattern_str(pattern, buf+len, size-len);
  len += snprintf(buf+len, size-len, "\n");
}


static void display_pattern_copy_clicked(GtkWidget *widget, gint all)
{
  char *text = NULL;
  GtkTreeIter iter;

  if ( all ) {
    int good;

    good = gtk_tree_model_get_iter_first(display_pattern_model, &iter);
    while ( good ) {
      display_pattern_copy_iter(&iter, &text);
      good = gtk_tree_model_iter_next(display_pattern_model, &iter);
    }
  }
  else {
    if ( gtk_tree_selection_get_selected(display_pattern_selection, NULL, &iter) ) {
      display_pattern_copy_iter(&iter, &text);
    }
  }

  if ( text != NULL ) {
    GtkClipboard *clipboard = gtk_clipboard_get(GDK_SELECTION_PRIMARY);
    gtk_clipboard_set_text(clipboard,  text, strlen(text));
  }
 }


static void display_pattern_remove_clicked(void)
{
  char *id;
  GtkTreeIter iter;

  if ( display_pattern_sock < 0 )
    return;

  if ( display_pattern_selected == NULL )
    return;
  id = display_pattern_selected->id;

  display_pattern_unselect_guts();

  /* Delete pattern from TVU client */
  if ( frame_ctl_pattern_remove(display_pattern_sock, id) < 0 ) {
    error("Failed to delete pattern %s in TVU client\n", id);
  }

  /* Delete pattern from list */
  if ( display_pattern_get_iter(id, &iter) )
    display_pattern_remove_iter(&iter);
}


static int display_pattern_iter_compare(GtkTreeIter *piter1, GtkTreeIter *piter2)
{
  GtkTreePath *path1 = gtk_tree_model_get_path(display_pattern_model, piter1);
  GtkTreePath *path2 = gtk_tree_model_get_path(display_pattern_model, piter2);
  int ret = gtk_tree_path_compare(path1, path2);
  gtk_tree_path_free(path1);
  gtk_tree_path_free(path2);
  return ret;
}


static int display_pattern_edit_updated(pattern_t *pattern)
{
  GtkTreeIter *piter = NULL;
  GtkTreeIter iter;

  /* Nothing to update if nothing was modified */
  if ( ! pattern_diff(pattern, display_pattern_selected) )
    return 1;

  if ( display_pattern_selected != NULL ) {
    char *id = display_pattern_selected->id;

    if ( display_pattern_get_iter(id, &iter) )
      piter = &iter;

    /* Process id modification */
    if ( strcmp(pattern->id, id) != 0 ) {
      pattern_t *other;
      GtkTreeIter other_iter;

      /* Check for conflict if id was modified */
      other = display_pattern_find(pattern->id, &other_iter);
      if ( (other != NULL) &&
	   ((piter == NULL) || (display_pattern_iter_compare(piter, &other_iter) != 0)) ) {
        eprintf("Pattern id '%s' already exists", pattern->id);
        return 0;
      }

      /* Delete old pattern from TVU client */
      // TODO: ignore if pattern not yet created
      if ( frame_ctl_pattern_remove(display_pattern_sock, id) < 0 ) {
        error("Failed to delete pattern %s in TVU client\n", id);
      }
    }
  }

  /* Update TVU client pattern */
  if ( frame_ctl_pattern(display_pattern_sock, pattern) < 0 ) {
    error("Failed to update pattern %s in TVU client\n", pattern->id);
  }

  /* Update pattern row */
  /* NOTICE: The following call may destroy variable 'pattern',
     so do not use it after ! */
  display_pattern_update_iter(pattern, piter);

  return 1;
}


static int display_pattern_edit_event(editor_event_t event, pattern_t *pattern)
{
  gboolean enable = TRUE;
  int ret = 0;

  switch ( event ) {
  case EDITOR_EVENT_MODIFIED:
    if ( pattern != NULL )
      enable = FALSE;
    break;
  case EDITOR_EVENT_UPDATE:
    ret = display_pattern_edit_updated(pattern);
    break;
  default:
    break;
  }

  gtk_widget_set_sensitive(GTK_WIDGET(display_pattern_new), enable);
  gtk_widget_set_sensitive(GTK_WIDGET(display_pattern_treeview), enable);

  return ret;
}


static void display_pattern_new_clicked(void)
{
  char new_id[20];
  int n;
  pattern_t *pattern;
  GtkTreeIter iter;

  if ( display_pattern_sock < 0 )
    return;

  /* Choose an non-existing pattern id */
  n = 0;
  do {
    n++;
    snprintf(new_id, sizeof(new_id), "Pattern%d", n);
  } while ( display_pattern_get_iter(new_id, &iter) );

  /* Alloc new pattern descriptor */
  pattern = pattern_alloc(new_id, display_desktop_get_current_frame(), NULL, 0);

  /* Enable APPEAR mode by default */
  pattern->mode |= PATTERN_MODE_APPEAR;

  /* Append a new pattern item to list */
  gtk_list_store_append(GTK_LIST_STORE(display_pattern_model), &iter);
  gtk_list_store_set(GTK_LIST_STORE(display_pattern_model), &iter,
		     PATTERN_LIST_ID, new_id,
		     PATTERN_LIST_OBJECT, pattern,
		     -1);

  if ( pattern->frame != NULL ) {
    gtk_list_store_set(GTK_LIST_STORE(display_pattern_model), &iter,
		       PATTERN_LIST_FRAME, pattern->frame->id,
		       -1);
  }

  /* Select new item */
  gtk_tree_selection_select_iter(display_pattern_selection, &iter);
}


static void display_pattern_match_clicked(void)
{
  char *cmd;
  int size;

  if ( display_pattern_sock < 0 )
    return;
  if ( display_pattern_selected == NULL )
    return;

  display_pattern_selected->state = 0;

  size = strlen(display_pattern_selected->id) + 16;
  cmd = malloc(size);
  snprintf(cmd, size, "match reset %s", display_pattern_selected->id);

  frame_ctl_command(display_pattern_sock, cmd);

  free(cmd);
}


int display_pattern_init(void)
{
  GtkWidget *widget;
  GtkCellRenderer *renderer;

  display_pattern_window = create_pattern_window();
  set_window_icon(GTK_WINDOW(display_pattern_window));
  gtk_signal_connect(GTK_OBJECT(display_pattern_window), "delete_event",
		     GTK_SIGNAL_FUNC(gtk_widget_hide_on_delete), NULL);

  /* Pattern list */
  display_pattern_treeview = GTK_TREE_VIEW(lookup_widget(display_pattern_window, "pattern_treeview"));

  display_pattern_model = GTK_TREE_MODEL(gtk_list_store_new(PATTERN_LIST_NCOLS,
							    /* PATTERN_LIST_ID         */ G_TYPE_STRING,
							    /* PATTERN_LIST_TYPE       */ G_TYPE_STRING,
							    /* PATTERN_LIST_SOURCE     */ G_TYPE_STRING,
							    /* PATTERN_LIST_FRAME      */ G_TYPE_STRING,
							    /* PATTERN_LIST_BACKGROUND */ G_TYPE_STRING,
							    /* PATTERN_LIST_WINDOW_STR */ G_TYPE_STRING,
							    /* PATTERN_LIST_OBJECT     */ G_TYPE_POINTER));
  gtk_tree_view_set_model(display_pattern_treeview, display_pattern_model);

  /* List selection */
  display_pattern_selection = gtk_tree_view_get_selection(display_pattern_treeview);
  gtk_tree_selection_set_mode(display_pattern_selection, GTK_SELECTION_SINGLE);
  gtk_tree_selection_set_select_function(display_pattern_selection,
					 (GtkTreeSelectionFunc) display_pattern_select, NULL,
					 NULL);

  /* Column #0: Id */
  renderer = gtk_cell_renderer_text_new();
  gtk_tree_view_insert_column_with_attributes(display_pattern_treeview, -1, "Id", renderer,
					      "text", PATTERN_LIST_ID,
					      "background", PATTERN_LIST_BACKGROUND,
					      NULL);

  /* Column #1: Frame */
  renderer = gtk_cell_renderer_text_new();
  gtk_tree_view_insert_column_with_attributes(display_pattern_treeview, -1, "Frame", renderer,
					      "text", PATTERN_LIST_FRAME,
					      "background", PATTERN_LIST_BACKGROUND,
					      NULL);

  /* Column #2: Match area */
  renderer = gtk_cell_renderer_text_new();
  gtk_tree_view_insert_column_with_attributes(display_pattern_treeview, -1, "Match Area", renderer,
					      "text", PATTERN_LIST_WINDOW_STR,
					      "background", PATTERN_LIST_BACKGROUND,
					      NULL);

  /* Column #3: Type */
  renderer = gtk_cell_renderer_text_new();
  gtk_object_set(GTK_OBJECT(renderer), "foreground", "#777", NULL);
  gtk_tree_view_insert_column_with_attributes(display_pattern_treeview, -1, "Type", renderer,
					      "text", PATTERN_LIST_TYPE,
					      "background", PATTERN_LIST_BACKGROUND,
					      NULL);

  /* Column #4: Source (image file or regex) */
  renderer = gtk_cell_renderer_text_new();
  gtk_tree_view_insert_column_with_attributes(display_pattern_treeview, -1, "Source", renderer,
					      "text", PATTERN_LIST_SOURCE,
					      "background", PATTERN_LIST_BACKGROUND,
					      NULL);

  /* Pattern management buttons */
  display_pattern_remove = lookup_widget(display_pattern_window, "pattern_button_remove");
  gtk_signal_connect_object(GTK_OBJECT(display_pattern_remove), "clicked",
                            GTK_SIGNAL_FUNC(display_pattern_remove_clicked), NULL);

  display_pattern_new = lookup_widget(display_pattern_window, "pattern_button_new");
  gtk_signal_connect_object(GTK_OBJECT(display_pattern_new), "clicked",
                            GTK_SIGNAL_FUNC(display_pattern_new_clicked), NULL);

  display_pattern_button_match = lookup_widget(display_pattern_window, "pattern_button_match");
  gtk_signal_connect_object(GTK_OBJECT(display_pattern_button_match), "clicked",
                            GTK_SIGNAL_FUNC(display_pattern_match_clicked), NULL);

  /* Clipboard copy buttons */
  display_pattern_copy = lookup_widget(display_pattern_window, "pattern_button_copy");
  gtk_signal_connect(GTK_OBJECT(display_pattern_copy), "clicked",
                     GTK_SIGNAL_FUNC(display_pattern_copy_clicked), GINT_TO_POINTER(0));

  widget = lookup_widget(display_pattern_window, "pattern_button_copy_all");
  gtk_signal_connect(GTK_OBJECT(widget), "clicked",
                     GTK_SIGNAL_FUNC(display_pattern_copy_clicked), GINT_TO_POINTER(1));

  /* Set initial button sensitivity */
  display_pattern_set_sensitive();

  /* Setup pattern editor */
  editor_init(GTK_WINDOW(display_pattern_window), display_pattern_edit_event);

  /* Setup image previewer */
  viewer_init(GTK_WINDOW(display_pattern_window));

  return 0;
}


void display_pattern_done(void)
{
  viewer_done();
  editor_done();

  if ( display_pattern_window != NULL )
    gtk_widget_destroy(display_pattern_window);
  display_pattern_window = NULL;

  display_pattern_sock = -1;
}


void display_pattern_popup(void)
{
  if ( display_pattern_window != NULL ) {
    gtk_widget_show(display_pattern_window);
    gtk_window_present(GTK_WINDOW(display_pattern_window));
  }
}
