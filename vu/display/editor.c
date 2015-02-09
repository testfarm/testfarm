/****************************************************************************/
/* TestFarm RFB Interface                                                   */
/* Remote Frame Buffer display - Pattern editor                             */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 14-JAN-2004                                                    */
/****************************************************************************/

/*
 * $Revision: 1131 $
 * $Date: 2010-03-31 16:01:06 +0200 (mer., 31 mars 2010) $
 */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <malloc.h>
#include <unistd.h>
#include <gtk/gtk.h>

#include "interface.h"
#include "support.h"
#include "utils.h"
#include "style.h"
#include "frame_hdr.h"
#include "png_file.h"
#include "pattern.h"
#include "fuzz.h"
#include "viewer.h"
#include "editor.h"


static GtkWidget *editor_box = NULL;
static GtkEntry *editor_id = NULL;

static GtkToggleButton *editor_mode_appear = NULL;
static GtkToggleButton *editor_mode_disappear = NULL;
static GtkToggleButton *editor_mode_retrigger = NULL;
static GtkToggleButton *editor_mode_brief = NULL;

static GtkLabel *editor_window_frame = NULL;
static GtkSpinButton *editor_window_width = NULL;
static GtkSpinButton *editor_window_height = NULL;
static GtkSpinButton *editor_window_x = NULL;
static GtkSpinButton *editor_window_y = NULL;
static GtkButton *editor_window_set = NULL;

static GtkComboBox *editor_source_type = NULL;

static GtkWidget *editor_image_box = NULL;
static GtkEntry *editor_image_fname = NULL;
static GtkLabel *editor_image_size = NULL;
static GtkButton *editor_image_grab = NULL;
static GtkEntry *editor_image_fuzz = NULL;
static GtkSpinButton *editor_image_badpixels = NULL;
static GtkSpinButton *editor_image_potential = NULL;

static GtkWidget *editor_text_box = NULL;
static GtkToggleButton *editor_text_inverse = NULL;
static GtkEntry *editor_text_regex = NULL;

static GtkButton *editor_apply = NULL;
static GtkButton *editor_revert = NULL;
static GtkLabel *editor_error = NULL;

static pattern_t *editor_pattern0 = NULL;
static pattern_t *editor_pattern = NULL;
static frame_hdr_t *editor_cur_frame = NULL;
static frame_hdr_t *editor_sel_frame = NULL;
static frame_geometry_t editor_sel = FRAME_GEOMETRY_NULL;
static editor_func_t *editor_event_fn = NULL;


#define editor_eprintf(args...) if ( eprintf_label ) eprintf(args)

