/****************************************************************************/
/* TestFarm                                                                 */
/* Graphical User Interface : Report Options editor                         */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 07-JUN-2004                                                    */
/****************************************************************************/

/*
 * $Revision: 274 $
 * $Date: 2006-11-11 18:45:38 +0100 (sam., 11 nov. 2006) $
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <gtk/gtk.h>

#include "install.h"
#include "useful.h"

#include "interface.h"
#include "support.h"
#include "report_config.h"
#include "report_config_gui.h"


#define DEFAULT_NAME "Default"


static void report_config_gui_setup(GtkWidget *window, report_config_t *rc)
{
  report_config_item_t *item = report_config_table;
  int state;

  /* Get current report configuration */
  while ( item->id != NULL ) {
    GtkWidget *w = lookup_widget(window, item->id);
    state = (rc->conf & item->mask) ? 1:0;
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(w), state);
    item++;
  }

  /* Setup stylesheet configuration */
  state = (rc->conf & REPORT_CONFIG_STANDARD) ? 0:1;
  gtk_widget_set_sensitive(lookup_widget(window, "stylesheet_box"), state);

  if ( rc->stylesheet != NULL )
    gtk_entry_set_text(GTK_ENTRY(lookup_widget(window, "stylesheet_entry")), rc->stylesheet);

  /* Setup output dump configuration */
  state = (rc->conf & REPORT_CONFIG_DUMP) ? 1:0;
  gtk_widget_set_sensitive(lookup_widget(window, "show_dump_box"), state);
}


static void report_config_gui_name_changed(GtkComboBox *combo, GtkWidget *window)
{
  int row = gtk_combo_box_get_active(combo);
  char *loaded = NULL;

  if ( row >= 0 ) {
    char *name = (char *) gtk_entry_get_text(GTK_ENTRY(GTK_BIN(combo)->child));
    report_config_t *rc = gtk_object_get_data(GTK_OBJECT(window), "report_config");

    loaded = report_config_load(rc, name);

    report_config_gui_setup(window, rc);
  }

  gtk_widget_set_sensitive(lookup_widget(window, "delete"), (loaded != NULL));
}


static void report_config_gui_name_feed(GtkWidget *window, char *name)
{
  GtkComboBox *combo;
  GtkListStore *model;
  GtkTreeIter iter;
  GDir *dir;
  int row = 0;

  combo = GTK_COMBO_BOX(lookup_widget(window, "name"));
  model = GTK_LIST_STORE(gtk_combo_box_get_model(combo));
  gtk_list_store_clear(model);

  dir = g_dir_open(get_user_config(REPORT_CONFIG_SUBDIR), 0, NULL);
  if ( dir != NULL ) {
    char *fn;

    while ( (fn = (char *) g_dir_read_name(dir)) != NULL ) {
      gtk_list_store_append(model, &iter);
      gtk_list_store_set(model, &iter, 0, fn, -1);

      /* Set default report config name */
      if ( (name != NULL) && (strcmp(fn, name) == 0) )
	gtk_combo_box_set_active(combo, row);

      row++;
    }

    g_dir_close(dir);
  }

  /* Add built-in default config name */
  gtk_list_store_append(model, &iter);
  gtk_list_store_set(model, &iter, 0, DEFAULT_NAME, -1);

  if ( (name == NULL) || (strcmp(name, DEFAULT_NAME) == 0)  ) {
    gtk_widget_set_sensitive(lookup_widget(window, "delete"), FALSE);
    gtk_combo_box_set_active(combo, row);
  }
}


static void report_config_gui_name_init(GtkWidget *window, char *name)
{
  GtkComboBox *combo;
  GtkListStore *model;

  /* Init combo */
  combo = GTK_COMBO_BOX(lookup_widget(window, "name"));
  model = gtk_list_store_new(1, G_TYPE_STRING);
  gtk_combo_box_set_model(combo, GTK_TREE_MODEL(model));
  gtk_combo_box_entry_set_text_column(GTK_COMBO_BOX_ENTRY(combo), 0);

  /* Get all available report configs */
  report_config_gui_name_feed(window, name);

  /* Setup change callback */
  gtk_signal_connect(GTK_OBJECT(combo), "changed",
		     GTK_SIGNAL_FUNC(report_config_gui_name_changed), window);
}


