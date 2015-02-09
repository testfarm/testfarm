/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* Display Tool - desktops management                                       */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 15-NOV-2007                                                    */
/****************************************************************************/

/*
 * $Revision: 1153 $
 * $Date: 2010-06-06 11:36:58 +0200 (dim., 06 juin 2010) $
 */

#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <dirent.h>
#include <gtk/gtk.h>

#include "support.h"

#include "utils.h"
#include "xpm.h"
#include "frame_ctl.h"
#include "display_screenshot.h"
#include "display_pattern.h"
#include "display_pad.h"
#include "display_desktop.h"


/* Desktop tree */
enum {
  DESKTOP_TREE_PIXBUF,
  DESKTOP_TREE_ID,
  DESKTOP_TREE_GEOMETRY,
  DESKTOP_TREE_STYLE,
  DESKTOP_TREE_SCALE,
  DESKTOP_TREE_DISPLAY,
  DESKTOP_TREE_FRAME,
  DESKTOP_TREE_NCOLS
};

static GtkTreeView *display_desktop_treeview = NULL;
static GtkTreeSelection *display_desktop_selection = NULL;
static GtkTreeModel *display_desktop_model = NULL;

static GtkLabel *display_label_title = NULL;
static guint display_timeout = 0;

display_t *display_current = NULL;
static GtkTreeIter display_current_iter;

static frame_hdr_t *display_current_frame = NULL;
static GtkTreeIter display_current_frame_iter;


color_rgb_t display_desktop_color = DISPLAY_DESKTOP_COLOR;


static int display_desktop_find_display_iter(display_t *d, GtkTreeIter *piter)
{
  int good;

  good = gtk_tree_model_get_iter_first(display_desktop_model, piter);
  while ( good ) {
    display_t *d2;

    gtk_tree_model_get(display_desktop_model, piter,
		       DESKTOP_TREE_DISPLAY, &d2,
		       -1);

    if ( d2 == d ) {
      return TRUE;
    }

    good = gtk_tree_model_iter_next(display_desktop_model, piter);
  }

  return FALSE;
}


static display_t *display_desktop_find_display_by_name(char *name)
{
  GtkTreeIter iter;
  int good;

  good = gtk_tree_model_get_iter_first(display_desktop_model, &iter);
  while ( good ) {
    display_t *d;

    gtk_tree_model_get(display_desktop_model, &iter,
		       DESKTOP_TREE_DISPLAY, &d,
		       -1);

    if ( strcmp(d->desktop.name, name) == 0 ) {
      return d;
    }

    good = gtk_tree_model_iter_next(display_desktop_model, &iter);
  }

  return NULL;
}


static frame_hdr_t *display_desktop_find_frame_children(int shmid, GtkTreeIter *iter, GtkTreeIter *parent)
{
  frame_hdr_t *frame = NULL;

  if ( gtk_tree_model_iter_children(display_desktop_model, iter, parent) ) {
    do {
      gtk_tree_model_get(display_desktop_model, iter,
			 DESKTOP_TREE_FRAME, &frame,
			 -1);

      if ( (frame == NULL) || (frame->shmid != shmid) ) {
	GtkTreeIter child;
	frame = display_desktop_find_frame_children(shmid, &child, iter);
	if ( frame != NULL )
	  *iter = child;
      }
    } while ( (frame == NULL) && gtk_tree_model_iter_next(display_desktop_model, iter) );
  }

  return frame;
}


static frame_hdr_t *display_desktop_find_frame(int shmid, GtkTreeIter *iter)
{
  if ( display_current == NULL )
    return NULL;

  if ( display_current->desktop.root.shmid == shmid ) {
    *iter = display_current_iter;
    return &(display_current->desktop.root);
  }

  return display_desktop_find_frame_children(shmid, iter, &display_current_iter);
}


static int display_desktop_find_layer(display_desktop_t *desktop, frame_hdr_t *frame)
{
  int i;

  for (i = 0; i < desktop->nlayers; i++) {
    if ( desktop->layers[i] == frame )
      return i;
  }

  return -1;
}


