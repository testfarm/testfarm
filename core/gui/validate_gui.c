/****************************************************************************/
/* TestFarm                                                                 */
/* Graphical User Interface : File validation user interface                */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 16-APR-2004                                                    */
/****************************************************************************/

/*
 * $Revision: 765 $
 * $Date: 2007-10-05 12:53:10 +0200 (ven., 05 oct. 2007) $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <time.h>
#include <gtk/gtk.h>

#include "useful.h"
#include "codegen.h"
#include "support.h"
#include "properties.h"
#include "validate.h"
#include "xpm_gui.h"
#include "validate_gui.h"


static void validate_gui_check(validate_gui_t *vg)
{
  if ( vg->checked == vg->selected )
    return;
  vg->checked = vg->selected;

  if ( vg->checked != NULL ) {
    vg->validated = validate_check(validate_object_data(vg->checked), vg->checked);
  }
}


static inline void validate_gui_pixbuf(GtkWidget *widget, GdkPixbuf *pixbuf)
{
  gtk_image_set_from_pixbuf(GTK_IMAGE(widget), pixbuf);
}


static GdkPixbuf **validate_gui_icon = NULL;


#define VALIDATE_GUI_SCRIPT_MAXLEN 32

static char *path_truncate(char *path, int maxlen)
{
  int len = strlen(path);
  char *buf;

  if ( len > maxlen ) {
    int i = len - maxlen;
    path += i;
    len -= i;
    buf = malloc(len+4);
    snprintf(buf, len+4, "...%s", path);
  }
  else {
    buf = strdup(path);
  }

  return buf;
}


static void validate_gui_show(validate_gui_t *vg)
{
  validate_gui_check(vg);

  if ( vg->selected == NULL ) {
    gtk_label_set_text(vg->w_script, "N/A");
    gtk_label_set_text(vg->w_last, "");
    validate_gui_pixbuf(vg->w_script_modified, pixbuf_blank);
    validate_gui_pixbuf(vg->w_updated, pixbuf_blank);
    if ( validate_gui_icon != NULL )
      validate_gui_pixbuf(vg->w_icon, validate_gui_icon[0]);
    else
      validate_gui_pixbuf(vg->w_icon, pixbuf_blank);
  }
  else {
    validate_t *v = validate_object_data(vg->selected);

    /* Show script file name */
    {
      char *str = path_truncate(vg->selected->d.Case->script->name, VALIDATE_GUI_SCRIPT_MAXLEN);
      gtk_label_set_text(vg->w_script, str);
      free(str);
    }

    /* Clamp level to known values */
    vg->level = v->level;
    if ( (vg->level < 0) || (vg->level > validate_max) )
      vg->level = 0;

    /* If script has validation state, check it is still valid */
    if ( vg->level > 0 ) {
      char *date, *p;
      char last[80];

      /* Format date as "DD Mmm YYYY HH:MM:SS" */
      p = date = strdup(v->date);
      while ( *p != NUL ) {
        if ( *p == '-' )
          *p = ' ';
        p++;
      }

      if ( vg->validated )
        snprintf(last, sizeof(last), "%s %s by %s", validate_get_id(vg->level), date, v->operator);
      else
        snprintf(last, sizeof(last), "(%s %s by %s)", validate_get_id(vg->level), date, v->operator);

      free(date);

      gtk_label_set_text(vg->w_last, last);
    }
    else {
      gtk_label_set_text(vg->w_last, validate_get_id(0));
    }

    /* Display script modification icon */
    if ( (vg->level > 0) && (! vg->validated) )
      validate_gui_pixbuf(vg->w_script_modified, pixbuf_modified);
    else
      validate_gui_pixbuf(vg->w_script_modified, pixbuf_blank);

    /* Set option menu to current validation state */
    {
      int i = vg->validated ? vg->level : 0;

      if ( validate_gui_icon != NULL )
	gtk_image_set_from_pixbuf(GTK_IMAGE(vg->w_icon), validate_gui_icon[i]);
    }

    /* Display state modification icon */
    if ( v->updated )
      validate_gui_pixbuf(vg->w_updated, pixbuf_updated);
    else
      validate_gui_pixbuf(vg->w_updated, pixbuf_blank);
  }

  gtk_widget_set_sensitive(vg->w_box, (vg->selected != NULL));
}


static int validation_gui_active(validate_gui_t *vg)
{
  return gtk_expander_get_expanded(GTK_EXPANDER(vg->w_expander));
}


void validate_gui_select(validate_gui_t *vg, tree_object_t *object)
{
  /* Unselect object if not a script */
  if ( (object != NULL) && (object->type == TYPE_CASE) )
    vg->selected = object;
  else
    vg->selected = NULL;

  /* Nothing to show if validation tool is not visible */
  if ( ! validation_gui_active(vg) )
    return;

  /* Show new selection */
  validate_gui_show(vg);
}