static char *report_config_gui_name_accept(GtkWidget *window)
{
  GtkComboBox *combo;
  GtkTreeModel *model;
  GtkTreeIter iter;
  int row;
  char *name = NULL;

  combo = GTK_COMBO_BOX(lookup_widget(window, "name"));
  model = gtk_combo_box_get_model(combo);
  row = gtk_combo_box_get_active(combo);

  /* New entry name */
  if ( row < 0 ) {
    char *text = (char *) gtk_entry_get_text(GTK_ENTRY(GTK_BIN(combo)->child));

    text = g_strstrip(text);

    /* Ensure new name is not already present in the list */
    if ( gtk_tree_model_get_iter_first(model, &iter) ) {
      do {
	gtk_tree_model_get(model, &iter, 0, &name, -1);
	if ( strcmp(name, text) == 0 )
	  text = NULL;
	g_free(name);
	name = NULL;
      } while ( (text != NULL) && gtk_tree_model_iter_next(model, &iter) );
    }

    /* If new name, add it to the list */
    if ( text != NULL ) {
      gtk_list_store_prepend(GTK_LIST_STORE(model), &iter);
      gtk_list_store_set(GTK_LIST_STORE(model), &iter, 0, text, -1);
    }
  }

  /* Name selected from list */
  else {
    GtkTreePath *path = gtk_tree_path_new_from_indices(row, -1);
    gtk_tree_model_get_iter(model, &iter, path);
    gtk_tree_path_free(path);
  }

  gtk_tree_model_get(model, &iter, 0, &name, -1);

  if ( name[0] == '\0' ) {
    g_free(name);
    name = strdup(DEFAULT_NAME);
  }

  return name;
}


static void report_config_gui_name_delete(GtkWidget *window)
{
  GtkWidget *parent_window = gtk_object_get_data(GTK_OBJECT(window), "parent_window");
  report_config_t *rc = gtk_object_get_data(GTK_OBJECT(parent_window), "report_config");

  if ( rc->fname != NULL ) {
    if ( remove(rc->fname) == 0 )
      fprintf(stderr, "Report Configuration \"%s\" removed\n", rc->name);
    else
      fprintf(stderr, "Cannot remove Report Configuration \"%s\": %s\n", rc->name, strerror(errno));
  }

  report_config_gui_name_feed(parent_window, NULL);

  report_config_clear(rc);
  report_config_gui_setup(parent_window, rc);

  gtk_widget_destroy(window);
}


static void report_config_gui_accept_clicked(GtkWidget *widget, GtkWidget *window)
{
  report_config_t *rc = gtk_object_get_data(GTK_OBJECT(window), "report_config");
  GtkWidget *entry;
  char *stylesheet;
  report_config_item_t *item;
  char *name;

  /* Set report stylesheet */
  entry = lookup_widget(window, "stylesheet_entry");
  stylesheet = strskip_spaces((char *) gtk_entry_get_text(GTK_ENTRY(entry)));
  if ( (*stylesheet != NUL) && (access(stylesheet, R_OK)) ) {
    GtkWidget *window;
    char str[256];

    window = create_error_window();
    gtk_signal_connect_object(GTK_OBJECT(lookup_widget(window, "ok")), "clicked",
			      GTK_SIGNAL_FUNC(gtk_widget_destroy), (gpointer) window);

    snprintf(str, sizeof(str), "Cannot access stylesheet\n%s:\n%s", stylesheet, strerror(errno));
    gtk_label_set_text(GTK_LABEL(lookup_widget(window, "label")), str);

    gtk_widget_show(window);
  }
  else {
    report_config_set_stylesheet(rc, stylesheet);
  }

  /* Set report configuration flags */
  item = report_config_table;
  while ( item->id != NULL ) {
    GtkWidget *w = lookup_widget(window, item->id);

    if ( gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w)) )
      rc->conf |= item->mask;
    else
      rc->conf &= ~(item->mask);

    item++;
  }

  /* Save report configuration */
  name = report_config_gui_name_accept(window);

  if ( (name != NULL) && (strcmp(name, DEFAULT_NAME) != 0) ) {
    if ( report_config_save(rc, name) == 0 )
      fprintf(stderr, "Report configuration \"%s\" saved\n", rc->name);

    g_free(name);
  }

  gtk_widget_destroy(window);
}


static void report_config_gui_delete_clicked(GtkWidget *parent_window)
{
  const char *message = "Please confirm deletion of\nReport Configuration \"%s\"";
  report_config_t *rc = gtk_object_get_data(GTK_OBJECT(parent_window), "report_config");
  GtkWidget *window;
  char *text;
  int len;

  /* Raise confirmation window */
  window = create_warning_window();
  gtk_object_set_data(GTK_OBJECT(window), "parent_window", parent_window);

  len = strlen(message) + strlen(rc->name);
  text = (char *) g_malloc(len+1);
  snprintf(text, len, message, rc->name);
  gtk_label_set_text(GTK_LABEL(lookup_widget(window, "label")), text);
  g_free(text);

  gtk_signal_connect_object(GTK_OBJECT(lookup_widget(window, "no")), "clicked",
                            GTK_SIGNAL_FUNC(gtk_widget_destroy), window);
  gtk_signal_connect_object(GTK_OBJECT(lookup_widget(window, "yes")), "clicked",
                            GTK_SIGNAL_FUNC(report_config_gui_name_delete), window);

  gtk_widget_show(window);
}


