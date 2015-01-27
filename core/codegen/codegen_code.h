/****************************************************************************/
/* TestFarm                                                                 */
/* Test Suite code generator : PERL generation                              */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 12-APR-2000                                                    */
/****************************************************************************/

/*
 * $Revision: 374 $
 * $Date: 2007-02-27 19:24:08 +0100 (mar., 27 févr. 2007) $
 */

#ifndef __CODEGEN_CODE_H
#define __CODEGEN_CODE_H

#include <stdio.h>
#include "codegen_tree.h"

extern void code_build(FILE *f, tree_t *tree);

#endif /* __CODEGEN_CODE_H */
