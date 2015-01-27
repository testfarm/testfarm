/****************************************************************************/
/* TestFarm                                                                 */
/* Graphical User Interface : Test Tree execution                           */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 21-JUL-2005                                                    */
/****************************************************************************/

/*
 * $Revision: 772 $
 * $Date: 2007-10-11 15:58:52 +0200 (jeu., 11 oct. 2007) $
 */

#ifndef __TREE_RUN_H
#define __TREE_RUN_H

#include <gtk/gtk.h>

#include "codegen.h"
#include "properties.h"
#include "perl_run.h"
#include "report_progress.h"
#include "output_gui.h"
#include "tree_gui.h"
#include "tcpsrv.h"


enum {
  stRESET=0,
  stSPAWN,
  stWAIT,
  stRUN,
  stINPUT,
  stFINISHED
};


typedef struct {
  tree_gui_t *gui;

  tree_t *tree;
  tree_object_t *current_object;
  int debug;
  int state;
  int stepping;
  unsigned long conf;
  properties_t *prop;
  perl_run_t *pg;

  report_progress_t progress;
  srv_t srv;
} tree_run_t;

extern tree_run_t *tree_run_init(GtkWidget *window);
extern void tree_run_destroy(tree_run_t *tg);

extern tree_t *tree_run_load(tree_run_t *tg, char *filename);
extern int tree_run_loaded(tree_run_t *tg);

/* Actions */
extern void tree_run_reset(tree_run_t *tg);
extern void tree_run_abort(tree_run_t *tg);
extern void tree_run_start(tree_run_t *tg, int step);
extern void tree_run_input(tree_run_t *tg, int verdict, char *text);
extern void tree_run_manual_action(tree_run_t *tg, char *action);
extern int tree_run_jump(tree_run_t *tg, unsigned long key);

/* Modes switch */
extern int tree_run_set_debug(tree_run_t *tg, int state);
extern int tree_run_set_brkpt(tree_run_t *tg, unsigned long key, int state);
extern int tree_run_set_skip(tree_run_t *tg, unsigned long key, int state);

#endif /* __TREE_RUN_H */
