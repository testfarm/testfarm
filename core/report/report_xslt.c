/****************************************************************************/
/* TestFarm                                                                 */
/* Test Report Generator - XSLT based generation                            */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 28-APR-2004                                                    */
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
#include <string.h>
#include <malloc.h>
#include <errno.h>
#include <unistd.h>
#include <wait.h>
#include <glib.h>
#include <sys/types.h>
#include <libxml/parser.h>

#include "useful.h"
#include "report_config.h"
#include "report.h"


#define HTML_INDEX "index.html"


static int filecopy(char *source, char *target)
{
  int ret = 0;
  FILE *f1, *f2;

  f1 = fopen(source, "r");
  if ( f1 == NULL ) {
    fprintf(stderr, "%s: Error opening file: %s\n", source, strerror(errno));
    ret = -1;
  }
  else {
    f2 = fopen(target, "w");
    if ( f2 == NULL ) {
      fprintf(stderr, "%s: Error creating file: %s\n", target, strerror(errno));
      ret = -1;
    }
    else {
      char buf[BUFSIZ];

      while ( ! feof(f1) ) {
        long size = fread(buf, 1, sizeof(buf), f1);
        if ( size < 0 ) {
          fprintf(stderr, "%s: Error reading file: %s\n", source, strerror(errno));
          ret = -1;
        }
        else if ( size > 0 ) {
          if ( fwrite(buf, 1, size, f2) < 0 ) {
            fprintf(stderr, "%s: Error writing file: %s\n", target, strerror(errno));
            ret = -1;
          }
        }
      }

      fclose(f2);
    }

    fclose(f1);
  }

  return ret;
}


#define CTX(_ptr_) ((ctx_t *)(_ptr_))

typedef struct {
  char **tab;
  int size;
  int index;
} ctx_t;

static void _startElement(void *ctx, const xmlChar *name, const xmlChar **attrs)
{
  if ( CTX(ctx)->index >= CTX(ctx)->size ) {
    (CTX(ctx)->size)++;
    CTX(ctx)->tab = realloc(CTX(ctx)->tab, sizeof(char *) * (CTX(ctx)->size));
  }

  if ( CTX(ctx)->index >= 0 ) {
    CTX(ctx)->tab[(CTX(ctx)->index)++] = strdup((char *) name);
  }
}

static void _endElement(void *ctx, const xmlChar *name)
{
  (CTX(ctx)->index)--;
}


static xmlSAXHandler handlers = {
  startElement:  _startElement,
  endElement:    _endElement,
};


static int xml_recover(char *filename)
{
  int ret = 0;
  FILE *f;
  char buf[BUFSIZ];
  int size;
  ctx_t ctx;

  /* Open XML file */
  f = fopen(filename, "r+");
  if ( f == NULL ) {
    fprintf(stderr, "%s: Error opening file: %s\n", filename, strerror(errno));
    return -1;
  }

  ctx.tab = NULL;
  ctx.size = 0;
  ctx.index = 0;

  /* Allocate XML SAX parser */
  size = fread(buf, 1, 4, f);
  if ( size < 0 ) {
    fprintf(stderr, "%s: Error reading file: %s\n", filename, strerror(errno));
    ret = -1;
  }
  else {
    xmlParserCtxtPtr xml = xmlCreatePushParserCtxt(&handlers, &ctx, buf, size, filename);

    while ( ! feof(f) ) {
      size = fread(buf, 1, sizeof(buf), f);
      if ( size < 0 ) {
        fprintf(stderr, "%s: Error reading file: %s\n", filename, strerror(errno));
        ret = -1;
        break;
      }
      if ( size > 0 ) {
        xmlParseChunk(xml, buf, size, 0);
      }
    }

    /* End XML parsing */
    xmlParseChunk(xml, buf, 0, 1);

    /* Free XML SAX parser */
    xmlFreeParserCtxt(xml);
  }

  if ( ret == 0 ) {
    if ( fseek(f, 0, SEEK_END) ) {
      fprintf(stderr, "%s: Error seeking file: %s\n", filename, strerror(errno));
      ret = -1;
    }
    else {
      while ( ctx.index > 0 ) {
        ctx.index--;
        fprintf(f, "</%s>\n", ctx.tab[ctx.index]);
      }
    }
  }

  fclose(f);

  return ret;
}