static void display_desktop_map_layers(display_desktop_t *desktop)
{
  int size = desktop->root.g0.width * desktop->root.g0.height;

  debug(DEBUG_FRAME, "LAYER MAP %s\n", desktop->name);

  /* Clear layout map */
  memset(desktop->layout, 0, size);

  if ( display_current_frame != NULL ) {
    unsigned int rowstride = desktop->root.g0.width;
    GtkTreeIter iter, child;
    GList *list = NULL;
    GList *l;

    /* Collect layers from deepest to root */
    iter = display_current_frame_iter;
    do {
      frame_hdr_t *frame;

      gtk_tree_model_get(display_desktop_model, &iter,
			 DESKTOP_TREE_FRAME, &frame,
			 -1);

      if ( frame != &(desktop->root) ) {
	list = g_list_prepend(list, frame);
      }

      child = iter;
    } while ( gtk_tree_model_iter_parent(display_desktop_model, &iter, &child) );

    /* Set layer indexes in layout table */
    l = list;
    while ( l != NULL ) {
      frame_hdr_t *frame = l->data;
      int n = display_desktop_find_layer(desktop, frame);

      debug(DEBUG_FRAME, "LAYER MAP frame %d id='%s' %s\n", n, frame->id, frame_geometry_str(&(frame->g0)));

      if ( n > 0 ) {
	unsigned char *py = desktop->layout + (frame->g0.y * rowstride) + frame->g0.x;
	int xmax = frame->g0.width - 1;
	int ymax = frame->g0.height - 1;
	int x, y;

	for (y = 0; y < frame->g0.height; y++) {
	  unsigned char *px = py;

	  for (x = 0; x < frame->g0.width; x++) {
	    unsigned char nn = n;

	    if ( (frame == display_current_frame) &&
		 ((x == 0) || (x == xmax) ||
		  (y == 0) || (y == ymax)) ) {
	      nn |= 0x80;
	    }

	    *(px++) = nn;
	  }

	  py += rowstride;
	}
      }

      l = g_list_next(l);
    }

    g_list_free(list);
  }
}


static int display_desktop_add_layer(display_desktop_t *desktop, frame_hdr_t *frame)
{
  int n = display_desktop_find_layer(desktop, frame);

  if ( n < 0 ) {
    n = desktop->nlayers;
    desktop->nlayers++;
    desktop->layers = (frame_hdr_t **) realloc(desktop->layers, desktop->nlayers * sizeof(frame_hdr_t *));
  }

  debug(DEBUG_FRAME, "LAYER ADD %d id='%s' shmid=%d\n", n, frame->id, frame->shmid);
  desktop->layers[n] = frame;

  return n;
}


static void display_desktop_remove_layer(display_desktop_t *desktop, frame_hdr_t *frame)
{
  int n = display_desktop_find_layer(desktop, frame);
  int i;

  if ( n >= 0 ) {
    for (i = (n+1); i < desktop->nlayers; i++) {
      desktop->layers[i-1] = desktop->layers[i];
    }
    desktop->nlayers--;
  }
}


static void display_desktop_init_layout(display_desktop_t *desktop)
{
  frame_hdr_t *root = &(desktop->root);
  int size = root->g0.width * root->g0.height;

  desktop->nlayers = 1;
  desktop->layers = (frame_hdr_t **) malloc(sizeof(frame_hdr_t *));
  desktop->layers[0] = root;

  desktop->layout = malloc(size);
  memset(desktop->layout, 0, size);
}


int display_desktop_init_root(display_desktop_t *desktop, int shmid)
{
  frame_hdr_t *root = &(desktop->root);
  int ret = 0;

  /* Free existing buffers */
  if ( desktop->layers != NULL ) {
    free(desktop->layers);
    desktop->nlayers = 0;
    desktop->layers = NULL;
  }

  if ( desktop->layout != NULL ) {
    free(desktop->layout);
    desktop->layout = NULL;
  }

  if ( root->fb != NULL ) {
    frame_buf_free(root->fb);
    root->fb = NULL;
  }

  /* Attach frame buffer */
  if ( shmid > 0 ) {
    root->fb = frame_buf_map(shmid, 1);
    if ( root->fb == NULL ) {
      shmid = -1;
      ret = -1;
    }
  }

  root->shmid = shmid;
  root->g0 = frame_geometry_null;

  if ( root->fb != NULL ) {
    root->g0.width = root->fb->rgb.width;
    root->g0.height = root->fb->rgb.height;
    display_desktop_init_layout(desktop);
  }

  /* Setup screenshot management */
  display_screenshot_set_frame(root);

  return ret;
}


