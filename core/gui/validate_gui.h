/****************************************************************************/
/* TestFarm                                                                 */
/* Graphical User Interface : File validation user interface                */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 16-APR-2004                                                    */
/****************************************************************************/

/*
 * $Revision: 299 $
 * $Date: 2006-11-26 14:54:03 +0100 (dim., 26 nov. 2006) $
 */

#ifndef __ATS_VALIDATE_GUI_H__
#define __ATS_VALIDATE_GUI_H__

#include <gtk/gtk.h>
#include "codegen.h"
#include "properties.h"
#include "validate.h"

typedef struct {
  properties_t *prop;
  GtkWidget *w_expander;
  GtkWidget *w_box;
  GtkWidget *w_icon;
  GtkLabel *w_script;
  GtkLabel *w_last;
  GtkWidget *w_script_modified;
  GtkWidget *w_menu;
  GtkWidget *w_updated;

  tree_object_t *selected;
  tree_object_t *checked;
  int validated;
  int level;
} validate_gui_t;


extern validate_gui_t *validate_gui_init(GtkWidget *window, properties_t *prop);
extern void validate_gui_destroy(validate_gui_t *vg);

extern void validate_gui_select(validate_gui_t *vg, tree_object_t *object);

#endif  /* __ATS_VALIDATE_GUI_H__ */
