/****************************************************************************/
/* TestFarm                                                                 */
/* Standard Installation Environment Settings                               */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 08-AUG-2003                                                    */
/****************************************************************************/

/*
 * $Revision: 200 $
 * $Date: 2006-08-03 15:01:45 +0200 (jeu., 03 août 2006) $
 */


#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <glib.h>
#include <sys/stat.h>

#include "install.h"

char *get_home(void)
{
  char *env = getenv("TESTFARM_HOME");
  return (env != NULL) ? env : INSTALL_HOME;
}


char *get_config(void)
{
  char *env = getenv("TESTFARM_CONFIG");
  return (env != NULL) ? env : INSTALL_CONFIG;
}


char *get_browser(void)
{
  char *env = getenv("TESTFARM_BROWSER");
  return (env != NULL) ? env : INSTALL_DEFAULT_BROWSER;
}


char *get_perldb(void)
{
  char *env = getenv("TESTFARM_PERLDB");
  return (env != NULL) ? env : INSTALL_DEFAULT_PERLDB;
}


char *get_user_config(char *fmt, ...)
{
  static char path[PATH_MAX+1];
  int len;
  va_list ap;

  len = snprintf(path, sizeof(path), "%s" G_DIR_SEPARATOR_S ".testfarm" G_DIR_SEPARATOR_S, g_get_home_dir());

  /* Ensure user config directory is created */
  if ( mkdir(path, 0777) ) {
    if ( errno != EEXIST )
      fprintf(stderr, "WARNING: Cannot create user config directory '%s': %s\n", path, strerror(errno));
  }

  if ( fmt != NULL ) {
    va_start(ap, fmt);
    vsnprintf(path+len, sizeof(path)-len, fmt, ap);
    va_end(ap);
  }

  return path;
}


static char *check_lib(char *dirname, char *fname)
{
  int flen = strlen(fname);
  char *fpath = NULL;
  int size;

  size = strlen(dirname) + flen + 6;
  fpath = (char *) malloc(size);
  snprintf(fpath, size, "%s" G_DIR_SEPARATOR_S "lib" G_DIR_SEPARATOR_S "%s", dirname, fname);

  if ( access(fpath, R_OK) ) {
    free(fpath);
    fpath = NULL;
  }

  return fpath;
}


char *get_lib(char *fname)
{
  char *fpath;

  if ( (fpath = check_lib(get_config(), fname)) )
    return fpath;

  if ( (fpath = check_lib(get_home(), fname)) )
    return fpath;

  return NULL;
}
