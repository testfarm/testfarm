/****************************************************************************/
/* TestFarm                                                                 */
/* HTML Log generator                                                       */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 09-JUL-2002                                                    */
/****************************************************************************/

/*
 * $Revision: 772 $
 * $Date: 2007-10-11 15:58:52 +0200 (jeu., 11 oct. 2007) $
 */

#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <glib.h>

#include "useful.h"
#include "report_log.h"

#define BGCOLOR_BODY         "#C0C0C0"
#define BGCOLOR_CELL         "#EFEFE0"
#define BGCOLOR_WCONTINUE    "#FFFFCC"
#define BGCOLOR_WTIMEOUT     "#FFCC99"
#define BGCOLOR_PASSED       "#66FF99"
#define BGCOLOR_FAILED       "#FF6666"
#define BGCOLOR_INCONCLUSIVE "#F6CB69"

enum {
  COL_DATE=0,
  COL_TIME,
  COL_GLOBAL_TS,
  COL_INTERFACE,
  COL_LOCAL_TS,
  COL_TAG,
  COL_INFO,
  NCOLS
};


static char *strword(char **str)
{
  char *s1, *s2;

  s1 = strskip_spaces(*str);
  s2 = strskip_chars(s1);
  if ( *s2 != NUL )
    *(s2++) = NUL;

  *str = strskip_spaces(s2);
  return s1;
}

static void fput_cell(char *str, int bold, int href, FILE *f)
{
  fputs("<TD><TT>", f);
  if ( bold ) fputs("<B>", f);
  if ( href ) fprintf(f, "<A NAME=%s>", str);
  fputs(str, f);
  if ( href ) fputs("</A>", f);
  if ( bold ) fputs("</B>", f);
  fputs("</TT></TD>\n", f);
}


static void fput_tstamp(char *str, int bold, FILE *f)
{

  /* Format time stamps in seconds with decimal point */
  if ( (*str != '\0') && (*str != '*') ) {
    long long ts;

    if ( sscanf(str, "%lld", &ts) == 1 ) {
      char buf[20];

      snprintf(buf, sizeof(buf), "%lld.%06lld", ts/1000000, ts%1000000);
      str = buf;
    }
  }

  /* Display formated cell */
  fput_cell(str, bold, 0, f);
}


static char *verdict_bgcolor(char *str)
{
  char *bgcolor = BGCOLOR_CELL;
  char *p = strskip_chars(str);
  char c = *p;

  *p = NUL;

  if ( strcmp(str, "PASSED") == 0 )
    bgcolor = BGCOLOR_PASSED;
  else if ( strcmp(str, "FAILED") == 0 )
    bgcolor = BGCOLOR_FAILED;
  else if ( strcmp(str, "INCONCLUSIVE") == 0 )
    bgcolor = BGCOLOR_INCONCLUSIVE;

  *p = c;

  return bgcolor;
}


static FILE *report_html_open(char *html_dir, char *case_name, char *signature, int append)
{
  char *fname;
  FILE *f;

  /* Build target file name */
  fname = (char *) malloc(strlen(html_dir)+strlen(case_name)+8);
  sprintf(fname, "%s" G_DIR_SEPARATOR_S "%s.html", html_dir, case_name);

  /* Open target HTML file */
  f = fopen(fname, append ? "a":"w");
  if ( f == NULL ) {
    fprintf(stderr, "Cannot %s HTML Log file '%s'\n", append ? "append":"create", fname);
  }
  else {
    /* Dump HTML file header */
    if ( append ) {
      fprintf(f, "<BR/>\n");
    }
    fprintf(f, "<HTML>\n");
    if ( ! append ) {
      fprintf(f, "<HEAD>\n");
      fprintf(f, "<TITLE>TestFarm Log: %s</TITLE>\n", case_name);
      fprintf(f, "<meta name=\"signature\" content=\"LOG %s\">\n", signature);
      fprintf(f, "</HEAD>\n");
    }
    fprintf(f, "<BODY BGCOLOR=" BGCOLOR_BODY ">\n");
    fprintf(f, "<TABLE WIDTH=100%%>\n");
  }

  free(fname);

  return f;
}


static void report_html_close(FILE *f)
{
  if ( f == NULL )
    return;

  /* Dump HTML file footer */
  fputs("</TABLE>\n", f);
  fputs("</BODY>\n", f);
  fputs("</HTML>\n", f);

  fclose(f);
}


static int report_log_reach(FILE *flog)
{
  char *buf = NULL;
  int size = 0;
  int log_found = 0;

  while ( fgets2(flog, &buf, &size) >= 0 ) {
    if ( strcmp(buf, "<LOG>") == 0 ) {
      log_found = 1;
      break;
    }
  }

  if ( buf != NULL )
    free(buf);

  return log_found;
}


