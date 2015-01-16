/****************************************************************************/
/* TestFarm                                                                 */
/* Interface logical link management - Text data source                     */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 06-APR-2007                                                    */
/****************************************************************************/

/*
 * $Revision: 471 $
 * $Date: 2007-04-11 16:30:42 +0200 (mer., 11 avril 2007) $
 */

#ifndef __INTERFACE_LINK_TXT_H__
#define __INTERFACE_LINK_TXT_H__

#include <glib.h>

typedef struct {
  GList *sources;
} link_src_txt_t;

extern int link_txt_init(void);
extern int link_txt_attached(link_src_txt_t *src_txt, char *id);

#endif /* __INTERFACE_LINK_TXT_H__ */
