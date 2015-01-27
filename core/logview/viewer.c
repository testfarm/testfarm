/****************************************************************************/
/* TestFarm                                                                 */
/* Test Log Viewer - Main window                                            */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 07-DEC-1999                                                    */
/****************************************************************************/

/*
 * $Revision: 262 $
 * $Date: 2006-10-11 16:17:19 +0200 (mer., 11 oct. 2006) $
 */

#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <errno.h>
#include <unistd.h>
#include <fam.h>
#include <gtk/gtk.h>

#include "install.h"
#include "useful.h"
#include "install_icon.h"
#include "list.h"
#include "viewer.h"


#define NAME "testfarm-logivew"
#define TITLE "TestFarm Log Viewer"


/*================================================================*/
/* FAM events                                                     */
/*================================================================*/

static void viewer_fam_close(viewer_t *viewer);


static void viewer_fam_cancel(viewer_t *viewer)
{
  if ( viewer->fc_req_ok )
    FAMCancelMonitor(&(viewer->fc), &(viewer->fc_req));
  viewer->fc_req_ok = 0;
}


static void viewer_fam_monitor(viewer_t *viewer, char *filename)
{
  if ( viewer->fc_tag < 0 )
    return;

  if ( FAMMonitorFile(&(viewer->fc), filename, &(viewer->fc_req), viewer) ) {
    fprintf(stderr, NAME ": WARNING: Failed to setup FAM monitor on file '%s': %s\n", filename, FamErrlist[FAMErrno]);
  }
  else {
    viewer->fc_req_ok = 1;
  }
}


static void viewer_fam_input(viewer_t *viewer, int fd, GdkInputCondition condition)
{
  FAMEvent fe;

  while ( FAMPending(&(viewer->fc)) > 0 ) {
    if ( FAMNextEvent(&(viewer->fc), &fe) < 0 ) {
      fprintf(stderr, NAME ": WARNING: Failed to retrieve FAM event: %s\n", FamErrlist[FAMErrno]);
      fprintf(stderr, NAME ": WARNING: Disabling file update detection.\n");
      viewer_fam_close(viewer);
      return;
    }

    switch ( fe.code ) {
    case FAMDeleted:
    case FAMMoved:
      viewer_clear(viewer);
      break;

    case FAMChanged:
    case FAMCreated:
      viewer_follow(viewer, 0);
      break;

    default:
      break;
    }
  }
}


static void viewer_fam_open(viewer_t *viewer)
{
  viewer->fc_tag = -1;
  viewer->fc_req_ok = 0;

  if ( FAMOpen2(&(viewer->fc), NAME) ) {
    fprintf(stderr, NAME ": WARNING: Failed to open FAM connection. No file updates will be detected.\n");
  }
  else {
    viewer->fc_tag = gdk_input_add(FAMCONNECTION_GETFD(&(viewer->fc)), GDK_INPUT_READ,
				   (GdkInputFunction) viewer_fam_input, (gpointer) viewer);
  }
}


static void viewer_fam_close(viewer_t *viewer)
{
  viewer_fam_cancel(viewer);

  if ( viewer->fc_tag != -1 ) {
    gdk_input_remove(viewer->fc_tag);
    viewer->fc_tag = -1;
    FAMClose(&(viewer->fc));
  }
}


/*================================================================*/
/* File operations                                                */
/*================================================================*/

void viewer_clear(viewer_t *viewer)
{
  list_unload(viewer->list);
  viewer->loaded = 0;
}


static void viewer_setup(viewer_t *viewer, char *filename)
{
  if ( filename != NULL ) {
    if ( viewer->filename != NULL )
      free(viewer->filename);
    viewer->filename = strdup(filename);

    viewer_fam_cancel(viewer);
    viewer_fam_monitor(viewer, filename);
  }

  if ( viewer->title == NULL ) {
    if ( filename == NULL ) {
      gtk_window_set_title(GTK_WINDOW(viewer->window), TITLE);
    }
    else {
      char title[strlen(TITLE)+strlen(filename)+4];
      snprintf(title, sizeof(title), TITLE ": %s", filename);
      gtk_window_set_title(GTK_WINDOW(viewer->window), title);
    }
  }
}


void viewer_load(viewer_t *viewer, char *filename)
{
  viewer_clear(viewer);
  viewer_setup(viewer, filename);

  if ( viewer->filename != NULL ) {
    viewer->loaded = (list_load(viewer->list, viewer->filename) == 0);
  }
}


