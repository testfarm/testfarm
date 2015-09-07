/****************************************************************************/
/* TestFarm                                                                 */
/* Graphical User Interface : Stdin/Stdout/Stderr display                   */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 07-MAR-2005                                                    */
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
#include <gtk/gtk.h>

#include "options.h"
#include "support.h"
#include "color.h"
#include "style.h"
#include "output_gui.h"


static GtkStyle *output_gui_style[OUTPUT_CHANNEL_N] = {};


static int output_gui_show_line(output_gui_t *og, int channel, char *str)
{
  GtkCList *clist = GTK_CLIST(og->clist);
  char *tab[1] = { str };
  int count;

  /* Append new line */
  count = gtk_clist_append(clist, tab);
  gtk_clist_set_row_data(clist, count, GINT_TO_POINTER(0));
  gtk_clist_set_column_width(clist, 0, gtk_clist_optimal_column_width(clist, 0));

  /* Barbouillate line in red if it comes from standard error flow */
  if ( channel < OUTPUT_CHANNEL_N ) {
    gtk_clist_set_row_style(clist, count, output_gui_style[channel]);
    //gtk_clist_set_foreground(clist, count, &color_red);
  }

  /* Remove first line if we go deeper than maximum depth */
  if ( (og->depth > 0) && (count >= og->depth) ) {
    /* Replace first line with warning message if not already done */
    if ( GPOINTER_TO_INT(gtk_clist_get_row_data(clist, 0)) ) {
      gtk_clist_remove(clist, 1);
      count--;
    }
    else {
      char warn[80];

      gtk_clist_remove(clist, 0);

      tab[0] = warn;
      sprintf(warn, "Output dump too long -- display truncated to %lu last lines", og->depth);
      gtk_clist_prepend(clist, tab);
      gtk_clist_set_row_data(clist, 0, GINT_TO_POINTER(1));
      gtk_clist_set_foreground(clist, 0, &color_green);
    }
  }

  return count;
}


void output_gui_show(output_gui_t *og, char *name, int is_root)
{
  GtkCList *clist;

  if ( name == NULL )
    return;

  /* Show node name (truncated if too long) */
  {
    char *str = name;
    int len = strlen(str);
    char buf[len+4];

    if ( len > 32 ) {
      str += (len-32);
      sprintf(buf, "...%s", str);
      str = buf;
    }

    gtk_label_set_text(GTK_LABEL(og->label), str);
  }

  clist = GTK_CLIST(og->clist);
  gtk_clist_freeze(clist);

  /* Clear output dump */
  gtk_clist_clear(clist);
  gtk_clist_set_column_width(clist, 0, gtk_clist_optimal_column_width(clist, 0));

  /* Show output dump */
  if ( og->out != NULL ) {
    output_read_node_stdio(og->out, is_root ? NULL : name,
			   (output_file_func_t *) output_gui_show_line, og,
			   NULL);
  }

  gtk_clist_thaw(clist);
}


void output_gui_scrolldown(output_gui_t *og)
{
  GtkCList *clist = GTK_CLIST(og->clist);
  int row = clist->rows - 1;
  gtk_clist_moveto(clist, row, 0, 1.0, 0.0);
}


void output_gui_feed(output_gui_t *og, int channel, char *str)
{
  GtkCList *clist = GTK_CLIST(og->clist);
  GtkVisibility visibility;
  int row;

  if ( clist->rows > 0 )
    visibility = gtk_clist_row_is_visible(clist, clist->rows - 1);
  else
    visibility = GTK_VISIBILITY_NONE;

  /* Update output display window */
  gtk_clist_freeze(clist);
  row = output_gui_show_line(og, channel, str);
  gtk_clist_thaw(clist);

  /* Shift display to make the new line viewable */
  if ( visibility == GTK_VISIBILITY_FULL )
    gtk_clist_moveto(clist, row, 0, 1.0, 0.0);
}


void output_gui_clear(output_gui_t *og)
{
  /* Clear output display */
  gtk_clist_freeze(GTK_CLIST(og->clist));
  gtk_clist_clear(GTK_CLIST(og->clist));
  gtk_clist_thaw(GTK_CLIST(og->clist));
}


void output_gui_set(output_gui_t *og, output_t *out)
{
  og->out = out;
}


output_gui_t *output_gui_init(GtkWidget *window)
{
  output_gui_t *og;
  int i;

  og = (output_gui_t *) malloc(sizeof(output_gui_t));

  og->window = window;
  og->label = lookup_widget(window, "case_name");
  og->clist = lookup_widget(window, "case_output");
  og->depth = opt_depth;
  og->out = NULL;

  for (i = 0; i < OUTPUT_CHANNEL_N; i++)
    output_gui_style[i] = style_white_fixed;

  output_gui_style[OUTPUT_CHANNEL_STDIN] = style_blue_fixed;
  output_gui_style[OUTPUT_CHANNEL_STDERR] = style_red_fixed;

  return og;
}


void output_gui_destroy(output_gui_t *og)
{
  if ( og == NULL )
    return;
  free(og);
}
