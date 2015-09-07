/****************************************************************************/
/* TestFarm                                                                 */
/* Test Engine - Nibble Allocation                                          */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 18-NOV-1999                                                    */
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

#ifndef __TESTFARM_ENGINE_NIBBLE_H__
#define __TESTFARM_ENGINE_NIBBLE_H__

#define NIBBLE_SIZE 80

typedef struct {
  int size;
  char *buf;
} nibble_t;

extern nibble_t *nibble_realloc(nibble_t *nibble, int size);
extern void nibble_free(nibble_t *nibble);

#endif /* __TESTFARM_ENGINE_NIBBLE_H__ */
