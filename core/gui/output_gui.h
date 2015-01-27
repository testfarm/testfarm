/****************************************************************************/
/* TestFarm                                                                 */
/* Graphical User Interface : Stdin/Stdout/Stderr display                   */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 07-MAR-2005                                                    */
/****************************************************************************/

/*
 * $Revision: 765 $
 * $Date: 2007-10-05 12:53:10 +0200 (ven., 05 oct. 2007) $
 */

#ifndef __OUTPUT_GUI_H__
#define __OUTPUT_GUI_H__

#include <gtk/gtk.h>
#include "output.h"

typedef struct {
  GtkWidget *window;
  GtkWidget *label;
  GtkWidget *clist;
  unsigned long depth;
  output_t *out;
} output_gui_t;

extern output_gui_t *output_gui_init(GtkWidget *window);
extern void output_gui_destroy(output_gui_t *og);

extern void output_gui_clear(output_gui_t *og);
extern void output_gui_set(output_gui_t *og, output_t *out);

extern void output_gui_show(output_gui_t *og, char *name, int is_root);
extern void output_gui_feed(output_gui_t *og, int channel, char *str);
extern void output_gui_scrolldown(output_gui_t *og);

#endif /* __OUTPUT_GUI_H__ */
