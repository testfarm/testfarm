/****************************************************************************/
/* TestFarm                                                                 */
/* Test Suite code generator : test case criticity identifiers              */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 07-DEC-2001                                                    */
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

#ifndef __TESTFARM_CODEGEN_CRITICITY_H
#define __TESTFARM_CODEGEN_CRITICITY_H

#define CRITICITY_UNKNOWN -1
#define CRITICITY_NONE 0

typedef int criticity_t;

extern int criticity_init(void);
extern void criticity_destroy(void);

extern char *criticity_id(criticity_t criticity);
extern char *criticity_color(criticity_t criticity);
extern criticity_t criticity_level(char *id);

#endif /* __TESTFARM_CODEGEN_CRITICITY_H */
