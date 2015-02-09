/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* Client options                                                           */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 24-JAN-2007                                                    */
/****************************************************************************/

/*
 * $Revision: 917 $
 * $Date: 2008-01-25 16:04:27 +0100 (ven., 25 janv. 2008) $
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
