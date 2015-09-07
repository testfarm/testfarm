/****************************************************************************/
/* TestFarm                                                                 */
/* Test Program for file list function                                      */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 11-JUN-2004                                                    */
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
#include <glib.h>

#include "filelist.h"

int main(int argc, char *argv[])
{
  GList *list;
  GList *l;

  if ( argc != 2 ) {
    fprintf(stderr, "Usage: filelist_test <regex>\n");
    exit(1);
  }

  list = filelist(argv[1]);

  l = list;
  while ( l != NULL ) {
    printf("%s\n", (char *) l->data);
    l = l->next;
  }

  filelist_free(list);

  return 0;
}
