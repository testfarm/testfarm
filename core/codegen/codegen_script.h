/****************************************************************************/
/* TestFarm                                                                 */
/* Test Suite code generator : test scripts                                 */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 12-APR-2000                                                    */
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

#ifndef __TESTFARM_CODEGEN_SCRIPT_H
#define __TESTFARM_CODEGEN_SCRIPT_H

typedef struct {
  char *name;
  char *wizname;
  char *params;
  char *package;
} script_t;


#define SCRIPT_PERL_SUFFIX ".pm"
#define SCRIPT_WIZ_SUFFIX ".wiz"

extern script_t *script_new(char *cmdline);
extern void script_destroy(script_t *script);
extern void script_show(script_t *script, FILE *f);
extern int script_check_wiz(script_t *script);

#endif /* __TESTFARM_CODEGEN_SCRIPT_H */