#define TYPE_WAIT -1
#define TYPE_OTHER 0
#define TYPE_CASE 1
#define TYPE_VERDICT 2


static int report_log_build_f(FILE *flog, char *html_dir, char *signature)
{
  GList *cases = NULL;
  char *buf = NULL;
  int size = 0;
  FILE *fhtml = NULL;
  char *bgcolor = BGCOLOR_BODY;

  while ( fgets2(flog, &buf, &size) > 0 ) {
    char *str[NCOLS];
    char *p = buf;
    int type = TYPE_OTHER;
    int i;

    /* Ignore XML stuffs / Check end of LOG section */
    if ( buf[0] == '<' ) {
      if ( strcmp(buf, "</LOG>") == 0 )
        break;
      else
        continue;
    }

    /* Get log line fields */
    for (i = 0; i < NCOLS-1; i++)
      str[i] = strword(&p);
    str[NCOLS-1] = p;

    /* Spot test case separation lines */
    if ( strcmp(str[COL_INTERFACE], "ENGINE") == 0 ) {
      if ( strcmp(str[COL_TAG], "CASE") == 0 )
        type = TYPE_CASE;
      else if ( strcmp(str[COL_TAG], "VERDICT") == 0 )
        type = TYPE_VERDICT;
      else if ( strcmp(str[COL_TAG], "WAIT") == 0 )
        type = TYPE_WAIT;
    }

    /* We enter a new test case... */
    if ( type == TYPE_CASE ) {
      char *name = str[COL_INFO];
      GList *l;

      /* Set default background color */
      bgcolor = BGCOLOR_CELL;

      /* Should the Test Case Log be appended to existing file ? */
      l = cases;
      while ( l != NULL ) {
	if ( strcmp(name, l->data) == 0 )
	  break;
	l = l->next;
      }

      if ( l == NULL ) {
	cases = g_list_append(cases, strdup(name));
      }

      /* Open a new HTML target file */
      report_html_close(fhtml);
      fhtml = report_html_open(html_dir, name, signature, (l != NULL));
    }
    else if ( type == TYPE_VERDICT ) {
      bgcolor = verdict_bgcolor(str[COL_INFO]);
    }

    if ( fhtml != NULL ) {
      char *bgcolor_save = bgcolor;

      /* Highlight WAIT TIMEOUT messages */
      if ( type == TYPE_WAIT ) {
        if ( strncmp(str[COL_INFO],"TIMEOUT ", 8) == 0 )
          bgcolor = BGCOLOR_WTIMEOUT;
        else if ( strncmp(str[COL_INFO],"CONTINUE ", 9) == 0 )
          bgcolor = BGCOLOR_WCONTINUE;
      }

      /* Dump fields to HTML target file */
      fprintf(fhtml, "<TR BGCOLOR=\"%s\">\n", bgcolor);
      for (i = 0; i < NCOLS; i++) {
	int bold = (type > TYPE_OTHER);

	if ( (i == COL_GLOBAL_TS) || (i == COL_LOCAL_TS) ) {
	  fput_tstamp(str[i], bold, fhtml);
	}
	else {
	  int href = (type == TYPE_CASE) && (i == COL_INFO);
	  fput_cell(str[i], bold, href, fhtml);
	}
      }
      fputs("</TR>\n", fhtml);

      bgcolor = bgcolor_save;
    }

    if ( type == 2 )
      bgcolor = BGCOLOR_BODY;
  }

  /* Close target HTML file */
  report_html_close(fhtml);

  /* Free input buffer */
  if ( buf != NULL )
    free(buf);

  /* Free the Test Case list */
  g_list_foreach(cases, (GFunc) free, NULL);
  g_list_free(cases);

  return 0;
}


int report_log_build(char *log_name, char *signature, int xml_required, char *html_dir)
{
  FILE *flog;
  int reached;
  int ret = -1;

  /* Open input log file */
  flog = fopen(log_name, "r");
  if ( flog == NULL ) {
    fprintf(stderr, "Cannot open Test Log file '%s'\n", log_name);
    return -1;
  }

  /* Reach the LOG section (if any) */
  reached = report_log_reach(flog);
  if ( (xml_required == 0) && (reached == 0) ) {
    rewind(flog);
    reached = 1;
  }

  /* Process HTML log generation */
  if ( reached ) {
    ret = report_log_build_f(flog, html_dir, signature);
  }

  /* Close input log file */
  fclose(flog);

  return ret;
}
