/****************************************************************************/
/* TestFarm                                                                 */
/* Test Engine - Result output management                                   */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 17-AUG-1999                                                    */
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

#ifndef __TESTFARM_RESULT_H__
#define __TESTFARM_RESULT_H__

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

#endif /* __TESTFARM_RESULT_H__ */
