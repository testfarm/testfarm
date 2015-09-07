/****************************************************************************/
/* TestFarm                                                                 */
/* Shell script interpreter - Alias resolution manager                      */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 26-AUG-1999                                                    */
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

#ifndef __SHELL_ALIAS_H__
#define __SHELL_ALIAS_H__

#include "shell_argv.h"

typedef struct {
  char *keyword;
  int argc;
  char **argv;
} shell_alias_item_t;


extern shell_alias_item_t *shell_alias_item_alloc(char *keyword, int argc, char **argv);
extern void shell_alias_item_free(shell_alias_item_t *item);
extern void shell_alias_item_show(shell_alias_item_t *item, char *hdr);



typedef struct {
  shell_alias_item_t **tab;
  int count;
} shell_alias_t;


extern shell_alias_t *shell_alias_init(shell_alias_t *inherit);
extern void shell_alias_done(shell_alias_t *alias);
extern shell_alias_item_t *shell_alias_retrieve(shell_alias_t *alias, char *keyword);
extern shell_alias_item_t *shell_alias_add(shell_alias_t *alias, char *keyword, int argc, char **argv);
extern void shell_alias_resolve(shell_alias_t *alias, shell_argv_t *argv);
extern void shell_alias_show(shell_alias_t *alias, char *hdr);

#endif /* __SHELL_ALIAS_H__ */
