/****************************************************************************/
/* TestFarm Core                                                            */
/* Test Progress dump                                                       */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 02-OCT-2007                                                    */
/****************************************************************************/

/*
 * $Revision: 766 $
 * $Date: 2007-10-05 12:54:23 +0200 (ven., 05 oct. 2007) $
 */

#ifndef __REPORT_PROGRESS_H__
#define __REPORT_PROGRESS_H__

#include <glib.h>

typedef struct {
  guint timeout_tag;
  int value;
  char *msg;
} report_progress_t;

extern void report_progress_init(report_progress_t *rp);

extern void report_progress_msg(report_progress_t *rp, char *msg);
extern void report_progress_value(report_progress_t *rp, int value);
extern void report_progress_clear(report_progress_t *rp);

#endif /* __REPORT_PROGRESS_H__ */
