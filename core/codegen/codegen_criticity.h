/****************************************************************************/
/* TestFarm                                                                 */
/* Test Suite code generator : test case criticity identifiers              */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 07-DEC-2001                                                    */
/****************************************************************************/

/*
 * $Revision: 42 $
 * $Date: 2006-06-03 15:30:01 +0200 (sam., 03 juin 2006) $
 */

#ifndef __CODEGEN_CRITICITY_H
#define __CODEGEN_CRITICITY_H

#define CRITICITY_UNKNOWN -1
#define CRITICITY_NONE 0

typedef int criticity_t;

extern int criticity_init(void);
extern void criticity_destroy(void);

extern char *criticity_id(criticity_t criticity);
extern char *criticity_color(criticity_t criticity);
extern criticity_t criticity_level(char *id);

#endif /* __CODEGEN_CRITICITY_H */
