/****************************************************************************/
/* TestFarm                                                                 */
/* Test Engine - Incoming events triggering                                 */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 11-AUG-2000                                                    */
/****************************************************************************/

/*
 * $Revision: 1088 $
 * $Date: 2009-10-30 15:24:46 +0100 (ven., 30 oct. 2009) $
 */

#ifndef __TRIG_H__
#define __TRIG_H__

#include <pcreposix.h>

#include "shell.h"
#include "periph_item.h"

#define TAG_TRIG  "TRIG   "

typedef struct {
  char *id;
  int state;
  char *info;
  periph_item_t *periph;
  char *regex;
  regex_t pattern;
} trig_t;

extern int trig_def(shell_t *shell, char *id, periph_item_t *periph, char **argv, int argc);
extern int trig_undef(char **argv, int argc);
extern int trig_clear(char **argv, int argc);
extern int trig_update(periph_item_t *periph, char *buf);

extern int trig_count(char *id);
extern int trig_info(char *id, char **info);

extern trig_t *trig_retrieve(char *id);

#endif /* __TRIG_H__ */