int viewer_follow(viewer_t *viewer, int moveto)
{
  int ret;

  if ( viewer == NULL )
    return -1;
  if ( viewer->filename == NULL )
    return -1;

  if ( viewer->loaded ) {
    ret = list_follow(viewer->list, moveto);
    if ( ret )
      fprintf(stderr, NAME ": WARNING: Cannot follow file %s\n", viewer->filename);
  }
  else {
    ret = list_load(viewer->list, viewer->filename);
    viewer->loaded = (ret == 0);
  }

  return ret;
}


/*================================================================*/
/*================================================================*/

static void button_periph_clicked(GtkWidget *widget, viewer_t *viewer)
{
  list_filter_periph(viewer->list, GTK_TOGGLE_BUTTON(widget)->active);
}

static void button_tag_clicked(GtkWidget *widget, viewer_t *viewer)
{
  list_filter_tag(viewer->list, GTK_TOGGLE_BUTTON(widget)->active);
}

static void button_case_clicked(GtkWidget *widget, viewer_t *viewer)
{
  list_filter_case(viewer->list, GTK_TOGGLE_BUTTON(widget)->active);
}

static void button_clear_clicked(GtkWidget *widget, viewer_t *viewer)
{
  list_select_clear(viewer->list);
}

static void button_destroyed(void *arg)
{
  if ( GTK_TOGGLE_BUTTON(arg)->active )
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(arg), 0);
}


/*================================================================*/
/*================================================================*/

static void button_follow_clicked(GtkWidget *widget, viewer_t *viewer)
{
  viewer_follow(viewer, 1);
}


/*================================================================*/
/*================================================================*/

static void viewer_destroyed_handler(GtkWidget *widget, viewer_t *viewer)
{
  if ( viewer->destroyed != NULL )
    viewer->destroyed(viewer->destroyed_arg);
}


/*================================================================*/
/* Search history management                                      */
/*================================================================*/

#define SEARCH_HISTORY_FILE "logview-search-history"

static void search_history_clear(viewer_t *viewer)
{
  if ( viewer->search_list != NULL ) {
    g_list_foreach(viewer->search_list, (GFunc) free, NULL);
    g_list_free(viewer->search_list);
  }
  viewer->search_list = NULL;
}


static char *search_history_file(viewer_t *viewer)
{
  if ( viewer->search_history == NULL ) {
    viewer->search_history = strdup(get_user_config(SEARCH_HISTORY_FILE));
  }

  return viewer->search_history;
}


static void search_history_save_item(char *text, FILE *f)
{
  fprintf(f, "%s\n", text);
}


static void search_history_save(viewer_t *viewer)
{
  FILE *f;

  /* Get history file name */
  if ( search_history_file(viewer) == NULL )
    return;

  /* Open history file name for write */
  if ( (f = fopen(viewer->search_history, "w")) == NULL ) {
    fprintf(stderr, NAME ": WARNING: Cannot open (w) search history %s: %s\n", viewer->search_history, strerror(errno));
    return;
  }

  /* Write history list */
  g_list_foreach(viewer->search_list, (GFunc) search_history_save_item, f);

  fclose(f);
}

static void search_history_load(viewer_t *viewer)
{
  FILE *f;
  char *buf = NULL;
  int size = 0;

  /* Get history file name */
  if ( search_history_file(viewer) == NULL )
    return;

  /* Open history file name for read */
  if ( (f = fopen(viewer->search_history, "r")) == NULL ) {
    fprintf(stderr, NAME ": WARNING: Cannot open (r) search history %s: %s\n", viewer->search_history, strerror(errno));
    return;
  }

  /* Clear history list */
  search_history_clear(viewer);

  /* Get history list from file */
  while ( ! feof(f) ) {
    int len;

    len = fgets2(f, &buf, &size);

    if ( len > 0 ) {
      if ( viewer->search_list == NULL )
	gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(viewer->search_combo)->entry), buf);

      viewer->search_list = g_list_append(viewer->search_list, strdup(buf));
    }
  }

  fclose(f);

  /* Free read buffer */
  if ( buf != NULL )
    free(buf);

  /* Update search entry combo */
  gtk_combo_set_popdown_strings(GTK_COMBO(viewer->search_combo), viewer->search_list) ;
}


static void search_history_done(viewer_t *viewer)
{
  /* Free serach history file name */
  if ( viewer->search_history != NULL )
    free(viewer->search_history);
  viewer->search_history = NULL;

  /* Free search history list */
  search_history_clear(viewer);
}


/*================================================================*/
/*================================================================*/

