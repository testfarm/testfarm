/****************************************************************************/
/* TestFarm                                                                 */
/* Test Suite code generator : Test Tree Error Recording                    */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 22-JUN-2000                                                    */
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
#include <stdarg.h>
#include <string.h>
#include <malloc.h>
#include <glib.h>

#include "codegen_tree.h"
#include "codegen_tree_error.h"


static tree_err_t *tree_error_add(tree_t *tree, int level, tree_err_loc_t *loc,
                                  char *fmt, va_list ap)
{
  tree_err_t *err;
  char buf[1024];

  err = (tree_err_t *) malloc(sizeof(tree_err_t));
  tree->errlist = g_list_append(tree->errlist, err);

  err->loc.filename = NULL;
  err->loc.lineno = 0;
  if ( loc != NULL ) {
    if ( loc->filename != NULL )
      err->loc.filename = strdup(loc->filename);
    err->loc.lineno = loc->lineno;
  }
  err->level = level;
  err->object = NULL;

  vsnprintf(buf, sizeof(buf), fmt, ap);
  err->msg = strdup(buf);

  return err;
}


static void tree_error_free(tree_err_t *err)
{
  if ( err->loc.filename != NULL )
    free(err->loc.filename);
  free(err->msg);
  free(err);
}


void tree_error_clear(tree_t *tree)
{
  if ( tree == NULL )
    return;

  g_list_foreach(tree->errlist, (GFunc) tree_error_free, NULL);
  g_list_free(tree->errlist);
  tree->errlist = NULL;

  tree->errcount = 0;
  tree->warncount = 0;
}


static void tree_error_show_item(tree_err_t *err, FILE *f)
{
  if ( err->loc.filename != NULL ) {
    fprintf(f, "%s:", err->loc.filename);
    if ( err->loc.lineno > 0 )
      fprintf(f, "%d:", err->loc.lineno);
  }

  switch ( err->level ) {
  case TREE_ERR_INFO:    fprintf(f, " (INFO) ");    break;
  case TREE_ERR_WARNING: fprintf(f, " (WARNING) "); break;
  case TREE_ERR_ERROR:   fprintf(f, " (ERROR) ");   break;
  case TREE_ERR_PANIC:   fprintf(f, " (*PANIC*) "); break;
  default : break;
  }

  fprintf(f, "%s\n", err->msg);
}


void tree_error_show(tree_t *tree, FILE* f)
{
  if ( tree == NULL )
    return;
  g_list_foreach(tree->errlist, (GFunc) tree_error_show_item, f);
}


void tree_info(tree_t *tree, tree_err_loc_t *loc, char *fmt, ...)
{
  va_list ap;

  va_start(ap, fmt);
  tree_error_add(tree, TREE_ERR_INFO, loc, fmt, ap);
  va_end(ap);
}


tree_err_t *tree_warning(tree_t *tree, tree_err_loc_t *loc, char *fmt, ...)
{
  va_list ap;
  tree_err_t *err;

  va_start(ap, fmt);
  err = tree_error_add(tree, TREE_ERR_WARNING, loc, fmt, ap);
  va_end(ap);

  (tree->warncount)++;

  return err;
}


void tree_error(tree_t *tree, tree_err_loc_t *loc, char *fmt, ...)
{
  va_list ap;

  va_start(ap, fmt);
  tree_error_add(tree, TREE_ERR_ERROR, loc, fmt, ap);
  va_end(ap);

  (tree->errcount)++;
}


void tree_panic(tree_t *tree, tree_err_loc_t *loc, char *fmt, ...)
{
  va_list ap;

  va_start(ap, fmt);
  tree_error_add(tree, TREE_ERR_PANIC, loc, fmt, ap);
  va_end(ap);

  (tree->errcount)++;
}
