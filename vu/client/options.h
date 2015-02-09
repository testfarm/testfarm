/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* Client options                                                           */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 24-JAN-2007                                                    */
/****************************************************************************/

/*
 * $Revision: 917 $
 * $Date: 2008-01-25 16:04:27 +0100 (ven., 25 janv. 2008) $
 */

#ifndef __TVU_OPTIONS_H__
#define __TVU_OPTIONS_H__

#include <glib.h>

extern int opt_debug;
extern int opt_shared;
extern char *opt_name;
extern int opt_rotate;
extern GList *opt_filters;
extern int opt_quad;    /* Quad-core processor */

extern int options_parse(int *pargc, char ***pargv);

#endif /* __TVU_OPTIONS_H__ */
