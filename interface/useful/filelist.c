/****************************************************************************/
/* TestFarm                                                                 */
/* Get a File List from a file path regular expression                      */
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
#include <string.h>
#include <malloc.h>
#include <dirent.h>
#include <regex.h>
#include <glib.h>

#include "filelist.h"


#if ! GLIB_CHECK_VERSION(2,0,0)
#define g_path_get_dirname(path) g_dirname(path)
#define g_path_get_basename(path) strdup(g_basename(path))
#endif

GList *filelist(char *path)
{
  char *dirname = g_path_get_dirname(path);
  char *basename = g_path_get_basename(path);
  GList *list = NULL;
  regex_t reg;

  if ( regcomp(&reg, basename, REG_EXTENDED | REG_NOSUB) == 0 ) {
    DIR *dir;

    /* Open directory */
    if ( (dir = opendir(dirname)) != NULL ) {
      struct dirent *entry;

      /* Scan directory */
      while ( (entry = readdir(dir)) != NULL ) {
	if ( regexec(&reg, entry->d_name, 0, NULL, 0) == 0 )
	  list = g_list_append(list, strdup(entry->d_name));
      }

      /* Close directory */
      closedir(dir);
    }

    regfree(&reg);
  }

  free(basename);
  free(dirname);

  return list;
}


void filelist_free(GList *list)
{
  g_list_foreach(list, (GFunc) free, NULL);
  g_list_free(list);
}
