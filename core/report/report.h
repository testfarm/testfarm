/****************************************************************************/
/* TestFarm                                                                 */
/* Test Report generator                                                    */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 28-MAR-2001                                                    */
/****************************************************************************/

/*
 * $Revision: 371 $
 * $Date: 2007-02-26 18:58:01 +0100 (lun., 26 févr. 2007) $
 */

#ifndef __TESTFARM_REPORT_H__
#define __TESTFARM_REPORT_H__

#include "report_config.h"

extern char *report_elapsed(long dt_ms);

extern int report_xslt(char *out_name, char *signature,
                       report_config_t *rc, char *html_dir);

#endif /* __TESTFARM_REPORT_H__ */
