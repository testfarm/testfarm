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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "useful.h"
#include "shell_argv.h"


static char *argv_reference(char *s, shell_argv_t *ref_argv)
{
  char buf[16];
  char *defvalue;

  /* No reference arguments */
  if ( ref_argv == NULL )
    return s;

  /* Resolve references to arguments list */
  if ( *s != '$' )
    return s;

  /* Find a default value specifier */
  if ( (defvalue = strchr(++s, ':')) != NULL )
    *(defvalue++) = NUL;

  /* $$ returns the current pid */
  if ( strcmp(s, "$") == 0 ) {
    if ( ref_argv->s_pid == NULL ) {
      sprintf(buf, "%d", (int) getpid());
      ref_argv->s_pid = strdup(buf);
    }
    s = ref_argv->s_pid;
  }

  /* $# returns argc */
  if ( strcmp(s, "#") == 0 ) {
    if ( ref_argv->s_argc == NULL ) {
      sprintf(buf, "%d", ref_argv->argc);
      ref_argv->s_argc = strdup(buf);
    }
    s = ref_argv->s_argc;
  }

  /* $n returns the corresponding argument (if any) */
  else {
    int num = strtol(s, (char **) NULL, 0);

    if ( num < ref_argv->argc ) {
      /* If reference is good, replace with corresponding argument */
      s = ref_argv->argv[num];
    }
    else {
      /* If reference is illegal, replace with default value (if any) */
      if ( defvalue != NULL )
        s = defvalue;
      else
        s = "";
    }
  }

  return s;
}


shell_argv_t *shell_argv_allocv(int cmd_argc, char **cmd_argv, shell_argv_t *ref_argv)
{
  shell_argv_t *argv;

  /* Alloc argv descriptor */
  argv = (shell_argv_t *) malloc(sizeof(shell_argv_t));
  argv->s_pid = NULL;
  argv->s_argc = NULL;
  argv->s_buf = NULL;
  argv->s_len = 0;

  /* Alloc arguments table */
  argv->argc = cmd_argc;
  argv->argv = (char **) malloc(sizeof(char *) * (cmd_argc+1));

  /* Build arguments table content */
  if ( cmd_argc > 0 ) {
    int size;
    int i;

    /* Alloc arguments buffer */
    size = 0;
    for (i = 0; i < cmd_argc; i++)
      size += strlen(argv_reference(cmd_argv[i], ref_argv)) + 1;
    argv->s_buf = (char *) malloc(size);
    argv->s_len = size;

    /* Duplicate argument list */
    size = 0;
    for (i = 0; i < cmd_argc; i++) {
      char *str = argv_reference(cmd_argv[i], ref_argv);
      int len = strlen(str);

      argv->argv[i] = strcpy(argv->s_buf + size, str);
      size += len + 1;
    }
  }

  /* Terminate table with null */
  argv->argv[cmd_argc] = NULL;

  return argv;
}


shell_argv_t *shell_argv_alloc(char *cmd, shell_argv_t *ref_argv)
{
  shell_argv_t *argv;
  int c;
  char **v;
  char *s;

  /* Alloc argv descriptor */
  argv = (shell_argv_t *) malloc(sizeof(shell_argv_t));
  argv->s_pid = NULL;
  argv->s_argc = NULL;
  argv->s_len = strlen(cmd) + 1;
  argv->s_buf = (char *) malloc(argv->s_len);
  memcpy(argv->s_buf, cmd, argv->s_len);

  s = argv->s_buf;
  c = 0;
  v = (char **) malloc(sizeof(char *));

  /* Split up command line */
  while ( *s != NUL ) {
    char *p = s;

    /* Extract item */
    if ( *s == '\'' ) {
      p = ++s;
      while ( (*p != NUL) && (*p != '\'') )
        p++;
    }
    else if ( *s == '"' ) {
      p = ++s;
      while ( (*p != NUL) && (*p != '"') )
        p++;
    }
    else {
      p = strskip_chars(s);
    }
    if ( *p != NUL )
      *(p++) = NUL;

    /* Add item to argument list */
    v = (char **) realloc(v, sizeof(char *) * (c+2));
    v[c++] = argv_reference(s, ref_argv);

    s = strskip_spaces(p);
  }

  v[c] = NULL;
  argv->argc = c;
  argv->argv = v;

  return argv;
}


void shell_argv_free(shell_argv_t *argv)
{
  if ( argv == NULL )
    return;

  /* Free miscellaneous buffers */
  if ( argv->s_pid != NULL )
    free(argv->s_pid);
  if ( argv->s_argc != NULL )
    free(argv->s_argc);
  if ( argv->s_buf != NULL )
    free(argv->s_buf);

  /* Free argument table */
  if ( argv->argv != NULL )
    free(argv->argv);

  free(argv);
}


char *shell_argv_pack(shell_argv_t *argv)
{
  char *buf;
  char *p;
  int i;

  /* Alloc arguments buffer */
  buf = (char *) malloc(argv->s_len);

  /* Build arguments buffer */
  p = buf;
  for (i = 0; i < argv->argc; i++) {
    char *str = argv->argv[i];
    int len = strlen(str);

    memcpy(p, str, len);
    p += len;

    if ( i < (argv->argc-1) )
      *(p++) = ' ';
  }

  *p = NUL;

  return buf;
}


int shell_argv_fprintf(FILE *f, shell_argv_t *argv, int offset)
{
  int count = 0;
  int i;

  if ( argv != NULL )
    for (i = offset; i < argv->argc; i++)
      count += fprintf(f, "%s ", argv->argv[i]);

  return count;
}
