/****************************************************************************/
/* TestFarm                                                                 */
/* Test Suite code generator                                                */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 12-APR-2000                                                    */
/****************************************************************************/

/*
 * $Revision: 374 $
 * $Date: 2007-02-27 19:24:08 +0100 (mar., 27 févr. 2007) $
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
