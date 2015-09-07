/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* Input event recorder                                                     */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 01-APR-2007                                                    */
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

#ifndef __TVU_RECORD_H__
#define  __TVU_RECORD_H__

extern void record_init(FILE *f);
extern int record_set_async(int async);

extern void record_date(void);
extern void record_delay(void);

extern void record_key(unsigned long keyval, unsigned char down);
extern int record_position(int x, int y);
extern void record_buttons(unsigned char buttons);
extern void record_scroll(unsigned char direction);

#endif /*  __TVU_RECORD_H__ */
