/****************************************************************************/
/* TestFarm                                                                 */
/* Test Suite User Interface: System Services and Manual User Interface     */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 04-MAR-2004                                                    */
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
#include <errno.h>
#include <fcntl.h>
#include <gtk/gtk.h>

#include "child.h"
#include "support.h"
#include "xpm_gui.h"
#include "error_gui.h"
#include "system_gui.h"



/*===========================================*/
/* System Services management                */
/*===========================================*/

static int system_gui_service(system_gui_t *sg, char *action, int wait, int stderr_fd)
{
  char *argv[] = {"testfarm-service", action, sg->config_name, NULL};
  child_t *child;
  int status = 0;

  if ( sg->config_name == NULL )
    return -1;

  child = child_spawn(argv, -1 /* stdin */, -1 /* stdout */, stderr_fd,
                      (child_handler_t *) NULL, NULL);

  if ( child == NULL ) {
    fprintf(stderr, "Could not run System Services starter '%s'\n", argv[0]);
    return -2;
  }

  if ( wait )
    child_waitpid(child, &status);

  return status;
}


static void system_gui_service_start(system_gui_t *sg)
{
  system_gui_service(sg, "start", 0, sg->stderr[1]);
}


static void system_gui_service_stop(system_gui_t *sg)
{
  system_gui_service(sg, "stop", 0, sg->stderr[1]);
}


/*===========================================*/
/* Manual User Interface management          */
/*===========================================*/

static void system_gui_manual_close(system_gui_manual_t *manual)
{
  if ( manual->stdout_tag >= 0 )
    gdk_input_remove(manual->stdout_tag);
  manual->stdout_tag = -1;

  if ( manual->ctl_fd >= 0 )
    close(manual->ctl_fd);
  manual->ctl_fd = -1;

  if ( manual->stdout >= 0 )
    close(manual->stdout);
  manual->stdout = -1;
}


static void system_gui_manual_completed(system_gui_t *sg)
{
  system_gui_manual_close(&(sg->manual));

  gtk_widget_set_sensitive(sg->manual_start, 1);
  gtk_widget_set_sensitive(sg->manual_stop, 0);
}


static void system_gui_stderr(system_gui_t *sg, int fd, GdkInputCondition condition)
{
  char c;
  int status;

  read(fd, &c, 1);

  /* Manual User Interface process termination */
  if ( c == 0 ) {
    read(fd, &status, sizeof(status));
    system_gui_manual_completed(sg);
  }

  /* Error message */
  else {
    int size = 80;
    char *buf = malloc(size);
    int len = 0;
    int ret;

    buf[len++] = c;
    while ( (ret = read(fd, &c, 1)) == 1 ) {
      if ( c == '\n' )
        break;

      if ( (len+2) >= size ) {
        size += 80;
        buf = realloc(buf, size);
      }

      buf[len++] = c;

      /* Fold the line after a colon */
      if ( c == ':' )
        buf[len++] = '\n';
    }

    buf[len] = '\0';

    eprintf(buf);

    free(buf);
  }
}


static void system_gui_manual_terminated(int status, system_gui_t *sg)
{
  char c = 0;

  sg->manual.child = NULL;

  write(sg->stderr[1], &c, 1);
  write(sg->stderr[1], &status, sizeof(status));
}


static void system_gui_manual_stdout(system_gui_manual_t *manual,
                                     int fd, GdkInputCondition condition)
{
  char *buf = NULL;
  int size = 0;
  int len = 0;
  char c;
  int ret;

  /* Get action from Manual User Interface */
  while ( (ret = read(fd, &c, 1)) == 1 ) {
    if ( c == '\n' )
      break;

    if ( (len+1) >= size ) {
      size += 80;
      buf = realloc(buf, size);
    }

    buf[len++] = c;

    /* Fold the line after a colon */
    if ( c == ':' )
      buf[len++] = '\n';
  }

  if ( buf == NULL )
    return;
  buf[len] = '\0';

  /* Call action handler */
  if ( manual->handler != NULL )
    manual->handler(manual->arg, buf);

  free(buf);
}