static int editor_apply_values(pattern_t *pattern)
{
  char *err = NULL;
  int ret = 0;
  char *str;
  frame_geometry_t g;
  fuzz_t fuzz = FUZZ_NULL;

  /* Set pattern id */
  str = g_strstrip(strdup((char *) gtk_entry_get_text(editor_id)));
  if ( *str == '\0' ) {
    editor_eprintf("Please define a Pattern Id");
    free(str);
    ret = -1;
  }
  else {
    free(pattern->id);
    pattern->id = str;
  }

  /* Set pattern type */
  pattern->type = gtk_combo_box_get_active(editor_source_type);

  /* Set pattern source */
  switch ( pattern->type ) {
  case PATTERN_TYPE_IMAGE:
    gtk_widget_hide(editor_text_box);
    gtk_widget_show(editor_image_box);

    /* Set image name */
    str = g_strstrip(strdup((char *) gtk_entry_get_text(editor_image_fname)));
    if ( *str == '\0' ) {
      editor_eprintf("Please define an Image File");
      ret = -1;
    }
    else {
      if ( pattern_set_source(pattern, PATTERN_TYPE_IMAGE, str, &err) )
	ret = -1;
    }
    free(str);

    /* Set fuzz name */
    str = g_strstrip(strdup((char *) gtk_entry_get_text(editor_image_fuzz)));
    if ( *str != '\0' ) {
	    if ( fuzz_parse(&fuzz, str) == 0 ) {
		    pattern->d.image.fuzz = fuzz;
	    }
	    else {
		    editor_eprintf("Syntax error in fuzz specification");
		    ret = -1;
	    }
    }
    free(str);

    /* Set loss value */
    pattern->d.image.badpixels_rate = gtk_spin_button_get_value_as_int(editor_image_badpixels);
    pattern->d.image.potential_rate = gtk_spin_button_get_value_as_int(editor_image_potential);
    break;

  case PATTERN_TYPE_TEXT:
    gtk_widget_hide(editor_image_box);
    gtk_widget_show(editor_text_box);

    str = g_strstrip(strdup((char *) gtk_entry_get_text(editor_text_regex)));
    if ( *str == '\0' ) {
      editor_eprintf("Please define a Text Regular Expression");
      ret = -1;
    }
    else {
      if ( pattern_set_source(pattern, PATTERN_TYPE_TEXT, str, &err) )
	ret = -1;
    }

    free(str);
    break;

  default:
    gtk_widget_hide(editor_image_box);
    gtk_widget_hide(editor_text_box);

    editor_eprintf("Please select a Source Type");
    ret = -1;
    break;
  }

  if ( err != NULL ) {
    editor_eprintf(err);
    free(err);
    err = NULL;
  }

  /* Set match window */
  pattern->frame = editor_cur_frame;
  g.x = gtk_spin_button_get_value_as_int(editor_window_x);
  g.y = gtk_spin_button_get_value_as_int(editor_window_y);
  g.width = gtk_spin_button_get_value_as_int(editor_window_width);
  g.height = gtk_spin_button_get_value_as_int(editor_window_height);

  if ( pattern_set_window(pattern, &g) ) {
    editor_eprintf("Illegal match window");
    ret = -1;
  }

  /* Set detection mode */
  pattern->mode = 0;
  if ( gtk_toggle_button_get_active(editor_mode_appear) )
    pattern->mode |= PATTERN_MODE_APPEAR;
  if ( gtk_toggle_button_get_active(editor_mode_disappear) )
    pattern->mode |= PATTERN_MODE_DISAPPEAR;
  if ( gtk_toggle_button_get_active(editor_mode_retrigger) )
    pattern->mode |= PATTERN_MODE_RETRIGGER;

  /* Set inverse mode flag (for text patterns) */
  if ( (pattern->type == PATTERN_TYPE_TEXT) && gtk_toggle_button_get_active(editor_text_inverse) )
    pattern->mode |= PATTERN_MODE_INVERSE;
  else
    pattern->mode &= ~PATTERN_MODE_INVERSE;

  /* Set dump mode */
  if ( gtk_toggle_button_get_active(editor_mode_brief) )
    pattern->mode |= PATTERN_MODE_BRIEF;

  if ( ret == 0 )
    lprintf(editor_error, "");

  return ret;
}


static void editor_set_source_image(pattern_t *pattern)
{
  char s_size[20];

  gtk_widget_hide(editor_text_box);
  gtk_widget_show(editor_image_box);

  gtk_entry_set_text(editor_image_fname, strfilename(pattern->source));

  if ( pattern->d.image.width > 0 )
    snprintf(s_size, sizeof(s_size), "%ux%u", pattern->d.image.width, pattern->d.image.height);
  else
    s_size[0] = '\0';

  gtk_label_set_text(editor_image_size, s_size);

  viewer_show(pattern->source);

  /* Set fuzz value if any */
  gtk_entry_set_text(editor_image_fuzz, fuzz_str(&pattern->d.image.fuzz));
}


static void editor_set_source_text(pattern_t *pattern)
{
  gtk_widget_hide(editor_image_box);
  gtk_widget_show(editor_text_box);

  gtk_entry_set_text(editor_text_regex, pattern->source);

  viewer_show(NULL);
}


static void editor_set_source(pattern_t *pattern)
{
  gtk_combo_box_set_active(editor_source_type, pattern->type);

  switch ( pattern->type ) {
  case PATTERN_TYPE_IMAGE: editor_set_source_image(pattern); break;
  case PATTERN_TYPE_TEXT:  editor_set_source_text(pattern);  break;
  default:                 break;
  }
}


static GtkAdjustment *editor_adjustment_new(int value, int min, int max)
{
  return GTK_ADJUSTMENT(gtk_adjustment_new(value, min, max, 1, 10, 0));
}


static void editor_set_adjustments(pattern_t *pattern)
{
  frame_rgb_t *rgb;
  GtkAdjustment *adj;

  if ( pattern == NULL )
    return;
  if ( pattern->frame == NULL )
    return;
  if ( pattern->frame->fb == NULL )
    return;

  rgb = &(pattern->frame->fb->rgb);

  adj = editor_adjustment_new(pattern->window.width, 1, rgb->width);
  gtk_spin_button_set_adjustment(editor_window_width, adj);

  adj = editor_adjustment_new(pattern->window.height, 1, rgb->height);
  gtk_spin_button_set_adjustment(editor_window_height, adj);

  adj = editor_adjustment_new(pattern->window.x, 0, rgb->width - 1);
  gtk_spin_button_set_adjustment(editor_window_x, adj);

  adj = editor_adjustment_new(pattern->window.y, 0, rgb->height - 1);
  gtk_spin_button_set_adjustment(editor_window_y, adj);
}


