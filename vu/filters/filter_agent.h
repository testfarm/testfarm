/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* Filter Agent program - Standard functions                                */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 02-JAN-2008                                                    */
/****************************************************************************/

/*
 * $Revision: 888 $
 * $Date: 2008-01-07 15:30:49 +0100 (lun., 07 janv. 2008) $
 */

#ifndef __TVU_FILTER_AGENT_H__
#define __TVU_FILTER_AGENT_H__

#include <glib.h>
#include "frame_buf.h"

/* Character strings exported by filter agent process:
   filter(s) identification and description */  
extern const char *filter_agent_name;
extern const char *filter_agent_desc;

/* Exported by filter agent process,
   called by filter agent program body as request functions */
extern int filter_agent_init(int argc, char *argv[]);
extern void filter_agent_add(unsigned long num, char *class_id, GList *options);
extern void filter_agent_remove(unsigned long num);
extern void filter_agent_show(unsigned long num);
extern void filter_agent_apply(unsigned long num, frame_buf_t *fb);

/* Exported by filter agent program body,
   called by filter agent process as response functions */
extern void filter_agent_register(char *class_id);
extern void filter_agent_added(unsigned long num);
extern void filter_agent_removed(unsigned long num);
extern void filter_agent_shown(unsigned long num, char *msg);
extern void filter_agent_applied(unsigned long num);

#endif /* __TVU_FILTER_AGENT_H__ */
