/****************************************************************************/
/* TestFarm RFB Interface                                                   */
/* RFB Input recording                                                      */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 30-MAR-2007                                                    */
/****************************************************************************/

/*
 * $Revision: 793 $
 * $Date: 2007-11-06 22:44:29 +0100 (mar., 06 nov. 2007) $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <malloc.h>
#include <unistd.h>
#include <sys/time.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

#include "filelist.h"

#include "support.h"
#include "utils.h"
#include "frame_ctl_msg.h"
#include "record.h"
#include "display_record.h"


#define DISPLAY_RECORD_MOTION_DELAY 500

static GtkWidget *display_record_box = NULL;
static GtkEntry *display_record_entry = NULL;
static GtkToggleButton *display_record_timing = NULL;
static GtkToggleButton *display_record_motion = NULL;

static GtkToggleButton *display_record_button_play = NULL;
static GtkToggleButton *display_record_button_record = NULL;
static GtkToggleButton *display_record_button_pause = NULL;
static GtkButton *display_record_button_stop = NULL;

typedef enum {
  DISPLAY_RECORD_IDLE=0,
  DISPLAY_RECORD_REC,
  DISPLAY_RECORD_PLAY,
} display_record_operation_t;

static display_record_operation_t display_record_operation = DISPLAY_RECORD_IDLE;
static int display_record_paused = 0;
static char *display_record_fname = NULL;
static FILE *display_record_f = NULL;


/*=======================================================*/
/* Events recording                                      */
/*=======================================================*/

static int display_record_sock = -1;

static unsigned long display_record_pointer_t0 = 0;

static unsigned char display_record_pointer_buttons = 0;
static unsigned int display_record_pointer_x = 0;
static unsigned int display_record_pointer_y = 0;


static void display_record_pointer(unsigned long time)
{
  if ( (display_record_operation == DISPLAY_RECORD_REC) && (! display_record_paused) ) {
    display_record_pointer_t0 = time;

    record_delay();
    record_position(display_record_pointer_x, display_record_pointer_y);
    record_buttons(display_record_pointer_buttons);
  }
}


void display_record_pointer_position(int x, int y, unsigned long time)
{
  display_record_pointer_x = x;
  display_record_pointer_y = y;

  frame_ctl_pointer(display_record_sock, display_record_pointer_buttons, x, y);

  if ( gtk_toggle_button_get_active(display_record_motion) &&
       ((time - display_record_pointer_t0) > DISPLAY_RECORD_MOTION_DELAY) ) {
    display_record_pointer(time);
  }
}


void display_record_pointer_button(unsigned int n, int pressed, unsigned long time)
{
  unsigned char buttons = display_record_pointer_buttons;
  unsigned char mask;

  if ( n < 1 )
    return;
  mask = (1 << (n-1));

  if ( pressed )
    buttons |= mask;
  else
    buttons &= ~mask;

  frame_ctl_pointer(display_record_sock, buttons, display_record_pointer_x, display_record_pointer_y);

  if ( buttons != display_record_pointer_buttons ) {
    display_record_pointer_buttons = buttons;
    display_record_pointer(time);
  }
}


void display_record_pointer_scroll(unsigned char direction)
{
  frame_ctl_scroll(display_record_sock, direction);
  record_scroll(direction);
}


void display_record_key(unsigned long keyval, int pressed)
{
  frame_ctl_key(display_record_sock, pressed, keyval);

  if ( (display_record_operation == DISPLAY_RECORD_REC) && (! display_record_paused) ) {
    record_key(keyval, pressed);
  }
}


void display_record_set_ctl(int sock, capture_cap_t cap)
{
  display_record_sock = sock;

  display_record_stop();

  if ( display_record_box != NULL ) {
    int enable = (sock >= 0) && (cap & CAPTURE_CAP_INPUT);
    gtk_widget_set_sensitive(display_record_box, enable);
  }
}


/*=======================================================*/
/* Record management widgets                             */
/*=======================================================*/

static void display_record_set_async(void)
{
  int async = ! gtk_toggle_button_get_active(display_record_timing);
  record_set_async(async);
}


static void display_record_fname_last(char *fname, int *pnum)
{
  char *p = strrchr(fname, '-');
  int num = 0;

  if ( p != NULL ) {
    *(p++) = '\0';
    num = atoi(p);
  }

  if ( num > *pnum )
    *pnum = num;
}


