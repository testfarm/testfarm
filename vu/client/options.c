/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* Client options                                                           */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 24-JAN-2007                                                    */
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

#include "options.h"


int opt_debug = 0;
int opt_shared = 0;
char *opt_name = NULL;
int opt_rotate = 0;
GList *opt_filters = NULL;
int opt_quad = 0;


int options_parse(int *pargc, char ***pargv)
{
  int argc = *pargc;
  char **argv = *pargv;
  int i;

  for (i = 1; i < argc; i++) {
    char *s = argv[i];

    if ( *s == '-' ) {
      s++;

      if ( strcmp(s, "debug") == 0 ) {
	opt_debug = 2;
      }
      else if ( strcmp(s, "shared") == 0 ) {
	opt_shared = 1;
      }
      else if ( strcmp(s, "rotate-cw") == 0 ) {
	opt_rotate = +1;
      }
      else if ( strcmp(s, "rotate-ccw") == 0 ) {
	opt_rotate = -1;
      }
      else if ( strcmp(s, "name") == 0 ) {
	char *s = argv[++i];
	if ( s == NULL )
	  return -1;
	if ( opt_name != NULL )
	  free(opt_name);
	opt_name = strdup(s);
      }
      else if ( strcmp(s, "filter") == 0 ) {
	char *s = argv[++i];
	if ( s == NULL )
	  return -1;
	opt_filters = g_list_append(opt_filters, strdup(s));
      }
      else if ( strcmp(s, "quad") == 0 ) {
	opt_quad = 1;
      }
      else {
	return -1;
      }
    }
    else {
      break;
    }
  }

  *pargc = argc - i;
  *pargv = argv + i;

  return 0;
}