static void editor_set_window_geometry(frame_geometry_t *g)
{
  gtk_spin_button_set_value(editor_window_width, g->width);
  gtk_spin_button_set_value(editor_window_height, g->height);
  gtk_spin_button_set_value(editor_window_x, g->x);
  gtk_spin_button_set_value(editor_window_y, g->y);
}


static void editor_set_window(pattern_t *pattern)
{
  if ( pattern == NULL )
    return;

  /* Set pattern frame as current frame */
  editor_cur_frame = pattern->frame;
  if ( editor_cur_frame != NULL ) {
    gtk_label_set_text(editor_window_frame, editor_cur_frame->id);
  }
  else {
    gtk_label_set_text(editor_window_frame, "*NONE*");
  }

  editor_set_window_geometry(&(pattern->window));
}


static void editor_set_mode(pattern_t *pattern)
{
  if ( pattern == NULL )
    return;

  gtk_toggle_button_set_active(editor_mode_appear, (pattern->mode & PATTERN_MODE_APPEAR));
  gtk_toggle_button_set_active(editor_mode_disappear, (pattern->mode & PATTERN_MODE_DISAPPEAR));
  gtk_toggle_button_set_active(editor_mode_retrigger, (pattern->mode & PATTERN_MODE_RETRIGGER));
  gtk_toggle_button_set_active(editor_text_inverse, (pattern->mode & PATTERN_MODE_INVERSE));
  gtk_toggle_button_set_active(editor_mode_brief, (pattern->mode & PATTERN_MODE_BRIEF));
}


static void editor_set_values(pattern_t *pattern)
{
  if ( pattern == NULL )
    return;

  gtk_entry_set_text(editor_id, pattern->id);
  editor_set_source(pattern);
  editor_set_adjustments(pattern);
  editor_set_window(pattern);
  editor_set_mode(pattern);
}


void editor_set_selection(frame_geometry_t *g)
{
  int enable;

  if ( g == NULL ) {
    editor_sel.x = 0;
    editor_sel.y = 0;
    editor_sel.width = 0;
    editor_sel.height = 0;
  }
  else {
    editor_sel = *g;
  }

  enable = (editor_sel.width > 0) && (editor_sel.height > 0);
  gtk_widget_set_sensitive(GTK_WIDGET(editor_image_grab), enable);
  gtk_widget_set_sensitive(GTK_WIDGET(editor_window_set), enable);
}


static int editor_get_selection(frame_hdr_t *frame, frame_geometry_t *g)
{
  int ret = frame_geometry_intersect(&(frame->g0), &editor_sel, g);

  if ( ret ) {
    g->x -= frame->g0.x;
    g->y -= frame->g0.y;
  }
  else {
    editor_eprintf("Please select an area in pattern frame");
  }

  return ret;
}


void editor_set_frame(frame_hdr_t *frame)
{
  if ( editor_sel_frame == NULL ) {
    frame_geometry_t g;

    editor_cur_frame = frame;
    gtk_label_set_text(editor_window_frame, editor_cur_frame->id);

    g.x = 0;
    g.y = 0;
    g.width = editor_cur_frame->g0.width;
    g.height = editor_cur_frame->g0.height;
    editor_set_window_geometry(&g);
  }

  editor_sel_frame = frame;
}


void editor_remove_frame(frame_hdr_t *frame)
{
  editor_cur_frame = NULL;
  editor_sel_frame = NULL;

  gtk_label_set_text(editor_window_frame, "*NONE*");
  editor_set_window_geometry((frame_geometry_t *) &frame_geometry_null);

  if ( editor_pattern0 != NULL ) {
    editor_pattern0->frame = NULL;
  }

  if ( editor_pattern != NULL ) {
    editor_pattern->frame = NULL;
  }
}


