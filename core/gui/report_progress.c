/****************************************************************************/
/* TestFarm Core                                                            */
/* Test Progress dump                                                       */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 02-OCT-2007                                                    */
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
#include <string.h>
#include <malloc.h>
#include <glib.h>

#include "options.h"
#include "report_progress.h"


void report_progress_msg(report_progress_t *rp, char *msg)
{
  if ( rp->msg != NULL )
    free(rp->msg);
  rp->msg = strdup(msg);
  rp->value = -1;
}


void report_progress_value(report_progress_t *rp, int value)
{
  rp->value = value;
}


static gboolean report_progress_timeout(report_progress_t *rp)
{
  if ( rp->value >= 0 ) {
    printf("%d%%\n", rp->value);
    fflush(stdout);
  }
  else if ( rp->msg != NULL ) {
    printf("%s\n", rp->msg);
    fflush(stdout);
  }

  if ( rp->msg != NULL ) {
    free(rp->msg);
    rp->msg = NULL;
  }

  rp->value = -1;

  return TRUE;
}


void report_progress_init(report_progress_t *rp)
{
  rp->timeout_tag = 0;
  rp->value = -1;
  rp->msg = NULL;

  if ( opt_command ) {
    rp->timeout_tag = g_timeout_add(500, (GSourceFunc) report_progress_timeout, rp);
  }
}


void report_progress_clear(report_progress_t *rp)
{
  if ( rp->timeout_tag > 0 ) {
    g_source_remove(rp->timeout_tag);
    rp->timeout_tag = 0;
  }

  if ( rp->msg != NULL ) {
    free(rp->msg);
    rp->msg = NULL;
  }
}
