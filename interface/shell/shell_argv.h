/****************************************************************************/
/* TestFarm                                                                 */
/* Shell script interpreter - Argument list builder                         */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 26-AUG-1999                                                    */
/****************************************************************************/

/*
 * $Revision: 29 $
 * $Date: 2006-06-03 10:39:29 +0200 (sam., 03 juin 2006) $
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