static void display_desktop_highlight_frame(frame_hdr_t *frame)
{
  if ( display_current == NULL )
    return;
  if ( frame == &(display_current->desktop.root) )
    return;

  display_pad_add(&(display_current->pad), &(frame->g0), display_desktop_color);
}


void display_desktop_add_frame(char *id, int shmid, frame_geometry_t *g0, int parent_shmid)
{
  GtkTreeIter parent_iter, iter;
  frame_hdr_t *parent_frame, *frame;

  if ( display_current == NULL )
    return;

  debug(DEBUG_FRAME, "FRAME ADD id='%s' shmid=%d g0=%s parent=%d\n", id, shmid, frame_geometry_str(g0), parent_shmid);

  parent_frame = display_desktop_find_frame(parent_shmid, &parent_iter);
  if ( parent_frame == NULL ) {
    error("FRAME ADD: Parent frame shmid=%d not found in current desktop %s\n", parent_shmid, display_current->desktop.name);
    return;
  }

  /* (Re)Alloc frame descriptor */
  frame = display_desktop_find_frame(shmid, &iter);

  if ( frame != NULL ) {
    free(frame->id);
    frame_buf_free(frame->fb);
  }
  else {
    frame = (frame_hdr_t *) malloc(sizeof(frame_hdr_t));
    gtk_tree_store_append(GTK_TREE_STORE(display_desktop_model), &iter, &parent_iter);
  }

  /* Set frame descriptor content */
  frame->id = strdup(id);
  frame->shmid = shmid;
  frame->fb = frame_buf_map(shmid, 1);
  frame->g0 = *g0;

  /* Add frame in layers table */
  display_desktop_add_layer(&(display_current->desktop), frame);

  /* Insert new frame descriptor in the desktop tree */
  gtk_tree_store_set(GTK_TREE_STORE(display_desktop_model), &iter,
		     DESKTOP_TREE_PIXBUF, pixbuf_frame,
		     DESKTOP_TREE_ID, frame->id,
		     DESKTOP_TREE_GEOMETRY, frame_geometry_str(&(frame->g0)),
		     DESKTOP_TREE_SCALE, 0.8,
		     DESKTOP_TREE_DISPLAY, display_current,
		     DESKTOP_TREE_FRAME, frame,
		     -1);

  /* Highlight new frame */
  display_desktop_highlight_frame(frame);
}


static void display_desktop_free_frame(display_desktop_t *desktop, frame_hdr_t *frame)
{
  /* Select root window if frame was selected */
  if ( display_current_frame == frame ) {
    display_current_frame = &(desktop->root);
    display_current_frame_iter = display_current_iter;
    gtk_tree_selection_select_iter(display_desktop_selection, &display_current_iter);
  }

  /* Remove frame from layout */
  display_desktop_remove_layer(desktop, frame);

  /* Free frame descriptor */
  free(frame->id);
  frame_buf_free(frame->fb);
  free(frame);
}


static void display_desktop_remove_children(display_desktop_t *desktop, GtkTreeIter *parent)
{
  GtkTreeIter iter;

  if ( gtk_tree_model_iter_children(display_desktop_model, &iter, parent) ) {
    do {
      frame_hdr_t *frame;

      display_desktop_remove_children(desktop, &iter);

      gtk_tree_model_get(display_desktop_model, &iter,
			 DESKTOP_TREE_FRAME, &frame,
			 -1);

      /* Free frame descriptor */
      if ( frame != NULL ) {
	display_desktop_free_frame(&(display_current->desktop), frame);
      }
    } while ( gtk_tree_store_remove(GTK_TREE_STORE(display_desktop_model), &iter) );
  }
}