static void display_record_free_fname(void)
{
  if ( display_record_fname != NULL )
    free(display_record_fname);
  display_record_fname = NULL;
}


static char *display_record_set_fname(void)
{
  char cwd[PATH_MAX];
  int cwdlen;
  char *fname;
  char *basename;
  int size;

  /* Free previously allocated string buffer */
  display_record_free_fname();

  /* Retrieve current working directory */
  if ( getcwd(cwd, sizeof(cwd)) != NULL ) {
    cwdlen = strlen(cwd);
    if ( (cwdlen > 0) && (cwd[cwdlen-1] != G_DIR_SEPARATOR) )
      cwd[cwdlen++] = G_DIR_SEPARATOR;
  }
  else {
    cwdlen = 0;
    cwd[0] = '\0';
  }

  /* Get file name from entry */
  fname = g_strstrip(strdup((char *) gtk_entry_get_text(display_record_entry)));
  if ( fname[0] == '\0' ) {
    free(fname);
    fname = strdup("Record");
  }

  /* Alloc large enough buffer to store the full file path */
  size = cwdlen + strlen(fname) + 32;
  display_record_fname = malloc(size);

  /* Prepend directory name */
  strcpy(display_record_fname, cwd);
  basename = display_record_fname + cwdlen;

  /* Append root file name */
  strcpy(basename, fname);
  free(fname);

  return basename;
}


static void display_record_update_fname(int inc)
{
  char *basename;
  char *suffix;
  GList *list;
  int num = -1;

  /* Set record file name from text entry field */
  basename = display_record_set_fname();

  /* Remove trailing record number */
  suffix = strrchr(basename, '-');
  if ( suffix != NULL )
    *suffix = '\0';
  else
    suffix = basename + strlen(basename);

  /* Get the list of existing backups */
  strcpy(suffix, "(-[0123456789]+)*");
  list = filelist(display_record_fname);

  /* Get the greatest backup number */
  g_list_foreach(list, (GFunc) display_record_fname_last, &num);

  /* Free the list of existing backups */
  g_list_foreach(list, (GFunc) free, NULL);
  g_list_free(list);

  if ( num >= 0 )
    sprintf(suffix, "-%d", num+inc);
  else
    *suffix = '\0';

  gtk_entry_set_text(display_record_entry, basename);
}


static void display_record_set_sensitive(void)
{
  int working = (display_record_operation != DISPLAY_RECORD_IDLE);

  gtk_widget_set_sensitive(GTK_WIDGET(display_record_entry), !working);
  gtk_widget_set_sensitive(GTK_WIDGET(display_record_button_play), !working);
  gtk_widget_set_sensitive(GTK_WIDGET(display_record_button_record), !working);
  gtk_widget_set_sensitive(GTK_WIDGET(display_record_button_pause), working);
  gtk_widget_set_sensitive(GTK_WIDGET(display_record_button_stop), working);
}


static void display_record_stop_playback(void)
{
  /* Stop playback */
  if ( display_record_operation == DISPLAY_RECORD_PLAY ) {
    frame_ctl_source(display_record_sock, "-stop");
  }
}


static void display_record_record_pushed(void)
{
  if ( display_record_operation != DISPLAY_RECORD_REC ) {
    display_record_stop_playback();
    display_record_update_fname(+1);

    display_record_f = fopen(display_record_fname, "w");
    if ( display_record_f == NULL ) {
      gtk_signal_emit_by_name(GTK_OBJECT(display_record_button_stop), "clicked");
      eprintf2("Cannot create record file '%s': %s", display_record_fname, strerror(errno));
      return;
    }

    record_init(display_record_f);
  }

  display_record_operation = DISPLAY_RECORD_REC;
  display_record_paused = 0;

  display_record_set_sensitive();
}


static void display_record_play_pushed(void)
{
  if ( display_record_operation != DISPLAY_RECORD_PLAY ) {
    char *basename = display_record_set_fname();

    if ( basename != NULL ) {
      gtk_entry_set_text(display_record_entry, basename);
      frame_ctl_source(display_record_sock, display_record_fname);
      display_record_operation = DISPLAY_RECORD_PLAY;
      display_record_paused = 0;
    }
    else {
      gtk_toggle_button_set_active(display_record_button_play, FALSE);
      eprintf2("Please enter Record File Name to play");
    }
  }

  display_record_set_sensitive();
}


