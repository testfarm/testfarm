/****************************************************************************/
/* TestFarm                                                                 */
/* Execution statistics                                                     */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 19-FEB-2001                                                    */
/****************************************************************************/

/*
 * $Revision: 772 $
 * $Date: 2007-10-11 15:58:52 +0200 (jeu., 11 oct. 2007) $
 */

#ifndef __TESTFARM_OUTPUT_STAT_H
#define __TESTFARM_OUTPUT_STAT_H

#include "codegen.h"

typedef struct {
  int nscenario;      /* Total number of scenarii */
  int ncase;          /* Total number of test cases */
  int executed;       /* Number of executed test cases */
  int passed;         /* Number of PASSED test cases */
  int failed;         /* Number of FAILED test cases */
  int inconclusive;   /* Number of INCONCLUSIVE test cases */
  int skip;           /* Number of SKIP test cases */
  long elapsed_time;
} output_stat_t;

extern void output_stat_init(output_stat_t *stat, tree_t *codegen_tree);
extern void output_stat_clear(output_stat_t *stat);
extern void output_stat_elapsed(output_stat_t *stat, int elapsed_time);
extern void output_stat_verdict(output_stat_t *stat, int verdict, int criticity);

typedef struct {
  int scenario_executed;
  int scenario_passed;
  int scenario_failed;
  int scenario_inconclusive;
  int scenario_skip;
} output_stat_sequence_t;

#endif /* __TESTFARM_OUTPUT_STAT_H */
