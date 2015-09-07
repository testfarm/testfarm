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

#ifndef __TESTFARM_PROPERTIES_H__
#define __TESTFARM_PROPERTIES_H__

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

#endif /* __TESTFARM_PROPERTIES_H__ */