void display_desktop_remove_frame(int shmid)
{
  display_desktop_t *desktop;
  GtkTreeIter iter;
  frame_hdr_t *frame;

  if ( display_current == NULL )
    return;
  desktop = &(display_current->desktop);

  debug(DEBUG_FRAME, "FRAME REMOVE shmid=%d\n", shmid);

  frame = display_desktop_find_frame(shmid, &iter);
  if ( frame != NULL ) {
    frame_geometry_t g0 = frame->g0;

    display_pattern_remove_frame(display_current_frame);

    display_desktop_remove_children(desktop, &iter);
    gtk_tree_store_remove(GTK_TREE_STORE(display_desktop_model), &iter);
    display_desktop_free_frame(desktop, frame);

    display_desktop_map_layers(desktop);
    display_refresh(display_current, &g0);
    display_selection_show(display_current);
    display_pattern_set_frame(display_current_frame);
    display_screenshot_set_frame(display_current_frame);
  }
  else {
    error("FRAME REMOVE: frame shmid=%d not found in current desktop %s\n", shmid, display_current->desktop.name);
  }
}


frame_hdr_t *display_desktop_get_current_frame(void)
{
  if ( display_current == NULL )
    return NULL;

  return display_current_frame;
}


frame_hdr_t *display_desktop_get_frame(int shmid)
{
  GtkTreeIter iter;

  if ( display_current == NULL )
    return NULL;
  return display_desktop_find_frame(shmid, &iter);
}


void display_desktop_show(display_t *d, display_desktop_state_t state)
{
  display_desktop_t *desktop;
  GtkTreeIter iter;
  GdkPixbuf *pixbuf = pixbuf_blank;
  PangoStyle style = PANGO_STYLE_NORMAL;

  if ( ! display_desktop_find_display_iter(d, &iter) )
    return;
  desktop = &(d->desktop);

  switch ( state ) {
  case DISPLAY_DESKTOP_DISCONNECTED:
    display_desktop_remove_children(desktop, &iter);
    if ( desktop->available )
      pixbuf = pixbuf_connect_no;
    break;
  case DISPLAY_DESKTOP_CONNECTING:
    lprintf(display_label_title, "%s - connection in progress", desktop->name);
    pixbuf = pixbuf_connect_creating;
    break;
  case DISPLAY_DESKTOP_CONNECTED:
    lprintf(display_label_title, "%s - %ux%u", desktop->name, desktop->root.g0.width, desktop->root.g0.height);
    pixbuf = pixbuf_connect_established;
    break;
  }

  style = desktop->available ? PANGO_STYLE_NORMAL : PANGO_STYLE_ITALIC;

  gtk_tree_store_set(GTK_TREE_STORE(display_desktop_model), &iter,
		     DESKTOP_TREE_PIXBUF, pixbuf,
		     DESKTOP_TREE_STYLE, style,
		     DESKTOP_TREE_SCALE, 1.0,
		     -1);
}


void display_desktop_available(display_t *d, int available)
{
  display_desktop_t *desktop = &(d->desktop);

  if ( available == desktop->available )
    return;
  desktop->available = available;

  if ( available == 0 ) {
    display_disconnect(d);
    display_desktop_show(d, DISPLAY_DESKTOP_DISCONNECTED);

    if ( d == display_current )
      lprintf(display_label_title, "%s - unavailable", desktop->name);
  }
  else {
    if ( d == display_current ) {
      if ( display_connect(d) )
        return;
    }
    else {
      display_desktop_show(d, DISPLAY_DESKTOP_DISCONNECTED);
    }
  }
}


