/****************************************************************************/
/* TestFarm                                                                 */
/* Output file abstract descriptor                                          */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 02-FEB-2004                                                    */
/****************************************************************************/

/*
 * $Revision: 754 $
 * $Date: 2007-09-29 17:54:20 +0200 (sam., 29 sept. 2007) $
 */

#include <stdio.h>
#include <string.h>
#include <malloc.h>

#include "output_file.h"


int output_file_type(char *filename)
{
  FILE *f;
  int type = OUTPUT_FILE_UNKNOWN;

  if ( (f = fopen(filename, "r")) != NULL ) {
    char buf[80];

    if ( fgets(buf, sizeof(buf), f) != NULL ) {
      if ( strncmp(buf, "<?xml", 5) == 0 )
        type = OUTPUT_FILE_XML;
    }

    fclose(f);
  }

  return type;
}


void output_file_name(output_file_t *of, char *filename)
{
  if ( of == NULL )
    return;

  if ( of->name != NULL )
    free(of->name);
  of->name = NULL;

  if ( filename != NULL )
    of->name = strdup(filename);
}
