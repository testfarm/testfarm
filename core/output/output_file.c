/****************************************************************************/
/* TestFarm                                                                 */
/* Output file abstract descriptor                                          */
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