static void system_gui_manual_start(system_gui_t *sg)
{
  system_gui_manual_t *manual = &(sg->manual);
  char ctl_opt[10];
  char *argv[] = {"testfarm-manual-interface", ctl_opt, sg->config_name, NULL};
  int ctl_pipe[2];
  int stdout_pipe[2];

  if ( sg->config_name == NULL )
    return;
  if ( manual->child != NULL )
    return;

  /* Setup control pipes */
  system_gui_manual_close(manual);

  if ( pipe(ctl_pipe) ) {
    fprintf(stderr, "Could not create Manual User Interface control pipe: %s\n", strerror(errno));
    return;
  }
  if ( pipe(stdout_pipe) ) {
    fprintf(stderr, "Could not create Manual User Interface stdout pipe: %s\n", strerror(errno));
    close(ctl_pipe[0]);
    close(ctl_pipe[1]);
    return;
  }

  fcntl(ctl_pipe[0], F_SETFD, 0);             /* Disable close-on-exec on read endpoint */
  fcntl(ctl_pipe[1], F_SETFD, FD_CLOEXEC);    /* Enable close-on-exec on write endpoint */
  manual->ctl_fd = ctl_pipe[1];
  snprintf(ctl_opt, sizeof(ctl_opt), "-c%d", ctl_pipe[0]);

  fcntl(stdout_pipe[0], F_SETFD, FD_CLOEXEC); /* Enable close-on-exec on read endpoint */
  fcntl(stdout_pipe[1], F_SETFD, 0);          /* Disable close-on-exec on write endpoint */
  manual->stdout = stdout_pipe[0];

  /* Launch Manual User Interface process */
  manual->child = child_spawn(argv, -1, stdout_pipe[1], sg->stderr[1],
                              (child_handler_t *) system_gui_manual_terminated, sg);

  /* Close useless pipe endpoints */
  close(ctl_pipe[0]);
  close(stdout_pipe[1]);

  if ( manual->child == NULL ) {
    system_gui_manual_close(manual);
    fprintf(stderr, "Could not run Manual User Interface '%s'\n", argv[0]);
    return;
  }

  /* Setup stdout handling */
  manual->stdout_tag = gdk_input_add(manual->stdout, GDK_INPUT_READ,
                                     (GdkInputFunction) system_gui_manual_stdout, manual);

  /* Setup Manual User Interface sensitivity */
  system_gui_manual_enable(sg, manual->enable);

  gtk_widget_set_sensitive(sg->manual_start, 0);
  gtk_widget_set_sensitive(sg->manual_stop, 1);
}


static void system_gui_manual_stop(system_gui_t *sg)
{
  child_terminate(sg->manual.child);
  sg->manual.child = NULL;
  system_gui_manual_completed(sg);
}


static void system_gui_manual_init(system_gui_manual_t *manual)
{
  manual->child = NULL;
  manual->enable = 0;
  manual->ctl_fd = -1;
  manual->stdout = -1;
  manual->stdout_tag = -1;
  manual->handler = NULL;
  manual->arg = NULL;
}


void system_gui_manual_enable(system_gui_t *sg, int state)
{
  char *buf = state ? "+\n" : "-\n";

  if ( sg == NULL )
    return;

  sg->manual.enable = state;

  if ( sg->manual.ctl_fd >= 0 )
    write(sg->manual.ctl_fd, buf, 2);
}


void system_gui_manual_handler(system_gui_t *sg, system_gui_handler_t *handler, void *arg)
{
  sg->manual.handler = handler;
  sg->manual.arg = arg;
}


/*===========================================*/
/* General System management                 */
/*===========================================*/

