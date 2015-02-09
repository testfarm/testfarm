/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* External Agent management                                                */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 08-DEC-2007                                                    */
/****************************************************************************/

/*
 * $Revision: 844 $
 * $Date: 2007-12-08 17:26:33 +0100 (sam., 08 déc. 2007) $
 */

#ifndef __TVU_AGENT_H__
#define __TVU_AGENT_H__

typedef void agent_callback_t(void *ctx, char *buf, int size);

#ifndef __TVU_AGENT_C__
typedef struct {
  // Opaque descriptor
} agent_t;
#endif

extern agent_t *agent_create(char *argv[], agent_callback_t *callback, void *ctx);
extern void agent_destroy(agent_t *agent);

extern int agent_request(agent_t *agent, char *buf, int size);

#endif /* __TVU_AGENT_H__ */

