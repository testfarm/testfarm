/****************************************************************************/
/* TestFarm                                                                 */
/* Graphical User Interface : Test Suite properties                         */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 03-MAR-2005                                                    */
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
#include <string.h>
#include <malloc.h>
#include <unistd.h>
#include <gtk/gtk.h>

#include "interface.h"
#include "support.h"
#include "options.h"
#include "properties.h"


static void properties_label_set(GtkWidget *window, char *id, char *text)
{
  if ( text == NULL )
    text = "(none)";
  gtk_label_set_text(GTK_LABEL(lookup_widget(window, id)), text);
}


static void properties_label_set_all(properties_t *prop)
{
  if ( prop->window == NULL )
    return;
  properties_label_set(prop->window, "system", prop->system);
  properties_label_set(prop->window, "directory", prop->dir_name);
  properties_label_set(prop->window, "tree", prop->tree_name);
}


static char *properties_entry_get(GtkWidget *window, char *id)
{
  /* Retrieve text entry */
  char *s = (char *) gtk_entry_get_text(GTK_ENTRY(lookup_widget(window, id)));

  /* Cleanup text entry */
  while ( (*s != '\0') && (*s <= ' ') )
    s++;

  return s;
}


static void properties_entry_set(GtkWidget *window, char *id, char *text)
{
  if ( text == NULL )
    return;
  gtk_entry_set_text(GTK_ENTRY(lookup_widget(window, id)), text);
}


static void properties_entry_validate(properties_t *prop)
{
  char *s;

  /* Set new operator name if not blank */
  if ( prop->operator != NULL )
    free(prop->operator);
  prop->operator = NULL;
  s = properties_entry_get(prop->window, "operator");
  if ( *s != '\0' )
    prop->operator = strdup(s);

  /* Set test suite identification */
  if ( prop->release != NULL )
    free(prop->release);
  prop->release = NULL;
  s = properties_entry_get(prop->window, "release");
  if ( *s != '\0' )
    prop->release = strdup(s);

  /* Destroy entry window */
  gtk_widget_destroy(prop->window);
}


static void properties_entry_done(properties_t *prop)
{
  void (*done)(void *) = gtk_object_get_data(GTK_OBJECT(prop->window), "done");
  void *arg = gtk_object_get_data(GTK_OBJECT(prop->window), "arg");

  prop->window = NULL;

  if ( done != NULL )
    done(arg);
}


void properties_entry(properties_t *prop)
{
  /* Create properties warning+entry window */
  prop->window = create_properties_window();

  gtk_signal_connect_object(GTK_OBJECT(prop->window), "destroy",
			    GTK_SIGNAL_FUNC(properties_entry_done), prop);
  gtk_signal_connect_object(GTK_OBJECT(lookup_widget(prop->window, "cancel")), "clicked",
                            GTK_SIGNAL_FUNC(gtk_widget_destroy), prop->window);
  gtk_signal_connect_object(GTK_OBJECT(lookup_widget(prop->window, "validate")), "clicked",
                            GTK_SIGNAL_FUNC(properties_entry_validate), prop);

  properties_label_set_all(prop);
  properties_entry_set(prop->window, "operator", prop->operator);
  properties_entry_set(prop->window, "release", prop->release);

  /* Show operator entry window */
  gtk_widget_show(prop->window);
}


void properties_check(properties_t *prop, char *msg, void (*done)(void *), void *arg)
{
  if ( prop->operator != NULL ) {
    if ( done != NULL )
      done(arg);
    return;
  }

  /* Create properties warning+entry window */
  properties_entry(prop);

  gtk_object_set_data(GTK_OBJECT(prop->window), "done", done);
  gtk_object_set_data(GTK_OBJECT(prop->window), "arg", arg);

  gtk_label_set_text(GTK_LABEL(lookup_widget(prop->window, "label")), msg);
  gtk_widget_show_all(lookup_widget(prop->window, "warning"));
}


void properties_set(properties_t *prop, char *system, char *tree_name)
{
  /* Set system name */
  if ( prop->system != NULL ) {
    free(prop->system);
    prop->system = NULL;
  }
  if ( system != NULL )
    prop->system = strdup(system);

  /* Set directory name */
  if ( prop->dir_name != NULL ) {
    free(prop->dir_name);
    prop->dir_name = NULL;
  }

  prop->dir_name = g_get_current_dir();

  /* Set test suite name */
  if ( prop->tree_name != NULL ) {
    free(prop->tree_name);
    prop->tree_name = NULL;
  }
  if ( tree_name != NULL )
    prop->tree_name = strdup(tree_name);

  /* Refresh properties display is needed */
  properties_label_set_all(prop);
}


properties_t *properties_init(GtkWidget *window)
{
  properties_t *prop;

  prop = (properties_t *) malloc(sizeof(properties_t));

  prop->system = NULL;
  prop->dir_name = NULL;
  prop->tree_name = NULL;

  prop->operator = NULL;
  if ( opt_operator != NULL )
    prop->operator = strdup(opt_operator);

  prop->release = NULL;
  if ( opt_release != NULL )
    prop->release = strdup(opt_release);

  /* No properties edition window yet */
  prop->window = NULL;

  /* Hook properties window raise event */
  if ( window != NULL ) {
    gtk_signal_connect_object(GTK_OBJECT(lookup_widget(window, "session_properties")), "activate",
			      GTK_SIGNAL_FUNC(properties_entry), prop);
  }

  return prop;
}


void properties_destroy(properties_t *prop)
{
  if ( prop == NULL )
    return;
  if ( prop->operator != NULL )
    free(prop->operator);
  if ( prop->release != NULL )
    free(prop->release);
  free(prop);
}
