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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>

#include "codegen.h"
#include "validate.h"
#include "output_node.h"

/*---------------------------------------------------*/
/* Test Case nodes                                   */
/*---------------------------------------------------*/

output_case_t *output_case_alloc(char *name, char *description)
{
  output_case_t *ocase = (output_case_t *) malloc(sizeof(output_case_t));
  output_info_init(&(ocase->info), name);
  output_info_description(&(ocase->info), description);
  ocase->seq = NULL;
  ocase->offset = 0;
  ocase->begin = 0;
  ocase->end = 0;
  ocase->validate = NULL;
  ocase->default_criticity = CRITICITY_NONE;

  return ocase;
}

void output_case_free(output_case_t *ocase)
{
  if ( ocase == NULL )
    return;

  output_info_clear(&(ocase->info));
  free(ocase);
}


static int output_case_compare(output_case_t *ocase, char *name)
{
  return strcmp(ocase->info.name, name);
}


output_case_t *output_case_find(GList *cases, char *name)
{
  GList *item = g_list_find_custom(cases, name, (GCompareFunc) output_case_compare);
  return (item != NULL) ? item->data : NULL;
}


/*---------------------------------------------------*/
/* Sequence nodes                                    */
/*---------------------------------------------------*/

output_sequence_t *output_sequence_alloc(char *name, char *description, int is_scenario)
{
  output_sequence_t *oseq = (output_sequence_t *) malloc(sizeof(output_sequence_t));
  output_info_init(&(oseq->info), name);
  output_info_description(&(oseq->info), description);
  oseq->is_scenario = is_scenario;
  oseq->cases = NULL;

  return oseq;
}


void output_sequence_free(output_sequence_t *oseq)
{
  output_info_clear(&(oseq->info));
  g_list_free(oseq->cases);
  free(oseq);
}


static int output_sequence_compare(output_sequence_t *oseq, char *name)
{
  return strcmp(oseq->info.name, name);
}


output_sequence_t *output_sequence_find(GList *sequences, char *name)
{
  GList *item = g_list_find_custom(sequences, name, (GCompareFunc) output_sequence_compare);
  return (item != NULL) ? item->data : NULL;
}


/*---------------------------------------------------*/
/* Sequence statistics                               */
/*---------------------------------------------------*/

/*
  Propagation rules for sequence statistics:
  * Verdict :
    - PASSED if sequence contains only PASSED or SKIP test cases;
    - FAILED if sequence contains at least one FAILED test case;
    - INCONCLUSIVE if sequence contains no FAILED and at least one INCONCLUSIVE
    - SKIP if sequence contain only SKIP test cases
  * Criticity:
    If verdict is X, take the highest criticity of the test cases that produced this verdict
  * Validated:
    If all test cases are validated
*/

static void output_sequence_verdict_update(output_case_t *ocase, output_sequence_t *oseq)
{
  switch ( ocase->info.verdict ) {
  case VERDICT_PASSED:
    if ( (oseq->info.verdict != VERDICT_FAILED) && (oseq->info.verdict != VERDICT_INCONCLUSIVE) )
      oseq->info.verdict = VERDICT_PASSED;
    break;
  case VERDICT_FAILED:
    oseq->info.verdict = VERDICT_FAILED;
    break;
  case VERDICT_INCONCLUSIVE:
    if ( oseq->info.verdict != VERDICT_FAILED )
      oseq->info.verdict = VERDICT_INCONCLUSIVE;
    break;
  case VERDICT_SKIP:
    if ( oseq->info.verdict == VERDICT_UNEXECUTED )
      oseq->info.verdict = VERDICT_SKIP;
    break;
  default:
    break;
  }
}


static void output_sequence_verdict_compute(output_sequence_t *oseq)
{
  oseq->info.verdict = VERDICT_UNEXECUTED;
  g_list_foreach(oseq->cases, (GFunc) output_sequence_verdict_update, oseq);
}


static void output_sequence_info_update(output_case_t *ocase, output_sequence_t *oseq)
{
  /* Update elapsed time */
  oseq->info.elapsed += ocase->info.elapsed; 

  /* Update criticity level */
  if ( (ocase->info.verdict == oseq->info.verdict) &&
       (ocase->info.criticity > oseq->info.criticity) ) {
    oseq->info.criticity = ocase->info.criticity;
  }

  /* Update validation flag */
  if ( ocase->info.validated == 0 )
    oseq->info.validated = 0;
}


static void output_sequence_info_compute(output_sequence_t *oseq)
{
  oseq->info.elapsed = 0;
  oseq->info.validated = 1;
  oseq->info.criticity = CRITICITY_NONE;
  g_list_foreach(oseq->cases, (GFunc) output_sequence_info_update, oseq);

  // TODO: Confirm sequence validation state by performing a full md5sum check
  // TODO: if ( seq->info.validated ) {
  // TODO:   cd <dir> && md5sum --status -c .check
  // TODO: }
}


static void output_scenario_count_executed(output_sequence_t *seq, int *count)
{
  if ( (seq->is_scenario) &&
       (seq->info.verdict != VERDICT_UNEXECUTED) &&
       (seq->info.verdict != VERDICT_SKIP) ) {
    (*count)++;
  }
}


#define DECLARE_SCENARIO_VERDICT_COUNTER(_verdict_) \
static void output_scenario_count_ ## _verdict_ (output_sequence_t *seq, int *count) { \
  if ( (seq->is_scenario) && \
       (seq->info.criticity != CRITICITY_NONE) && \
       (seq->info.verdict == VERDICT_ ## _verdict_) ) \
    (*count)++; \
}

#define COMPUTE_SCENARIO_COUNTER(var, func) \
  sstat->scenario_ ## var = 0; \
  g_list_foreach(sequences, (GFunc) output_scenario_count_ ## func, &(sstat->scenario_ ## var))

DECLARE_SCENARIO_VERDICT_COUNTER(PASSED);
DECLARE_SCENARIO_VERDICT_COUNTER(FAILED);
DECLARE_SCENARIO_VERDICT_COUNTER(INCONCLUSIVE);
DECLARE_SCENARIO_VERDICT_COUNTER(SKIP);


void output_sequence_compute(GList *sequences, output_stat_sequence_t *sstat)
{
  /* Compute criticity and validation flag of all sequences */
  g_list_foreach(sequences, (GFunc) output_sequence_verdict_compute, NULL);
  g_list_foreach(sequences, (GFunc) output_sequence_info_compute, NULL);

  /* Compute number of executed scenarios */
  COMPUTE_SCENARIO_COUNTER(executed, executed);

  /* Compute verdict statistics for scenarios */
  COMPUTE_SCENARIO_COUNTER(passed, PASSED);
  COMPUTE_SCENARIO_COUNTER(failed, FAILED);
  COMPUTE_SCENARIO_COUNTER(inconclusive, INCONCLUSIVE);
  COMPUTE_SCENARIO_COUNTER(skip, SKIP);
}
