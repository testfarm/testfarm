/****************************************************************************/
/* TestFarm                                                                 */
/* ATS GUI: System Management GUI                                           */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 04-MAR-2004                                                    */
/****************************************************************************/

/*
 * $Revision: 42 $
 * $Date: 2006-06-03 15:30:01 +0200 (sam., 03 juin 2006) $
 */

#ifndef __SYSTEM_GUI_H__
#define __SYSTEM_GUI_H__

#include <gtk/gtk.h>
#include "child.h"

typedef void system_gui_handler_t(void *arg, char *action);

typedef struct {
  child_t *child;
  int enable;
  int ctl_fd;
  int stdout;
  int stdout_tag;
  system_gui_handler_t *handler;
  void *arg;
} system_gui_manual_t;

typedef struct {
  char *config_name;
  GtkWidget *label;
  GtkWidget *box;
  GtkWidget *manual_start;
  GtkWidget *manual_stop;
  system_gui_manual_t manual;
  int stderr[2];
  int stderr_tag;
} system_gui_t;

extern system_gui_t *system_gui_init(GtkWidget *window);
extern void system_gui_destroy(system_gui_t *sg);
extern void system_gui_enable(system_gui_t *sg, int state);
extern void system_gui_set(system_gui_t *sg, char *config_name);
extern void system_gui_manual_enable(system_gui_t *sg, int state);
extern void system_gui_manual_handler(system_gui_t *sg, system_gui_handler_t *handler, void *arg);

#endif /* __SYSTEM_GUI_H__ */
