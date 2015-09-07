/****************************************************************************/
/* TestFarm                                                                 */
/* Shell script interpreter - Argument list builder                         */
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

#ifndef __SHELL_ARGV_H__
#define __SHELL_ARGV_H__

#include <stdio.h>

typedef struct {
  char *s_pid;
  char *s_argc;
  char *s_buf;
  int s_len;
  int argc;
  char **argv;
} shell_argv_t;

extern shell_argv_t *shell_argv_alloc(char *cmd, shell_argv_t *ref_argv);
extern shell_argv_t *shell_argv_allocv(int argc, char **argv, shell_argv_t *ref_argv);
extern void shell_argv_free(shell_argv_t *argv);
extern char *shell_argv_pack(shell_argv_t *argv);
extern int shell_argv_fprintf(FILE *f, shell_argv_t *argv, int offset);

#endif /* __SHELL_ARGV_H__ */
