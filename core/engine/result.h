/****************************************************************************/
/* TestFarm                                                                 */
/* Test Engine - Result output management                                   */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 17-AUG-1999                                                    */
/****************************************************************************/

/*
 * $Revision: 42 $
 * $Date: 2006-06-03 15:30:01 +0200 (sam., 03 juin 2006) $
 */

#ifndef __RESULT_H__
#define __RESULT_H__

#include "periph.h"

extern int result_init(void);
extern void result_done(void);
extern int result_puts(char *msg);

extern int result_global_set(char *fname);
extern char *result_global_get(void);

extern int result_local_open(void);
extern void result_local_close(void);
extern void result_local_clear(void);
extern int result_local_set(char *fname);
extern char *result_local_get(void);

extern char *result_header_periph(periph_item_t *item);
extern int result_dump_periph(periph_item_t *item, char **buf);

extern char *result_header_engine(char *tag);
extern int result_dump_engine(char *tag, char *fmt, ...);

extern void result_global_set_case(char *id);

#endif /* __RESULT_H__ */
