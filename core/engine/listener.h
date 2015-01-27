/****************************************************************************/
/* TestFarm                                                                 */
/* Test Engine - Incoming events listener                                   */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 11-SEP-2003 (split from trig.h)                                */
/****************************************************************************/

/* $Revision: 42 $ */
/* $Date: 2006-06-03 15:30:01 +0200 (sam., 03 juin 2006) $ */

#ifndef __LISTENER_H__
#define __LISTENER_H__

#include <glib.h>

typedef struct {
  char *str;
  GList *list;
} listener_t;
#if 0
typedef GList listener_t;
#endif

extern listener_t *listener_new(int argc, char **argv);
extern void listener_destroy(listener_t *listener);
extern char *listener_str(listener_t *listener);
extern char *listener_raised(listener_t *listener);
extern int listener_check(listener_t *listener);

#endif /* __LISTENER_H__ */
