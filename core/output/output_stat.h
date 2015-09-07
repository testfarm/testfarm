/****************************************************************************/
/* TestFarm                                                                 */
/* Execution statistics                                                     */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 19-FEB-2001                                                    */
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
