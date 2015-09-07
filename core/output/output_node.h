/****************************************************************************/
/* TestFarm                                                                 */
/* Output Test Case and Sequence                                            */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 02-FEB-2004                                                    */
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