static char *report_xslt_filename = NULL;

static char *report_xslt_recover(char *out_name)
{
  int complete = 0;
  FILE *f;

  /* Open input file */
  f = fopen(out_name, "r");
  if ( f == NULL ) {
    fprintf(stderr, "%s: Error opening Test Output file: %s\n", out_name, strerror(errno));
    return NULL;
  }

  /* Check for file completion */
  if ( fseek(f, -20, SEEK_END) ) {
    fprintf(stderr, "%s: Error seeking Test Output file: %s\n", out_name, strerror(errno));
  }
  else {
    char *buf = NULL;
    int size = 0;

    /* Look for last XML element */
    while ( !complete && !feof(f) ) {
      int len = fgets2(f, &buf, &size);

      if ( len < 0 )
        break;

      if ( (len > 0) && (buf != NULL) ) {
        if ( strcmp(strskip_spaces(buf), "</RESULT>") == 0 )
          complete = 1;
      }
    }

    if ( buf != NULL )
      free(buf);
  }

  /* Close input file */
  fclose(f);

  if ( complete )
    return out_name;

  /* Setup name of recovered file */
  report_xslt_filename = (char *) realloc(report_xslt_filename, strlen(out_name)+16);
  sprintf(report_xslt_filename, "%s.tmp%d", out_name, getpid());

  if ( filecopy(out_name, report_xslt_filename) )
    return NULL;

  if ( xml_recover(report_xslt_filename) )
    return NULL;

  return report_xslt_filename;
}


int report_xslt(char *out_name, char *signature,
                report_config_t *rc, char *html_dir)
{
  int ret;
  char *filename;
  char *html_name = NULL;
  char *signature_buf = NULL;
  char *argv[1+3+3+2+1+1+1];
  int argc;
  pid_t pid;
  int status;
  int size;

  /* Check for uncomplete Test Output file */
  filename = report_xslt_recover(out_name);
  if ( filename == NULL )
    return -1;

  /* Set HTML file name */
  size = strlen(html_dir) + strlen(HTML_INDEX) + 2;
  html_name = (char *) malloc(size);
  snprintf(html_name, size, "%s" G_DIR_SEPARATOR_S HTML_INDEX, html_dir);

  /* Setup program arguments */
  argc = 0;
  argv[argc++] = "xsltproc";

  if ( rc->fname != NULL ) {
    argv[argc++] = "--stringparam";
    argv[argc++] = "report-config";
    argv[argc++] = rc->fname;
  }

  if ( signature != NULL ) {
    signature_buf = (char *) malloc(strlen(signature)+20);
    sprintf(signature_buf, "'REPORT %s'", signature);

    argv[argc++] = "--param";
    argv[argc++] = "signature";
    argv[argc++] = signature_buf;
  }

  argv[argc++] = "-o";
  argv[argc++] = html_name;
  argv[argc++] = report_config_get_stylesheet(rc);
  argv[argc++] = filename;

  argv[argc] = NULL;

  //for (ret = 0; ret < argc; ret++)
  //  fprintf(stderr, "%s ", argv[ret]);
  //fprintf(stderr, "\n");

  /* Spawn child */
  ret = -1;
  pid = fork();
  switch ( pid ) {
  case 0: /* Child */
    execvp(argv[0], argv);

    /* Return from exec: something went wrong */
    fprintf(stderr, "execvp(%s): %s\n", argv[0], strerror(errno));
    exit(99);
    break;

  case -1: /* Error */
    fprintf(stderr, "Cannot fork XSLT processor: %s\n", strerror(errno));
    break;

  default : /* Parent */
    waitpid(pid, &status, 0);

    if ( WIFEXITED(status) )
      ret = WEXITSTATUS(status);

    break;
  }

  /* Free buffers */
  if ( html_name != NULL )
    free(html_name);
  html_name = NULL;

  if ( signature_buf != NULL )
    free(signature_buf);
  signature_buf = NULL;

  if ( filename != out_name )
     remove(filename);

  return ret;
}
