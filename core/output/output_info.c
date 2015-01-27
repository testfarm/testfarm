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