static void editor_changed(void)
{
  pattern_t *pattern;
  int apply = 0;
  int revert = 0;

  if ( editor_pattern0 != NULL ) {
    /* Check for values modification */
    pattern = pattern_alloc("", editor_pattern0->frame, NULL, 0);
    if ( editor_apply_values(pattern) == 0 ) {
      if ( pattern_diff(editor_pattern0, pattern) ) {
	apply = 1;
	revert = 1;
      }
    }
    else {
      revert = 1;
    }

    /* Signal change event */
    if ( editor_event_fn != NULL ) {
      pattern_t *pattern2 = (apply || revert) ? pattern : NULL;
      editor_event_fn(EDITOR_EVENT_MODIFIED, pattern2);
    }

    pattern_free(pattern);
  }

  /* Update buttons sensitivity */
  gtk_widget_set_sensitive(GTK_WIDGET(editor_apply), apply);
  gtk_widget_set_sensitive(GTK_WIDGET(editor_revert), revert);
}


void editor_show_match(int state)
{
  if ( state ) {
    gtk_widget_modify_base(GTK_WIDGET(editor_id), GTK_STATE_NORMAL, &yellow);
  }
  else {
    gtk_widget_modify_base(GTK_WIDGET(editor_id), GTK_STATE_NORMAL, NULL);
  }
}


int editor_show(pattern_t *pattern)
{
  if ( editor_sel_frame == NULL )
    return -1;

  if ( editor_pattern != NULL )
    pattern_free(editor_pattern);

  if ( pattern != NULL )
    editor_pattern = pattern_clone(pattern);
  else
    editor_pattern = pattern_alloc("", editor_sel_frame, NULL, 0);

  editor_pattern0 = pattern;
  editor_set_values(editor_pattern);

  editor_show_match((pattern != NULL) && pattern->state);

  gtk_widget_set_sensitive(editor_box, (pattern != NULL));

  if ( pattern == NULL ) {
    lprintf(editor_error, "");
    gtk_combo_box_set_active(editor_source_type, 0);
  }

  /* Update buttons sensitivity */
  gtk_widget_set_sensitive(GTK_WIDGET(editor_apply), 0);
  gtk_widget_set_sensitive(GTK_WIDGET(editor_revert), 0);

  return 0;
}


static void editor_image_grab_clicked(void)
{
  char *fname0;
  char *fname;
  frame_geometry_t g;

  if ( editor_pattern == NULL )
    return;
  if ( editor_pattern->frame == NULL )
    return;
  if ( ! editor_get_selection(editor_pattern->frame, &g) )
    return;

  fname0 = g_strstrip(strdup((char *) gtk_entry_get_text(editor_image_fname)));

  if ( *fname0 == '\0' ) {
    free(fname0);
    fname0 = NULL;
  }

  /* If not valid file name, create one */
  fname = png_filename(fname0);

  if ( png_save(&(editor_pattern->frame->fb->rgb), &g, fname) == 0 ) {
    pattern_set_source(editor_pattern, PATTERN_TYPE_IMAGE, fname, NULL);
    editor_set_source(editor_pattern);

    if ( (editor_pattern->window.width == 0) ||
	 (editor_pattern->window.height == 0) ) {
      editor_pattern->window = g;
      editor_set_window(editor_pattern);
    }
  }

  if ( fname != NULL )
    free(fname);
  if ( fname0 != NULL )
    free(fname0);
}


static void editor_window_set_clicked(void)
{
  frame_geometry_t g = FRAME_GEOMETRY_NULL;

  if ( editor_sel_frame == NULL )
    return;
  if ( editor_pattern == NULL )
    return;

  if ( editor_get_selection(editor_sel_frame, &g) ) {
    editor_pattern->frame = editor_sel_frame;

    if ( pattern_set_window(editor_pattern, &g) )
      return;

    editor_set_window(editor_pattern);
  }
}


static void editor_apply_clicked(void)
{
  if ( editor_pattern == NULL )
    return;

  if ( editor_apply_values(editor_pattern) == 0 ) {
    if ( editor_event_fn != NULL )
      editor_event_fn(EDITOR_EVENT_UPDATE, editor_pattern);
  }
}


static void editor_revert_clicked(void)
{
  if ( editor_pattern == NULL )
    return;

  pattern_copy(editor_pattern, editor_pattern0);
  editor_set_values(editor_pattern);
}


static GtkWidget *editor_lookup_widget(GtkWindow *window, char *widget_name, char *signal_name)
{
  GtkWidget *widget = lookup_widget(GTK_WIDGET(window), widget_name);

  gtk_signal_connect_object(GTK_OBJECT(widget), signal_name,
			    GTK_SIGNAL_FUNC(editor_changed), NULL);

  return widget;
}


