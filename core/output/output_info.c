/****************************************************************************/
/* TestFarm                                                                 */
/* Output Test Case/Scenario information                                    */
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
#include <string.h>
#include <malloc.h>

#include "codegen.h"
#include "output_info.h"


void output_info_init(output_info_t *info, char *name)
{
  info->name = strdup(name);
  info->description = NULL;
  info->reference = NULL;

  info->verdict = VERDICT_UNEXECUTED;
  info->elapsed = 0;
  info->validated = 0;
  info->criticity = CRITICITY_NONE;
}


void output_info_description(output_info_t *info, char *description)
{
  if ( info->description != NULL) 
    free(info->description);
  info->description = NULL;

  if ( (description != NULL) && (description[0] != '\0') )
    info->description = strdup(description);
}


void output_info_reference(output_info_t *info, char *reference)
{
  if ( info->reference != NULL) 
    free(info->reference);
  info->reference = NULL;

  if ( (reference != NULL) && (reference[0] != '\0') )
    info->reference = strdup(reference);
}


void output_info_clear(output_info_t *info)
{
  if ( info->name != NULL) 
    free(info->name);
  info->name = NULL;

  output_info_description(info, NULL);
  output_info_reference(info, NULL);

  info->verdict = VERDICT_UNEXECUTED;
  info->elapsed = 0;
  info->validated = 0;
  info->criticity = CRITICITY_NONE;
}
