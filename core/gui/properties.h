/****************************************************************************/
/* TestFarm                                                                 */
/* Graphical User Interface : Test Suite properties                         */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 03-MAR-2005                                                    */
/****************************************************************************/

/*
 * $Revision: 42 $
 * $Date: 2006-06-03 15:30:01 +0200 (sam., 03 juin 2006) $
 */

#ifndef __PROPERTIES_H__
#define __PROPERTIES_H__

#include <gtk/gtk.h>

typedef struct {
  char *system;
  char *dir_name;
  char *tree_name;
  char *operator;
  char *release;

  GtkWidget *window;
} properties_t;

extern properties_t *properties_init(GtkWidget *window);
extern void properties_destroy(properties_t *prop);

extern void properties_set(properties_t *prop, char *system, char *tree_name);
extern void properties_entry(properties_t *prop);
extern void properties_check(properties_t *prop, char *msg, void (*done)(void *arg), void *arg);

#endif /* __PROPERTIES_H__ */