int editor_init(GtkWindow *window, editor_func_t *event_fn)
{
  editor_box = lookup_widget(GTK_WIDGET(window), "edit_box");

  editor_error = GTK_LABEL(lookup_widget(GTK_WIDGET(window), "edit_error"));
  gtk_label_set_use_markup(editor_error, TRUE);
  eprintf_label = editor_error;

  editor_id = GTK_ENTRY(editor_lookup_widget(window, "edit_id", "changed"));

  editor_mode_appear = GTK_TOGGLE_BUTTON(editor_lookup_widget(window, "edit_mode_appear", "toggled"));
  editor_mode_disappear = GTK_TOGGLE_BUTTON(editor_lookup_widget(window, "edit_mode_disappear", "toggled"));
  editor_mode_retrigger = GTK_TOGGLE_BUTTON(editor_lookup_widget(window, "edit_mode_retrigger", "toggled"));
  editor_mode_brief = GTK_TOGGLE_BUTTON(editor_lookup_widget(window, "edit_mode_brief", "toggled"));

  editor_source_type = GTK_COMBO_BOX(editor_lookup_widget(window, "edit_source_type", "changed"));

  editor_image_box = lookup_widget(GTK_WIDGET(window), "edit_image_box");
  editor_image_fname = GTK_ENTRY(editor_lookup_widget(window, "image_fname", "changed"));
  editor_image_size = GTK_LABEL(lookup_widget(GTK_WIDGET(window), "image_size"));
  editor_image_grab = GTK_BUTTON(lookup_widget(GTK_WIDGET(window), "image_grab"));
  gtk_signal_connect_object(GTK_OBJECT(editor_image_grab), "clicked",
			    GTK_SIGNAL_FUNC(editor_image_grab_clicked), NULL);
  editor_image_fuzz = GTK_ENTRY(editor_lookup_widget(window, "image_fuzz", "changed"));
  editor_image_badpixels = GTK_SPIN_BUTTON(editor_lookup_widget(window, "image_badpixels", "changed"));
  editor_image_potential = GTK_SPIN_BUTTON(editor_lookup_widget(window, "image_potential", "changed"));

  editor_text_box = lookup_widget(GTK_WIDGET(window), "edit_text_box");
  editor_text_inverse = GTK_TOGGLE_BUTTON(editor_lookup_widget(window, "text_inverse", "toggled"));
  editor_text_regex = GTK_ENTRY(editor_lookup_widget(window, "text_regex", "changed"));

  editor_window_frame = GTK_LABEL(lookup_widget(GTK_WIDGET(window), "edit_frame"));
  editor_window_width = GTK_SPIN_BUTTON(editor_lookup_widget(window, "edit_width", "changed"));
  editor_window_height = GTK_SPIN_BUTTON(editor_lookup_widget(window, "edit_height", "changed"));
  editor_window_x = GTK_SPIN_BUTTON(editor_lookup_widget(window, "edit_x", "changed"));
  editor_window_y = GTK_SPIN_BUTTON(editor_lookup_widget(window, "edit_y", "changed"));
  editor_window_set = GTK_BUTTON(lookup_widget(GTK_WIDGET(window), "edit_set"));
  gtk_signal_connect_object(GTK_OBJECT(editor_window_set), "clicked",
			    GTK_SIGNAL_FUNC(editor_window_set_clicked), NULL);

  editor_apply = GTK_BUTTON(lookup_widget(GTK_WIDGET(window), "edit_apply"));
  gtk_signal_connect_object(GTK_OBJECT(editor_apply), "clicked",
			    GTK_SIGNAL_FUNC(editor_apply_clicked), NULL);

  editor_revert = GTK_BUTTON(lookup_widget(GTK_WIDGET(window), "edit_revert"));
  gtk_signal_connect_object(GTK_OBJECT(editor_revert), "clicked",
			    GTK_SIGNAL_FUNC(editor_revert_clicked), NULL);

  editor_pattern0 = NULL;
  editor_pattern = NULL;
  editor_cur_frame = NULL;
  editor_sel_frame = NULL;
  editor_event_fn = event_fn;

  /* Init buttons sensitivity */
  gtk_widget_set_sensitive(editor_box, 0);
  gtk_widget_set_sensitive(GTK_WIDGET(editor_apply), 0);
  gtk_widget_set_sensitive(GTK_WIDGET(editor_revert), 0);

  return 0;
}


void editor_done(void)
{
  if ( editor_pattern != NULL ) {
    pattern_free(editor_pattern);
    editor_pattern = NULL;
  }

  editor_pattern0 = NULL;
  editor_cur_frame = NULL;
  editor_sel_frame = NULL;
  editor_event_fn = NULL;

  eprintf_label = NULL;
}