static gboolean display_desktop_select(GtkTreeSelection *selection, GtkTreeModel *model,
				       GtkTreePath *path, gboolean path_currently_selected,
				       void *data)
{
  int path_newly_selected = path_currently_selected ? 0:1;
  display_t *d = NULL;
  frame_hdr_t *frame = NULL;
  GtkTreeIter iter;

  if ( path_newly_selected ) {
    if ( gtk_tree_model_get_iter(model, &iter, path) ) {
      gtk_tree_model_get(model, &iter,
			 DESKTOP_TREE_DISPLAY, &d,
			 DESKTOP_TREE_FRAME, &frame,
			 -1);
    }
  }

  /* Manage desktop switch */
  if ( (d != NULL) && (d != display_current) ) {
    display_t *display_previous;

    /* Select new desktop */
    display_previous = display_current;
    display_current = d;
    display_current_iter = iter;

    /* Disconnect previously selected desktop */
    display_disconnect(display_previous);

    debug(DEBUG_FRAME, "SELECT DESKTOP name='%s'\n", d->desktop.name);

    /* Start connection if display is available */
    if ( d->desktop.available ) {
      if ( display_connect(d) )
	display_desktop_available(d, 0);
    }

    /* Show jammed screen if display not available */
    else {
      display_setup(d);
    }
  }

  if ( (frame != NULL) && (frame != display_current_frame) ) {
    display_current_frame = frame;
    display_current_frame_iter = iter;

    debug(DEBUG_FRAME, "SELECT FRAME id='%s'\n", frame->id);

    display_desktop_map_layers(&(display_current->desktop));
    display_refresh(display_current, &(display_current->desktop.root.g0));
    display_selection_show(display_current);
    display_pattern_set_frame(frame);
    display_screenshot_set_frame(frame);

    display_desktop_highlight_frame(frame);
  }

  return TRUE;
}


static void display_desktop_row_activated(GtkTreeView *treeview,
					  GtkTreePath *path,
					  GtkTreeViewColumn *column,
					  void *data)
{
  GtkTreeIter iter;

  if ( gtk_tree_model_get_iter(display_desktop_model, &iter, path) ) {
    frame_hdr_t *frame;

    gtk_tree_model_get(display_desktop_model, &iter,
		       DESKTOP_TREE_FRAME, &frame,
		       -1);

    /* Highlight activated frame */
    if ( frame != NULL ) {
      display_desktop_highlight_frame(frame);
    }
  }
}


static void display_desktop_mklist(void)
{
  char *sockdir;
  DIR *dir;
  display_t *d;
  int changed = 0;
  GtkTreeIter iter;
  int good;

  /* Add and/or check all present desktops */
  sockdir = frame_ctl_sockdir();
  if (sockdir != NULL) {
    if ( (dir = opendir(sockdir)) != NULL ) {
      struct dirent *ent;

      while ( (ent = readdir(dir)) != NULL ) {
        char *name = ent->d_name;

	if ( (name[0] != '\0') && (name[0] != '.') ) {
	  d = display_desktop_find_display_by_name(name);

	  if ( d == NULL ) {
	    GtkTreeIter iter;

	    d = display_alloc(name);

	    gtk_tree_store_append(GTK_TREE_STORE(display_desktop_model), &iter, NULL);
	    gtk_tree_store_set(GTK_TREE_STORE(display_desktop_model), &iter,
			       DESKTOP_TREE_ID, d->desktop.name,
			       DESKTOP_TREE_GEOMETRY, "(root)",
			       DESKTOP_TREE_DISPLAY, d,
			       DESKTOP_TREE_FRAME, &(d->desktop.root),
			       -1);
	  }

	  d->desktop.available |= 2;
	  changed = 1;
	}
      }

      closedir(dir);
    }

    free(sockdir);
  }

  /* Close all unchecked desktops */
  good = gtk_tree_model_get_iter_first(display_desktop_model, &iter);
  while ( good ) {
    display_t *d;
    int available;

    gtk_tree_model_get(display_desktop_model, &iter,
		       DESKTOP_TREE_DISPLAY, &d,
		       -1);

    available = (d->desktop.available & 2) ? 1:0;
    d->desktop.available &= 1;
    display_desktop_available(d, available);

    good = gtk_tree_model_iter_next(display_desktop_model, &iter);
  }

  /* Select first desktop if none is already selected */
  if ( display_current == NULL ) {
    GtkTreeIter iter;

    if ( gtk_tree_model_get_iter_first(display_desktop_model, &iter) ) {
      gtk_tree_selection_select_iter(display_desktop_selection, &iter);
    }
  }
}