static void report_config_gui_standard_clicked(GtkWidget *widget, GtkWidget *window)
{
  int state = ! gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));

  gtk_widget_set_sensitive(lookup_widget(window, "stylesheet_box"), state);
}


static void report_config_gui_stylesheet_ok_clicked(GtkWidget *widget, GtkWidget *browse)
{
  char *stylesheet = (char *) gtk_file_selection_get_filename(GTK_FILE_SELECTION(browse));
  GtkWidget *entry = gtk_object_get_data(GTK_OBJECT(browse), "stylesheet_entry");

  gtk_entry_set_text(GTK_ENTRY(entry), stylesheet);

  gtk_widget_destroy(browse);
}


static void report_config_gui_stylesheet_browse_clicked(GtkWidget *widget, GtkWidget *window)
{
  GtkWidget *browse = gtk_file_selection_new("TestFarm Report Stylesheet");

  gtk_object_set_data(GTK_OBJECT(browse), "stylesheet_entry", lookup_widget(window, "stylesheet_entry"));

  gtk_file_selection_hide_fileop_buttons(GTK_FILE_SELECTION(browse));
  gtk_window_set_position(GTK_WINDOW(browse), GTK_WIN_POS_MOUSE);
  gtk_window_set_modal(GTK_WINDOW(browse), TRUE);

  gtk_signal_connect(GTK_OBJECT(GTK_FILE_SELECTION(browse)->ok_button), "clicked",
                     (GtkSignalFunc) report_config_gui_stylesheet_ok_clicked, (gpointer) browse);
  gtk_signal_connect_object(GTK_OBJECT(GTK_FILE_SELECTION(browse)->cancel_button), "clicked",
                            (GtkSignalFunc) gtk_widget_destroy, (gpointer) browse);

  gtk_widget_show(browse);
}


static void report_config_gui_dump_clicked(GtkWidget *widget, GtkWidget *window)
{
  int state = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
  gtk_widget_set_sensitive(lookup_widget(window, "show_dump_box"), state);
}


GtkWidget *report_config_gui(char *name)
{
  GtkWidget *window;
  report_config_t *rc;
  char *loaded;

  /* Create report config editor window */
  window = create_report_config_window();

  /* Load report config file */
  rc = report_config_alloc();
  loaded = report_config_load(rc, name);
  gtk_widget_set_sensitive(lookup_widget(window, "delete"), (loaded != NULL));

  /* Attach config descriptor to window */
  gtk_object_set_data_full(GTK_OBJECT(window), "report_config", rc,
			   (GtkDestroyNotify) report_config_destroy);

  /* Setup report config name entry */
  report_config_gui_name_init(window, rc->name);

  /* Setup report config representation */
  report_config_gui_setup(window, rc);

  gtk_signal_connect(GTK_OBJECT(lookup_widget(window, "show_standard")), "clicked",
                     GTK_SIGNAL_FUNC(report_config_gui_standard_clicked), window);
  gtk_signal_connect(GTK_OBJECT(lookup_widget(window, "stylesheet_browse")), "clicked",
                     GTK_SIGNAL_FUNC(report_config_gui_stylesheet_browse_clicked), window);

  gtk_signal_connect(GTK_OBJECT(lookup_widget(window, "show_dump")), "clicked",
                     GTK_SIGNAL_FUNC(report_config_gui_dump_clicked), window);

  /* Hook report config window callbacks */
  gtk_signal_connect_object(GTK_OBJECT(lookup_widget(window, "cancel")), "clicked",
                            GTK_SIGNAL_FUNC(gtk_widget_destroy), window);
  gtk_signal_connect(GTK_OBJECT(lookup_widget(window, "accept")), "clicked",
                     GTK_SIGNAL_FUNC(report_config_gui_accept_clicked), window);
  gtk_signal_connect_object(GTK_OBJECT(lookup_widget(window, "delete")), "clicked",
			    GTK_SIGNAL_FUNC(report_config_gui_delete_clicked), window);

  gtk_widget_show(window);
  return window;
}