static void search_text(viewer_t *viewer, int from_start)
{
  char *text = (char *) gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(viewer->search_combo)->entry));
  int info_only = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(viewer->search_info));
  int case_sensitive = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(viewer->search_case));
  GList *item;

  if ( *text == '\0' )
    return;

  item = g_list_find_custom(viewer->search_list, text, (GCompareFunc) strcmp);
  if ( item == NULL ) {
    text = strdup(text);
  }
  else {
    text = g_list_nth_data(item, 0);
    viewer->search_list = g_list_remove(viewer->search_list, text);
  }

  viewer->search_list = g_list_prepend(viewer->search_list, text);

  gtk_combo_set_popdown_strings(GTK_COMBO(viewer->search_combo), viewer->search_list) ;
  gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(viewer->search_combo)->entry), text);

  gdk_window_set_cursor(viewer->window->window, viewer->cursor_watch);
  gdk_flush();

  list_search_text(viewer->list, text, from_start, info_only, case_sensitive);

  gdk_window_set_cursor(viewer->window->window, NULL);
}


static void search_entry_validated(GtkWidget *widget, viewer_t *viewer)
{
  search_text(viewer, 0);
}


static void search_button_clicked(GtkWidget *widget, viewer_t *viewer)
{
  search_text(viewer, 1);
}


/*================================================================*/
/*================================================================*/

#define VIEWER_SIZE_WIDTH 768
#define VIEWER_SIZE_HIGHT 400


static GtkWidget *viewer_button(GtkWidget *box, char *label)
{
  GtkWidget *button;

  if ( label[0] == '+' )
    button = gtk_toggle_button_new_with_label(label+1);
  else
    button = gtk_button_new_with_label(label);
  gtk_widget_show(button);
  gtk_box_pack_start(GTK_BOX(box), button, TRUE, TRUE, 0);
  return button;
}


static GtkWidget *viewer_vsep(GtkWidget *box)
{
  GtkWidget *vsep = gtk_vseparator_new();
  gtk_widget_show(vsep);
  gtk_box_pack_start(GTK_BOX(box), vsep, FALSE, FALSE, 5);
  return vsep;
}


