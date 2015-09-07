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

#include <stdio.h>
#include <glib.h>

#include "codegen.h"
#include "output_stat.h"


void output_stat_init(output_stat_t *stat, tree_t *codegen_tree)
{
  output_stat_clear(stat);

  if ( codegen_tree != NULL ) {
    tree_count(codegen_tree, &(stat->ncase), &(stat->nscenario));
  }
  else {
    stat->ncase = 0;
    stat->nscenario = 0;
  }
}


void output_stat_clear(output_stat_t *stat)
{
  stat->executed = 0;
  stat->passed = 0;
  stat->failed = 0;
  stat->inconclusive = 0;
  stat->skip = 0;
  stat->elapsed_time = 0;
}


void output_stat_elapsed(output_stat_t *stat, int elapsed_time)
{
  if ( elapsed_time > stat->elapsed_time )
    stat->elapsed_time = elapsed_time;
}


void output_stat_verdict(output_stat_t *stat, int verdict, int criticity)
{
  switch ( verdict ) {
  case VERDICT_PASSED:
    (stat->executed)++;
    if ( criticity > CRITICITY_NONE )
      (stat->passed)++;
    break;
  case VERDICT_FAILED:
    (stat->executed)++;
    if ( criticity > CRITICITY_NONE )
      (stat->failed)++;
    break;
  case VERDICT_INCONCLUSIVE:
    (stat->executed)++;
    if ( criticity > CRITICITY_NONE )
      (stat->inconclusive)++;
    break;
  case VERDICT_SKIP:
    (stat->skip)++;
    break;
  default:
    break;
  }
}