system_gui_t *system_gui_init(GtkWidget *window)
{
  system_gui_t *sg;
  GtkWidget *w;

  /* Alloc system management descriptor */
  sg = (system_gui_t *) malloc(sizeof(system_gui_t));
  sg->config_name = NULL;
  system_gui_manual_init(&(sg->manual));

  /* Setup error handling */
  sg->stderr[0] = -1;
  sg->stderr[1] = -1;
  sg->stderr_tag = -1;

  if ( pipe(sg->stderr) ) {
    fprintf(stderr, "Could not create stderr pipe: %s\n", strerror(errno));
  }
  else {
    fcntl(sg->stderr[0], F_SETFD, FD_CLOEXEC);  /* Enable close-on-exec on read endpoint */
    fcntl(sg->stderr[1], F_SETFD, 0);           /* Disable close-on-exec on write endpoint */

    sg->stderr_tag = gdk_input_add(sg->stderr[0], GDK_INPUT_READ,
                                   (GdkInputFunction) system_gui_stderr, sg);
  }

  /* Setup system management widgets */
  sg->label = lookup_widget(window, "system_label");
  sg->box = lookup_widget(window, "system_menu");

  /* Setup System Services management widgets */
  w = lookup_widget(window, "system_start_services");
  gtk_signal_connect_object(GTK_OBJECT(w), "activate",
                            GTK_SIGNAL_FUNC(system_gui_service_start), (gpointer) sg);

  w = lookup_widget(window, "system_stop_services");
  gtk_signal_connect_object(GTK_OBJECT(w), "activate",
                            GTK_SIGNAL_FUNC(system_gui_service_stop), (gpointer) sg);

  /* Setup Manual User Interface management widgets */
  sg->manual_start = w = lookup_widget(window, "system_start_manual");
  gtk_signal_connect_object(GTK_OBJECT(w), "activate",
                            GTK_SIGNAL_FUNC(system_gui_manual_start), (gpointer) sg);

  sg->manual_stop = w = lookup_widget(window, "system_stop_manual");
  gtk_signal_connect_object(GTK_OBJECT(w), "activate",
                            GTK_SIGNAL_FUNC(system_gui_manual_stop), (gpointer) sg);

  /* Setup initial widget sensitivity */
  system_gui_enable(sg, 0);
  gtk_widget_set_sensitive(sg->manual_stop, 0);

  return sg;
}


void system_gui_destroy(system_gui_t *sg)
{
  if ( sg == NULL )
    return;

  /* Close error handling */
  if ( sg->stderr_tag >= 0 )
    gdk_input_remove(sg->stderr_tag);
  sg->stderr_tag = -1;

  if ( sg->stderr[0] >= 0 )
    close(sg->stderr[0]);
  sg->stderr[0] = -1;

  if ( sg->stderr[1] >= 0 )
    close(sg->stderr[1]);
  sg->stderr[1] = -1;

  /* Abort Manual User Interface */
  system_gui_manual_handler(sg, NULL, NULL);
  system_gui_manual_stop(sg);

  /* Free all other resources */
  system_gui_set(sg, NULL);
  free(sg);
}


void system_gui_enable(system_gui_t *sg, int state)
{
  gtk_widget_set_sensitive(sg->box, state);
}


void system_gui_set(system_gui_t *sg, char *config_name)
{
  if ( config_name == NULL ) {
    gtk_label_set_text(GTK_LABEL(sg->label), "(None)");
    system_gui_enable(sg, 0);

    if ( sg->config_name != NULL ) {
      system_gui_service(sg, "stop", 1, -1);
      free(sg->config_name);
    }

    sg->config_name = NULL;
  }
  else {
    gtk_label_set_text(GTK_LABEL(sg->label), config_name);
    system_gui_enable(sg, 1);

    if ( (sg->config_name == NULL) || (strcmp(sg->config_name, config_name) != 0) ) {
      if ( sg->config_name != NULL ) {
        system_gui_service(sg, "stop", 1, -1);
        free(sg->config_name);
      }

      sg->config_name = strdup(config_name);
      system_gui_service(sg, "start", 0, -1);
    }
  }
}
