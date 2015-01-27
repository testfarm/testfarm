/****************************************************************************/
/* TestFarm                                                                 */
/* File validation check                                                    */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 29-JAN-2004                                                    */
/****************************************************************************/

/*
 * $Revision: 42 $
 * $Date: 2006-06-03 15:30:01 +0200 (sam., 03 juin 2006) $
 */

#ifndef __TESTFARM_VALIDATE_H__
#define __TESTFARM_VALIDATE_H__

#include "codegen.h"
#include "md5.h"

extern int validate_max;

typedef struct {
  char *md5sum;
  int level;
  char *date;
  char *operator;
  int updated;
} validate_t;

extern int validate_init(void);
extern void validate_destroy(void);

extern validate_t *validate_alloc(void);
extern void validate_free(validate_t *v);
extern validate_t *validate_object_data(tree_object_t *object);

extern int validate_check(validate_t *v, tree_object_t *object);
extern int validate_update(validate_t *v, tree_object_t *object, int level, char *operator);

extern char *validate_get_id(int level);
extern char *validate_get_icon(int level);

#endif  /* __TESTFARM_VALIDATE_H__ */
