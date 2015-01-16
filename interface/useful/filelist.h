/****************************************************************************/
/* TestFarm                                                                 */
/* Get a File List from a file path regular expression                      */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 11-JUN-2004                                                    */
/****************************************************************************/

/*
 * $Revision: 728 $
 * $Date: 2007-09-22 10:20:01 +0200 (sam., 22 sept. 2007) $
 */

#ifndef __FILELIST_H__
#define __FILELIST_H__

#include <glib.h>

extern GList *filelist(char *path);
extern void filelist_free(GList *list);

#endif /* __FILELIST_H__ */
