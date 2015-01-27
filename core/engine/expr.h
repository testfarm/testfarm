/****************************************************************************/
/* TestFarm                                                                 */
/* Test Engine - Trigger expression evaluator                               */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 11-SEP-2003                                                    */
/****************************************************************************/

/* $Revision: 42 $ */
/* $Date: 2006-06-03 15:30:01 +0200 (sam., 03 juin 2006) $ */

#ifndef __EXPR_H__
#define __EXPR_H__

#include <glib.h>

#define EXPR_NOP -1
#define EXPR_AND  0
#define EXPR_OR   1

typedef struct {
  int op;
  char *value;
  void *data;
} expr_item_t;

extern void expr_compile(GList **plist, char *expression);
extern void expr_free(GList *list);

typedef int expr_eval_hdl_t(expr_item_t *);

extern int expr_eval(GList *list, expr_eval_hdl_t *hdl);

#endif /* __EXPR_H__ */
