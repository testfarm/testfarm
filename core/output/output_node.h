/****************************************************************************/
/* TestFarm                                                                 */
/* Output Test Case and Sequence                                            */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 02-FEB-2004                                                    */
/****************************************************************************/

/*
 * $Revision: 772 $
 * $Date: 2007-10-11 15:58:52 +0200 (jeu., 11 oct. 2007) $
 */

#ifndef __TESTFARM_OUTPUT_NODE_H
#define __TESTFARM_OUTPUT_NODE_H

#include <glib.h>

#include "validate.h"
#include "output_info.h"
#include "output_stat.h"

typedef struct {
  output_info_t info;      /* Sequence info data */
  int is_scenario;         /* Set if sequence is a scenario (leaf sequence) */
  GList *cases;            /* List of test cases the sequence points to */
} output_sequence_t;

typedef struct {
  output_info_t info;       /* Test case info data */
  output_sequence_t *seq;   /* Parent sequence */
  unsigned long offset;     /* Offset of test case in the output file */
  long long begin;          /* Date (ms) at which the Test Case began */
  long long end;            /* Date (ms) at which the Test Case ended */
  validate_t *validate;
  int default_criticity;
} output_case_t;

extern output_case_t *output_case_alloc(char *name, char *description);
extern void output_case_free(output_case_t *ocase);
extern output_case_t *output_case_find(GList *cases, char *name);

extern output_sequence_t *output_sequence_alloc(char *name, char *description, int is_scenario);
extern void output_sequence_free(output_sequence_t *oseq);
extern output_sequence_t *output_sequence_find(GList *sequences, char *name);
extern void output_sequence_compute(GList *sequences, output_stat_sequence_t *sstat);

#endif /* __TESTFARM_OUTPUT_NODE_H */
