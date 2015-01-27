/****************************************************************************/
/* TestFarm                                                                 */
/* Output Test Case/Scenario information                                    */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 19-FEB-2001                                                    */
/****************************************************************************/

/*
 * $Revision: 42 $
 * $Date: 2006-06-03 15:30:01 +0200 (sam., 03 juin 2006) $
 */

#ifndef __TESTFARM_OUTPUT_INFO_H
#define __TESTFARM_OUTPUT_INFO_H

typedef struct {
  char *name;          /* Node name */
  char *description;   /* Textual Description */
  char *reference;     /* reference to Test Specification */
  int verdict;         /* Verdict result */
  long elapsed;        /* Duration */
  int validated;       /* Validation check */
  int criticity;       /* Criticity level */
} output_info_t;

extern void output_info_init(output_info_t *info, char *name);
extern void output_info_description(output_info_t *info, char *description);
extern void output_info_reference(output_info_t *info, char *reference);
extern void output_info_clear(output_info_t *info);

#endif /* __TESTFARM_OUTPUT_INFO_H */