static void display_record_record_toggled(void)
{
  if ( gtk_toggle_button_get_active(display_record_button_record) )
    display_record_record_pushed();
}


static void display_record_play_toggled(void)
{
  if ( gtk_toggle_button_get_active(display_record_button_play) )
    display_record_play_pushed();
}


static void display_record_pause_toggled(void)
{
  if ( gtk_toggle_button_get_active(display_record_button_pause) ) {
    display_record_paused = 1;
  }
  else {
    display_record_paused = 0;
  }

  if ( display_record_operation == DISPLAY_RECORD_PLAY ) {
    if ( display_record_paused )
      frame_ctl_source(display_record_sock, "-pause");
    else
      frame_ctl_source(display_record_sock, "-resume");
  }
}


void display_record_stop(void)
{
  /* Stop record*/
  if ( display_record_f != NULL ) {
    /* Close record file */
    long size = ftell(display_record_f);
    fclose(display_record_f);
    display_record_f = NULL;

    /* Delete file if it is empty */
    if ( (display_record_fname != NULL) && (size == 0) ) {
      unlink(display_record_fname);
      display_record_free_fname();
    }

    record_init(NULL);
  }

  display_record_operation = DISPLAY_RECORD_IDLE;
  display_record_paused = 0;

  gtk_toggle_button_set_active(display_record_button_play, FALSE);
  gtk_toggle_button_set_active(display_record_button_record, FALSE);
  gtk_toggle_button_set_active(display_record_button_pause, FALSE);

  display_record_set_sensitive();
}


static void display_record_stop_clicked(void)
{
  display_record_stop_playback();
  display_record_stop();
}


void display_record_fkey(unsigned int keyval)
{
  int working = (display_record_operation != DISPLAY_RECORD_IDLE);

  switch ( keyval ) {
  case GDK_F4:
    if ( ! working )
      display_record_record_pushed();
    break;
  case GDK_F5:
    if ( ! working )
      display_record_play_pushed();
    break;
  case GDK_F6:
    if ( working )
      gtk_toggle_button_set_active(display_record_button_pause, !display_record_paused);
    break;
  case GDK_F7:
    display_record_stop_clicked();
    break;
  default:
    break;
  }
}


int display_record_init(GtkWindow *window)
{
  display_record_box = lookup_widget(GTK_WIDGET(window), "record_box");

  display_record_entry = GTK_ENTRY(lookup_widget(GTK_WIDGET(window), "record_entry"));

  display_record_timing = GTK_TOGGLE_BUTTON(lookup_widget(GTK_WIDGET(window), "record_timing"));
  gtk_signal_connect_object(GTK_OBJECT(display_record_timing), "toggled",
			    GTK_SIGNAL_FUNC(display_record_set_async), NULL);

  display_record_motion = GTK_TOGGLE_BUTTON(lookup_widget(GTK_WIDGET(window), "record_motion"));

  display_record_button_play = GTK_TOGGLE_BUTTON(lookup_widget(GTK_WIDGET(window), "record_play"));
  gtk_signal_connect_object(GTK_OBJECT(display_record_button_play), "toggled",
			    GTK_SIGNAL_FUNC(display_record_play_toggled), NULL);

  display_record_button_record = GTK_TOGGLE_BUTTON(lookup_widget(GTK_WIDGET(window), "record_record"));
  gtk_signal_connect_object(GTK_OBJECT(display_record_button_record), "toggled",
			    GTK_SIGNAL_FUNC(display_record_record_toggled), NULL);

  display_record_button_pause = GTK_TOGGLE_BUTTON(lookup_widget(GTK_WIDGET(window), "record_pause"));
  gtk_signal_connect_object(GTK_OBJECT(display_record_button_pause), "toggled",
			    GTK_SIGNAL_FUNC(display_record_pause_toggled), NULL);

  display_record_button_stop = GTK_BUTTON(lookup_widget(GTK_WIDGET(window), "record_stop"));
  gtk_signal_connect_object(GTK_OBJECT(display_record_button_stop), "clicked",
			    GTK_SIGNAL_FUNC(display_record_stop_clicked), NULL);

  display_record_fname = NULL;
  display_record_f = NULL;
  display_record_stop();

  display_record_sock = -1;
  gtk_widget_set_sensitive(display_record_box, 0);

  record_init(NULL);
  display_record_set_async();

  return 0;
}


void display_record_done(void)
{
  display_record_stop();
  display_record_free_fname();
  display_record_sock = -1;
}