static void validate_gui_update_0(validate_gui_t *vg)
{
  validate_t *v;

  /* Ignore operation if no Operator Name given */
  if ( vg->prop->operator == NULL )
    return;

  /* Get validation state descriptor */
  v = validate_object_data(vg->checked);

  /* Update validation state */
  validate_update(v, vg->checked, vg->level, vg->prop->operator);
  vg->validated = 1;

  validate_gui_show(vg);
}


static void validate_gui_update(GtkMenuItem *menuitem, validate_gui_t *vg)
{
  validate_t *v;

  /* Retrieve currently selected test case */
  if ( vg->checked == NULL )
    return;

  /* Get validation state descriptor */
  v = validate_object_data(vg->checked);

  /* Get selected validation level */
  vg->level = GPOINTER_TO_INT(gtk_object_get_data(GTK_OBJECT(menuitem), "level"));
  if ( vg->level == v->level )
    return;

  /* Check operator name, then perform validation change */
  properties_check(vg->prop, "Operator Name is required\nfor updating the Script Validation State",
		   (void *) validate_gui_update_0, vg);
}


static void validate_gui_switch(validate_gui_t *vg)
{
  int expanded = ! validation_gui_active(vg);

  if ( expanded ) {
    /* Show current selection */
    validate_gui_show(vg);
  }
}


static void validate_gui_change(validate_gui_t *vg)
{
  if ( vg->selected != NULL )
    gtk_menu_popup(GTK_MENU(vg->w_menu), NULL, NULL, NULL, NULL, 1, gtk_get_current_event_time());
}


validate_gui_t *validate_gui_init(GtkWidget *window, properties_t *prop)
{
  validate_gui_t *vg;
  int i;

  /* Alloc validation session buffer */
  vg = (validate_gui_t *) malloc(sizeof(validate_gui_t));

  /* Session properties */
  vg->prop = prop;

  /* Retrieve widgets */
  vg->w_expander = lookup_widget(window, "validation_expander");
  vg->w_box = lookup_widget(window, "validation_eventbox");
  vg->w_icon = lookup_widget(window, "validation_icon");
  vg->w_script = GTK_LABEL(lookup_widget(window, "validation_script"));
  vg->w_last = GTK_LABEL(lookup_widget(window, "validation_last"));
  vg->w_script_modified = lookup_widget(window, "validation_script_modified");
  vg->w_updated = lookup_widget(window, "validation_updated");
  vg->selected = NULL;
  vg->checked = NULL;
  vg->validated = 0;

  /* Setup icons */
  validate_gui_pixbuf(vg->w_script_modified, pixbuf_blank);
  validate_gui_pixbuf(vg->w_updated, pixbuf_blank);

  if ( validate_gui_icon == NULL ) {
    validate_gui_icon = (GdkPixbuf **) calloc(sizeof(GdkPixbuf *), (validate_max+1));
    for (i = 0; i <= validate_max; i++) {
      validate_gui_icon[i] = create_pixbuf(validate_get_icon(i));
    }
  }

  /* Setup menus */
  vg->w_menu = gtk_menu_new();

  for (i = 0; i <= validate_max; i++) {
    GtkWidget *menuitem;
    GtkWidget *image;
    char str[32];
    char *id;

    /* Get validation state id */
    id = validate_get_id(i);

    /* Setup update menu for this validation state */
    snprintf(str, sizeof(str), "Set as %s", id);
    menuitem = gtk_image_menu_item_new_with_mnemonic(str);
    gtk_container_add(GTK_CONTAINER(vg->w_menu), menuitem);
    gtk_widget_show_all(vg->w_menu);

    image = create_pixmap(window, validate_get_icon(i));
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menuitem), image);
    gtk_object_set_data(GTK_OBJECT(menuitem), "level", GINT_TO_POINTER(i));
    gtk_signal_connect(GTK_OBJECT(menuitem), "activate",
		       GTK_SIGNAL_FUNC(validate_gui_update), vg);
  }

  gtk_widget_show_all(vg->w_menu);

  /* Setup state change popup event */
  gtk_signal_connect_object(GTK_OBJECT(lookup_widget(window, "validation_eventbox")), "button_press_event",
			    GTK_SIGNAL_FUNC(validate_gui_change), vg);

  /* Setup validation area expander */
  validate_gui_switch(vg);
  gtk_signal_connect_object(GTK_OBJECT(vg->w_expander), "activate",
			    GTK_SIGNAL_FUNC(validate_gui_switch), vg);

  return vg;
}


void validate_gui_destroy(validate_gui_t *vg)
{
  if ( vg == NULL )
    return;

  free(vg);
}
