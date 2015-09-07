/****************************************************************************/
/* TestFarm                                                                 */
/* Test Suite code generator : test tree                                    */
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

#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <unistd.h>
#include <glib.h>
#include <sys/stat.h>

#include "useful.h"
#include "codegen_script.h"


/*======================================================*/
/* Script descriptor                                    */
/*======================================================*/

static char *script_get_package(char *filename)
{
  FILE *f;
  char buf[BUFSIZ];
  int size;
  char *package;
  int comment;

  if ( (f = fopen(filename, "r")) == NULL )
    return NULL;

  package = NULL;
  size = 0;
  comment = 0;
  while ( (package == NULL) && (! feof(f)) ) {
    int c = fgetc(f);

    switch ( c ) {
    case ';': {
      char *s0, *s1;

      buf[size] = NUL;
      size = 0;

      /* Extract first keyword */
      s0 = strskip_spaces(buf);
      s1 = strskip_chars(s0);
      if ( *s1 != NUL )
        *(s1++) = NUL;

      if ( strcmp(s0, "package") == 0 ) {
        s0 = strskip_spaces(s1);
        s1 = strskip_chars(s1);
        if ( *s1 != NUL )
          *(s1++) = NUL;

        package = strdup(s0);
      }
    }
    break;

    case '#':
      comment = 1;
      break;

    case EOF:
      break;

    default:
      if ( comment ) {
        if ( c == '\n' )
          comment = 0;
      }
      else {
        if ( c < ' ' )
          c = ' ';
        if ( size <= BUFSIZ )
          buf[size++] = c;
      }
      break;
    }
  }

  fclose(f);

  return package;
}


static char *script_get_wizname(char *filename)
{
  char *wizname;
  char *s;

  if ( filename == NULL )
    return NULL;

  wizname = (char *) malloc(strlen(filename)+5);
  strcpy(wizname, filename);

  if ( (s = strrchr(wizname, '.')) != NULL ) {
    strcpy(s, SCRIPT_WIZ_SUFFIX);
    if ( access(wizname, R_OK) ) {
      free(wizname);
      wizname = NULL;
    }
  }
  else {
    free(wizname);
    wizname = NULL;
  }

  return wizname;
}


script_t *script_new(char *cmdline)
{
  char *name, *params;
  script_t *script;

  /* Extract script name */
  name = strskip_spaces(cmdline);
  if ( *name == NUL )
    return NULL;

  /* Extract script parameters */
  if ( (params = strchr(name, ' ')) != NULL ) {
    *(params++) = NUL;
    if ( *strskip_spaces(params) == NUL )
      params = NULL;
  }

  /* Retrieve cannonical file name */
  if ( (name[0] == '.') && (name[1] == G_DIR_SEPARATOR) )
    name += 2;

  /* Alloc and feed script descriptor */
  script = (script_t *) malloc(sizeof(script_t));
  script->name = strdup(name);
  script->wizname = script_get_wizname(name);
  script->params = (params == NULL) ? NULL : strdup(params);

  /* Retrieve module path and name */
  script->package = script_get_package(script->name);

  return script;
}

void script_destroy(script_t *script)
{
  /* Free script descriptor */
  if ( script->name != NULL )
    free(script->name);
  if ( script->wizname != NULL )
    free(script->wizname);
  if ( script->params != NULL )
    free(script->params);
  if ( script->package != NULL )
    free(script->package);
  free(script);
}


void script_show(script_t *script, FILE *f)
{
  if ( script == NULL ) {
    fprintf(f, "      (none)\n");
    return;
  }

  fprintf(f, "      Name: %s", script->name);
  if ( script->wizname != NULL )
    fprintf(f, " (%s)", script->wizname);
  fprintf(f, "\n");
  if ( script->params != NULL )
    fprintf(f, "      Parameters: %s\n", script->params);
  fprintf(f, "      Package: %s\n", script->package);
}


int script_check_wiz(script_t *script)
{
  struct stat stat1;
  struct stat stat2;

  if ( script->wizname == NULL )
    return 0;

  if ( (stat(script->name, &stat1) == 0) && (stat(script->wizname, &stat2) == 0) ) {
    if ( stat1.st_mtime < stat2.st_mtime )
      return 1;
  }

  return 0;
}
