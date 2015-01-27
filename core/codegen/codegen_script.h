/****************************************************************************/
/* TestFarm                                                                 */
/* Test Suite code generator : test scripts                                 */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 12-APR-2000                                                    */
/****************************************************************************/

/*
 * $Revision: 42 $
 * $Date: 2006-06-03 15:30:01 +0200 (sam., 03 juin 2006) $
 */

#ifndef __CODEGEN_SCRIPT_H
#define __CODEGEN_SCRIPT_H

typedef struct {
  char *name;
  char *wizname;
  char *params;
  char *package;
} script_t;


#define SCRIPT_PERL_SUFFIX ".pm"
#define SCRIPT_WIZ_SUFFIX ".wiz"

extern script_t *script_new(char *cmdline);
extern void script_destroy(script_t *script);
extern void script_show(script_t *script, FILE *f);
extern int script_check_wiz(script_t *script);

#endif /* __CODEGEN_SCRIPT_H */
