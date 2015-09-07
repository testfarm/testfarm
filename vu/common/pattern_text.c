/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* Text Pattern management                                                  */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 27-JUN-2007                                                    */
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
#include <pcre.h>

#include "pattern.h"
#include "pattern_text.h"


static void pattern_text_err(char **err, char *fmt, int position, const char *msg)
{
  if ( err != NULL ) {
    int size = strlen(fmt) + strlen(msg) + 20;
    *err = malloc(size);
    if ( position < 0 )
      snprintf(*err, size, fmt, msg);
    else
      snprintf(*err, size, fmt, position, msg);
  }
}


static int pattern_text_compile(pattern_t *pattern, char **err)
{
  const char *errptr = NULL;
  int erroffset;

  /* Reject empty regex */
  if ( pattern->source[0] == '\0' ) {
    pattern_text_err(err, "regex error: empty string", -1, errptr);
    return -1;
  }

  /* Compile regular expression */
  pattern->d.text.re = pcre_compile(pattern->source, 0, &errptr, &erroffset, NULL);
  if ( pattern->d.text.re == NULL ) {
    pattern_text_err(err, "regex error at position %d: %s", erroffset, errptr);
    return -1;
  }

  pattern->d.text.hints = pcre_study(pattern->d.text.re, 0, &errptr);
  if ( errptr != NULL ) {
    pattern_text_err(err, "regex error (study): %s", -1, errptr);
  }

  return 0;
}


int pattern_text_set(pattern_t *pattern, char *source, char **err)
{
  /* Set regex source specification */
  if ( pattern->source != NULL )
    free(pattern->source);
  pattern->source = strdup(source);

  /* Free regex gears */
  pattern_text_free(pattern);

  /* Set regex substring capture vector size */
  pattern->d.text.ovec_n = 30;

  /* Compile regex */
  if ( pattern_text_compile(pattern, err) < 0 )
    return -1;

  return 0;
}


int pattern_text_set_options(pattern_t *pattern, GList *options, char **perr)
{
  int ret = 0;

  if ( options != NULL ) {
    char *err = NULL;
    int errlen = 0;

    GList *l = options;
    while ( l ) {
      char *str = l->data;
      int size = strlen(str) + 1;
      char *prefix = "";

      if ( err == NULL ) {
	err = strdup("Illegal option for text pattern: ");
	errlen = strlen(err);
      }
      else {
	prefix = ", ";
	size += 2;
      }

      err = (char *) realloc(err, errlen+size);
      errlen += snprintf(err+errlen, size, "%s%s", prefix, str);

      l = g_list_next(l);
    }

    if ( perr != NULL )
      *perr = err;
    ret = -1;
  }

  return ret;
}


void pattern_text_free(pattern_t *pattern)
{
  if ( pattern->d.text.re != NULL ) {
    pcre_free(pattern->d.text.re);
    pattern->d.text.re = NULL;
  }
  if ( pattern->d.text.hints != NULL ) {
    pcre_free(pattern->d.text.hints);
    pattern->d.text.hints = NULL;
  }
  if ( pattern->d.text.str != NULL ) {
    free(pattern->d.text.str);
    pattern->d.text.str = NULL;
  }
  if ( pattern->d.text.ovec != NULL ) {
    free(pattern->d.text.ovec);
    pattern->d.text.ovec = NULL;
  }
  pattern->d.text.ovec_n = 0;
  if ( pattern->d.text.mvec != NULL ) {
    free(pattern->d.text.mvec);
    pattern->d.text.mvec = NULL;
  }
}


void pattern_text_copy(pattern_t *pattern, pattern_t *pattern0)
{
  pattern_text_compile(pattern, NULL);
}


int pattern_text_str(pattern_t *pattern, char *buf, int size)
{
	int len = 0;

	if ( pattern->mode & PATTERN_MODE_INVERSE )
		len += snprintf(buf+len, size-len, " inverse");

	return len;
}


int pattern_text_diff(pattern_t *p1, pattern_t *p2)
{
	  return 0;
}
