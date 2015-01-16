/****************************************************************************/
/* TestFarm                                                                 */
/* Get a File List from a file path regular expression                      */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 11-JUN-2004                                                    */
/****************************************************************************/

/*
 * $Revision: 728 $
 * $Date: 2007-09-22 10:20:01 +0200 (sam., 22 sept. 2007) $
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
