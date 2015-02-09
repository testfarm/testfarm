/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* Display Tool - command entry and history                                 */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 08-AUG-2010                                                    */
/****************************************************************************/

/*
 * $Revision: 1185 $
 * $Date: 2010-08-08 15:34:16 +0200 (dim., 08 août 2010) $
 */

#include <stdio.h>
#include <gtk/gtk.h>

#include "support.h"

#include "frame_ctl_msg.h"
#include "display_command.h"


#define eprintf(args...) //fprintf(stderr, "display_command: " args)


/* Command history tree model */
enum {
  COMMAND_TREE_ID,
  COMMAND_TREE_TEXT,
  COMMAND_TREE_NCOLS
};

static GtkListStore *display_command_list = NULL;

static GtkWidget *display_command_box = NULL;
static GtkComboBoxEntry *display_command_combo = NULL;
static GtkEntry *display_command_entry = NULL;
static int display_command_sock = -1;


static void display_command_activated(void)
{
  if ( display_command_sock >= 0 ) {
    char *cmd = (char *) gtk_combo_box_get_active_text(GTK_COMBO_BOX(display_command_combo));
    frame_ctl_command(display_command_sock, cmd);
  }
}


static gboolean display_command_find(int id, GtkTreeIter *iter)
{
	/* Find iter corresponding to id */
	if (gtk_tree_model_get_iter_first(GTK_TREE_MODEL(display_command_list), iter)) {
		while (1) {
			int cur_id;

			gtk_tree_model_get(GTK_TREE_MODEL(display_command_list), iter,
					   COMMAND_TREE_ID, &cur_id,
					   -1);

			if (cur_id == id)
				return TRUE;

			gtk_tree_model_iter_next(GTK_TREE_MODEL(display_command_list), iter);
		}
	}

	return FALSE;
}


void display_command_history(int id, char *cmd)
{
	GtkTreeIter iter;

	if (id < 0) {
		eprintf("DEL %d\n", -id);
		if (display_command_find(-id, &iter))
			gtk_list_store_remove(display_command_list, &iter);
	}
	else if (id == 0) {
		eprintf("CLEAR\n");
		gtk_list_store_clear(display_command_list);
	}
	else {
		/* Command history reordering */
		if (*cmd == '\0') {
			eprintf("MOVE UP %d\n", id);
			if (display_command_find(id, &iter)) {
				gtk_list_store_move_after(display_command_list, &iter, NULL);
			}
		}

		/* New command to prepend */
		else {
			eprintf("ADD %d '%s'\n", id, cmd);
			gtk_list_store_prepend(display_command_list, &iter);
			gtk_list_store_set(display_command_list, &iter,
					   COMMAND_TREE_ID, id,
					   COMMAND_TREE_TEXT, cmd,
					   -1);
		}
	}
}


void display_command_connect(int sock)
{
	display_command_sock = sock;
	gtk_widget_set_sensitive(display_command_box, 1);
	gtk_list_store_clear(display_command_list);
}


void display_command_disconnect(void)
{
	display_command_sock = -1;
	gtk_widget_set_sensitive(display_command_box, 0);
	gtk_list_store_clear(display_command_list);
}


void display_command_init(GtkWindow *window)
{
	/* Retrieve command entry widgets */
	display_command_box = lookup_widget(GTK_WIDGET(window), "command_box");
	gtk_widget_set_sensitive(display_command_box, 0);

	display_command_combo = GTK_COMBO_BOX_ENTRY(lookup_widget(GTK_WIDGET(window), "command_entry"));
	display_command_entry = GTK_ENTRY(gtk_bin_get_child(GTK_BIN(display_command_combo)));
	gtk_signal_connect_object(GTK_OBJECT(display_command_entry), "activate",
				  GTK_SIGNAL_FUNC(display_command_activated), NULL);
	gtk_signal_connect_object(GTK_OBJECT(lookup_widget(GTK_WIDGET(window), "command_send")), "clicked",
				  GTK_SIGNAL_FUNC(display_command_activated), NULL);

	/* Create command history tree model */
	display_command_list = gtk_list_store_new(COMMAND_TREE_NCOLS,
						  /* COMMAND_TREE_ID   */ G_TYPE_INT,
						  /* COMMAND_TREE_TEXT */ G_TYPE_STRING);
	gtk_combo_box_set_model(GTK_COMBO_BOX(display_command_combo), GTK_TREE_MODEL(display_command_list));
	gtk_combo_box_entry_set_text_column(display_command_combo, COMMAND_TREE_TEXT);
}


void display_command_done(void)
{
	display_command_disconnect();
}