viewer_t *viewer_init(char *title)
{
  GtkTooltips *tooltips;
  GtkWidget *main_box;
  GtkWidget *hbox;
  GtkWidget *button;

  /* New viewer instance */
  viewer_t *viewer = (viewer_t *) malloc(sizeof(viewer_t));

  /* The main window */
  viewer->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_widget_set_usize(viewer->window, VIEWER_SIZE_WIDTH, VIEWER_SIZE_HIGHT);
  gtk_signal_connect(GTK_OBJECT(viewer->window), "destroy",
                     GTK_SIGNAL_FUNC(viewer_destroyed_handler), viewer);

  /* The window icon */
  set_window_icon(GTK_WINDOW(viewer->window));

  /* The window title */
  if ( title == NULL ) {
    viewer->title = NULL;
    gtk_window_set_title(GTK_WINDOW(viewer->window), TITLE);
  }
  else {
    viewer->title = (char *) malloc(strlen(TITLE)+strlen(title)+4);
    sprintf(viewer->title, TITLE ": %s", title);
    gtk_window_set_title(GTK_WINDOW(viewer->window), viewer->title);
  }

  /* A vertical box to put the list and the button in */
  main_box = gtk_vbox_new(FALSE, 5);
  gtk_container_set_border_width(GTK_CONTAINER(main_box), 5);
  gtk_container_add(GTK_CONTAINER(viewer->window), main_box);
  gtk_widget_show(main_box);

  /* Some tooltips to make life easier */
  tooltips = gtk_tooltips_new();

  /* The result log list */
  viewer->list = list_init(main_box);
  viewer->loaded = 0;

  /* The button box */
  viewer->button_box = gtk_vbox_new(FALSE, 5);
  gtk_widget_show(viewer->button_box);
  gtk_box_pack_start(GTK_BOX(main_box), viewer->button_box, FALSE, FALSE, 0);

  /* The file buttons */
  hbox = gtk_hbox_new(FALSE, 5);
  gtk_widget_show(hbox);
  gtk_box_pack_start(GTK_BOX(viewer->button_box), hbox, FALSE, FALSE, 0);

  button = viewer_button(hbox, "Follow");
  gtk_tooltips_set_tip(tooltips, button, "Follow the Test Log file as it grows", NULL);
  gtk_signal_connect(GTK_OBJECT(button), "clicked",
                     GTK_SIGNAL_FUNC(button_follow_clicked), viewer);

  button = viewer_button(hbox, "Close");
  gtk_tooltips_set_tip(tooltips, button, "Quit the TestFarm Log Viewer", NULL);
  gtk_signal_connect_object(GTK_OBJECT(button), "clicked",
                            GTK_SIGNAL_FUNC(gtk_widget_destroy), (gpointer) viewer->window);

  viewer_vsep(hbox);

  /* The selection buttons */
  button = viewer_button(hbox, "+Interfaces");
  gtk_tooltips_set_tip(tooltips, button, "Perform filtering on interfaces", NULL);
  gtk_signal_connect(GTK_OBJECT(button), "clicked",
                     GTK_SIGNAL_FUNC(button_periph_clicked), viewer);
  list_destroyed_periph(viewer->list, button_destroyed, button);

  button = viewer_button(hbox, "+Tags");
  gtk_tooltips_set_tip(tooltips, button, "Perform filtering on tags", NULL);
  gtk_signal_connect(GTK_OBJECT(button), "clicked",
                     GTK_SIGNAL_FUNC(button_tag_clicked), viewer);
  list_destroyed_tag(viewer->list, button_destroyed, button);

  button = viewer_button(hbox, "+Test cases");
  gtk_tooltips_set_tip(tooltips, button, "Perform filtering on Test Cases", NULL);
  gtk_signal_connect(GTK_OBJECT(button), "clicked",
                     GTK_SIGNAL_FUNC(button_case_clicked), viewer);
  list_destroyed_case(viewer->list, button_destroyed, button);

  viewer_vsep(hbox);

  button = viewer_button(hbox, "Clear selection");
  gtk_tooltips_set_tip(tooltips, button, "Clear all selected Log lines", NULL);
  gtk_signal_connect(GTK_OBJECT(button), "clicked",
                     GTK_SIGNAL_FUNC(button_clear_clicked), viewer);

  /* The search entry and buttons */
  hbox = gtk_hbox_new(FALSE, 10);
  gtk_widget_show(hbox);
  gtk_box_pack_start(GTK_BOX(viewer->button_box), hbox, FALSE, FALSE, 0);

  viewer->search_combo = gtk_combo_new();
  gtk_widget_show(viewer->search_combo);
  gtk_combo_disable_activate(GTK_COMBO(viewer->search_combo));
  gtk_combo_set_case_sensitive(GTK_COMBO(viewer->search_combo), 1);
  gtk_box_pack_start(GTK_BOX(hbox), viewer->search_combo, TRUE, TRUE, 0);
  gtk_signal_connect(GTK_OBJECT(GTK_COMBO(viewer->search_combo)->entry), "activate",
                     GTK_SIGNAL_FUNC(search_entry_validated), viewer);

  button = gtk_button_new_with_label("Find First");
  gtk_widget_show(button);
  gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 0);
  gtk_signal_connect(GTK_OBJECT(button), "clicked",
                     GTK_SIGNAL_FUNC(search_button_clicked), viewer);

  button = gtk_button_new_with_label("Find Next");
  gtk_widget_show(button);
  gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 0);
  gtk_signal_connect(GTK_OBJECT(button), "clicked",
                     GTK_SIGNAL_FUNC(search_entry_validated), viewer);

  viewer->search_info = gtk_check_button_new_with_label("Information only");
  gtk_widget_show(viewer->search_info);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(viewer->search_info), 1);
  gtk_box_pack_start(GTK_BOX(hbox), viewer->search_info, FALSE, FALSE, 0);

  viewer->search_case = gtk_check_button_new_with_label("Match case");
  gtk_widget_show(viewer->search_case);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(viewer->search_case), 0);
  gtk_box_pack_start(GTK_BOX(hbox), viewer->search_case, FALSE, FALSE, 0);

  viewer->search_history = NULL;
  viewer->search_list = NULL;
  search_history_load(viewer);

  /* Show this beautiful piece of art... */
  gtk_widget_show(viewer->window);

  /* Load result log file and fill the list with it */
  viewer->filew = NULL;
  viewer->filename = NULL;

  /* Clear destroyed signal handling */
  viewer->destroyed = NULL;
  viewer->destroyed_arg = NULL;

  /* The mouse cursors */
  viewer->cursor_watch = gdk_cursor_new(GDK_WATCH);

  /* Open the FAM connection */
  viewer_fam_open(viewer);

  return viewer;
}


void viewer_done(viewer_t *viewer)
{
  if ( viewer == NULL )
    return;

  /* Close FAM connection */
  viewer_fam_close(viewer);

  /* Close search history */
  search_history_save(viewer);
  search_history_done(viewer);

  free(viewer->title);

  if ( viewer->filename != NULL )
    free(viewer->filename);

  list_done(viewer->list);

  if ( viewer->filew != NULL )
    gtk_widget_destroy(viewer->filew);

  gdk_cursor_destroy(viewer->cursor_watch);
  gtk_widget_destroy(viewer->search_combo);
  gtk_widget_destroy(viewer->window);

  free(viewer);
}


void viewer_destroyed(viewer_t *viewer, viewer_destroyed_t *destroyed, void *arg)
{
  viewer->destroyed = destroyed;
  viewer->destroyed_arg = arg;
}
