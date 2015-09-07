/****************************************************************************/
/* TestFarm                                                                 */
/* Test Suite code generator                                                */
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
#include <stdlib.h>
#include <string.h>

#include "codegen.h"
#include "codegen_tree_show.h"
#include "codegen_tree_error.h"


#define NAME "codegen"


/*******************************************************/
/* Program body                                        */
/*******************************************************/

void usage(void)
{
  fprintf(stderr, "Usage: " NAME " <tree-file> [-gui] [-show] [-log <local-log>]\n");
  fprintf(stderr, "  <tree-file>: Test Tree file\n");
  fprintf(stderr, "  -show: dump tree structure to stderr\n");
  exit(EXIT_FAILURE);
}


int main(int argc, char *argv[])
{
  char *filename = NULL;
  int show = 0;
  tree_t *tree;
  int i;

  /* Check arguments */
  for (i = 1; i < argc; i++) {
    char *s = argv[i];

    if ( s[0] == '-' ) {
      if ( strcmp(s, "-show") == 0 ) {
        show = 1;
      }
      else {
        usage();
      }
    }
    else if ( filename == NULL ) {
      filename = s;
    }
    else {
      usage();
    }
  }

  if ( filename == NULL )
    usage();

  /* Load Test Tree file */
  tree = tree_load(tree_new(), filename);
  if ( tree == NULL ) {
    fprintf(stderr, "codegen: File %s not found\n", filename);
    return 1;
  }

  /* Show error messages */
  tree_error_show(tree, stderr);

  if ( tree->errcount == 0 ) {
    /* Show tree content */
    if ( show )
      tree_show(tree, stderr);

    /* Build PERL code */
    code_build(stdout, tree);
  }

  /* That's all folks */
  tree_destroy(tree, 0);

  return 0;
}