static gboolean display_desktop_heartbeat(void)
{
  display_desktop_mklist();
  return TRUE;
}


void display_desktop_init(GtkWindow *window)
{
  GtkTreeViewColumn *column;
  GtkCellRenderer *renderer;

  display_label_title = GTK_LABEL(lookup_widget(GTK_WIDGET(window), "label_title"));
  display_desktop_treeview = GTK_TREE_VIEW(lookup_widget(GTK_WIDGET(window), "desktop_treeview"));

  display_current = NULL;

  display_desktop_model = GTK_TREE_MODEL(gtk_tree_store_new(DESKTOP_TREE_NCOLS,
							    /* DESKTOP_TREE_PIXBUF     */ GDK_TYPE_PIXBUF,
							    /* DESKTOP_TREE_ID         */ G_TYPE_STRING,
							    /* DESKTOP_TREE_GEOMETRY   */ G_TYPE_STRING,
							    /* DESKTOP_TREE_STYLE      */ PANGO_TYPE_STYLE,
							    /* DESKTOP_TREE_SCALE      */ G_TYPE_FLOAT,
							    /* DESKTOP_TREE_DISPLAY    */ G_TYPE_POINTER,
							    /* DESKTOP_TREE_FRAME      */ G_TYPE_POINTER));
  gtk_tree_view_set_model(display_desktop_treeview, display_desktop_model);

  /* List selection */
  display_desktop_selection = gtk_tree_view_get_selection(display_desktop_treeview);
  gtk_tree_selection_set_mode(display_desktop_selection, GTK_SELECTION_BROWSE);
  gtk_tree_selection_set_select_function(display_desktop_selection,
					 (GtkTreeSelectionFunc) display_desktop_select, NULL,
					 NULL);

  /* Setup row activation handler */
  gtk_signal_connect(GTK_OBJECT(display_desktop_treeview), "row-activated",
		     GTK_SIGNAL_FUNC(display_desktop_row_activated), NULL);

  /* Column #0: Status pixbuf and Desktop/Frame id */
  column = gtk_tree_view_column_new();
  gtk_tree_view_column_set_title(column, "Desktop / Frame");
  gtk_tree_view_column_set_expand(column, TRUE);

  renderer = gtk_cell_renderer_pixbuf_new();
  gtk_tree_view_column_pack_start(column, renderer, FALSE);
  gtk_tree_view_column_add_attribute(column, renderer, "pixbuf", DESKTOP_TREE_PIXBUF);

  renderer = gtk_cell_renderer_text_new();
  gtk_tree_view_column_pack_start(column, renderer, TRUE);
  gtk_tree_view_column_add_attribute(column, renderer, "text", DESKTOP_TREE_ID);
  gtk_tree_view_column_add_attribute(column, renderer, "style", DESKTOP_TREE_STYLE);
  gtk_tree_view_column_add_attribute(column, renderer, "scale", DESKTOP_TREE_SCALE);

  gtk_tree_view_append_column(display_desktop_treeview, column);

  /* Column #1: Frame geometry */
  renderer = gtk_cell_renderer_text_new();
  gtk_tree_view_insert_column_with_attributes(display_desktop_treeview, -1, "Geometry", renderer,
					      "text", DESKTOP_TREE_GEOMETRY,
					      "style", DESKTOP_TREE_STYLE,
					      "scale", DESKTOP_TREE_SCALE,
					      NULL);

  /* Collect all available display connections */
  display_desktop_mklist();

  if ( display_timeout == 0 )
    display_timeout = g_timeout_add(1000, (GSourceFunc) display_desktop_heartbeat, NULL);
}


void display_desktop_done(void)
{
  if ( display_timeout > 0 ) {
    g_source_remove(display_timeout);
    display_timeout = 0;
  }

  if ( display_desktop_model != NULL ) {
    gtk_tree_store_clear(GTK_TREE_STORE(display_desktop_model));
  }
}
