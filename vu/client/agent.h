/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* External Agent management                                                */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 08-DEC-2007                                                    */
/****************************************************************************/

/*
    This file is part of TestFarm,
    the Test Automation Tool for Embedded Software.
    Please visit http://www.testfarm.org.

    TestFarm is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    TestFarm is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with TestFarm.  If not, see <http://www.gnu.org/licenses/>.
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

