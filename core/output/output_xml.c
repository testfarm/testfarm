/****************************************************************************/
/* TestFarm                                                                 */
/* Output file, XML format                                                  */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 29-JAN-2004                                                    */
/****************************************************************************/

/*
 * $Revision: 1132 $
 * $Date: 2010-04-02 17:30:55 +0200 (ven., 02 avril 2010) $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <errno.h>
#include <fcntl.h>
#include <glib.h>
#include <time.h>
#include <sys/time.h>

#include "install.h"
#include "useful.h"
#include "codegen.h"
#include "validate.h"
#include "output_xml.h"


/*---------------------------------------------------*/
/* Output XML Setup                                  */
/*---------------------------------------------------*/

output_xml_t *output_xml_alloc(void)
{
  output_xml_t *ox = (output_xml_t *) malloc(sizeof(output_xml_t));
  ox->file.type = OUTPUT_FILE_XML;
  ox->file.name = NULL;
  ox->file.f = NULL;
  ox->cur_obj = NULL;
  ox->indent = 0;
  ox->channel = OUTPUT_CHANNEL_NONE;

  return ox;
}


void output_xml_free(output_xml_t *ox)
{
  if ( ox == NULL )
    return;

  output_xml_close(ox);
  output_xml_filename(ox, NULL);
  free(ox);
}


void output_xml_filename(output_xml_t *ox, char *filename)
{
  if ( ox == NULL )
    return;
  output_file_name(&(ox->file), filename);
}


/*---------------------------------------------------*/
/* Output XML Write operations                       */
/*---------------------------------------------------*/

static void output_xml_indent(output_xml_t *ox)
{
  int i;

  for (i = 0; i < ox->indent; i++)
    fprintf(ox->file.f, "  ");
}


static char *output_xml_tag(tree_object_t *obj)
{
  char *tag;

  switch ( obj->type ) {
  case TYPE_CASE:
    tag = "CASE";
    break;
  case TYPE_SEQ:
    if ( obj->parent_seq == NULL )
      tag = "OUTPUT";
    else
      tag = "SEQ";
    break;
  default:
    tag = "UNKNOWN";
    break;
  }

  return tag;
}


static void output_xml_cdata_write(FILE *f, char c)
{
  switch ( c ) {
  case '<':
    fputs("&lt;", f);
    break;
  case '&':
    fputs("&amp;", f);
    break;
  case '>':
    fputs("&gt;", f);
    break;
#if 0
  case '\'':
    fputs("&apos;", f);
    break;
  case '"':
    fputs("&quot;", f);
    break;
#endif
  default:
    fputc(c, f);
    break;
  }
}


static void output_xml_cdata_raw(FILE *f, char *str)
{
  while ( *str != '\0' ) {
    output_xml_cdata_write(f, *(str++));
  }
}


static void output_xml_cdata(output_xml_t *ox, char *str)
{
  char c = '\n';

  (ox->indent)++;

  while ( *str != '\0' ) {
    if ( c == '\n' )
      output_xml_indent(ox);

    c = *(str++);
    output_xml_cdata_write(ox->file.f, c);
  }

  (ox->indent)--;
}


static void output_xml_stdio_close(output_xml_t *ox)
{
  if ( ox->channel == OUTPUT_CHANNEL_NONE )
    return;

  output_xml_indent(ox);
  fprintf(ox->file.f, "</STDIO>\n");

  ox->channel = OUTPUT_CHANNEL_NONE;
}


int output_xml_stdio(output_xml_t *ox, int channel, char *str)
{
  char *attr;

  if ( ox == NULL )
    return -1;
  if ( ox->file.f == NULL )
    return -2;

  /* Close previously open channel */
  if ( ox->channel != channel ) {
    /* Close stdio section if any */
    output_xml_stdio_close(ox);

    ox->channel = channel;

    switch ( channel ) {
    case OUTPUT_CHANNEL_STDIN:      attr = "in";        break;
    case OUTPUT_CHANNEL_STDOUT:     attr = "out";        break;
    case OUTPUT_CHANNEL_STDERR:     attr = "err";        break;
    case OUTPUT_CHANNEL_IN_TITLE:   attr = "in_title";   break;
    case OUTPUT_CHANNEL_IN_HEADER:  attr = "in_header";  break;
    case OUTPUT_CHANNEL_IN_VERDICT: attr = "in_verdict"; break;
    default:                        attr = "unknown";    break;
    }

    output_xml_indent(ox);
    fprintf(ox->file.f, "<STDIO channel=\"%s\">\n", attr);
  }

  output_xml_cdata_raw(ox->file.f, str);
  fprintf(ox->file.f, "\n");

  fflush(ox->file.f);

  return 0;
}


static void output_xml_date(output_xml_t *ox)
{
  struct timeval date;
  time_t t;
  char str[32];

  gettimeofday(&date, NULL);
  fprintf(ox->file.f, " date=\"%ld%03ld\"", date.tv_sec, date.tv_usec / 1000);

  t = date.tv_sec;
  strftime(str, sizeof(str), "%d-%b-%Y %H:%M:%S", localtime(&t));
  fprintf(ox->file.f, " localtime=\"%s\"", str);
}


static void output_xml_begin(output_xml_t *ox, tree_object_t *obj)
{
  output_xml_indent(ox);
  fprintf(ox->file.f, "<%s id=\"%s\"", output_xml_tag(obj), obj->parent_item->name);

  if ( obj->type == TYPE_CASE ) {
    output_xml_date(ox);

    if ( obj->d.Case->criticity != CRITICITY_NONE )
      fprintf(ox->file.f, " criticity=\"%d\"", obj->d.Case->criticity);
    fprintf(ox->file.f, " exec=\"%lu\"", obj->d.Case->exec_count);
  }
  else if ( obj->type == TYPE_SEQ ) {
    if ( obj->parent_seq == NULL ) {
      int ncase, nscenario;

      output_xml_date(ox);

      tree_count(obj->parent_item->parent_tree, &ncase, &nscenario);
      fprintf(ox->file.f, " nscenario=\"%d\"", nscenario);
      fprintf(ox->file.f, " ncase=\"%d\"", ncase);
    }
    else {
      if ( ! tree_object_has_seq(obj) )
        fprintf(ox->file.f, " scenario=\"yes\"");
    }
  }

  fprintf(ox->file.f, ">\n");
  (ox->indent)++;

  if ( obj->parent_item->comment != NULL ) {
    output_xml_indent(ox);
    fprintf(ox->file.f, "<DESCRIPTION>\n");

    output_xml_cdata(ox, obj->parent_item->comment);
    fprintf(ox->file.f, "\n");

    output_xml_indent(ox);
    fprintf(ox->file.f, "</DESCRIPTION>\n");
  }

  if ( obj->parent_item->reference != NULL ) {
    output_xml_indent(ox);
    fprintf(ox->file.f, "<REFERENCE>\n");

    output_xml_cdata(ox, obj->parent_item->reference);
    fprintf(ox->file.f, "\n");

    output_xml_indent(ox);
    fprintf(ox->file.f, "</REFERENCE>\n");
  }
}


static void output_xml_end(output_xml_t *ox, tree_object_t *obj)
{
  /* Close stdio section if any */
  output_xml_stdio_close(ox);

  (ox->indent)--;
  output_xml_indent(ox);
  fprintf(ox->file.f, "</%s>  <!-- %s -->\n", output_xml_tag(obj), obj->parent_item->name);

  fflush(ox->file.f);
}


void output_xml_close(output_xml_t *ox)
{
  if ( ox == NULL )
    return;
  if ( ox->file.f == NULL )
    return;

  output_xml_stdio_close(ox);

  if ( ox->cur_obj != NULL ) {
    while ( ox->cur_obj != NULL ) {
      output_xml_end(ox, ox->cur_obj);
      ox->cur_obj = ox->cur_obj->parent_seq;
    }
  }
  else {
    fprintf(ox->file.f, "</OUTPUT>\n");
  }

  fprintf(ox->file.f, "</RESULT>\n");
  fclose(ox->file.f);
  ox->file.f = NULL;
}


int output_xml_open(output_xml_t *ox, output_tree_t *tree)
{
  char *home;

  if ( ox == NULL )
    return -1;

  /* Check file is defined */
  if ( ox->file.name == NULL )
    return -2;

  /* Check file is not already open */
  if ( ox->file.f != NULL )
    return -3;

  /* Open file */
  ox->file.f = fopen(ox->file.name, "w");
  if ( ox->file.f == NULL ) {
    fprintf(stderr, "*PANIC* Cannot open XML output file '%s': %s\n", ox->file.name, strerror(errno));
    return -4;
  }

  /* Child programs should not have access this file descriptor */
  fcntl(fileno(ox->file.f), F_SETFD, FD_CLOEXEC);

  /* Get install directory */
  home = get_home();

  /* Write XML header */
  ox->indent = 0;
  fprintf(ox->file.f, "<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\n");
  fprintf(ox->file.f, "<!DOCTYPE OUTPUT SYSTEM \"%s/lib/output.dtd\">\n", home);
  fprintf(ox->file.f, "<?xml-stylesheet type=\"text/xsl\" href=\"%s/lib/output.xsl\"?>\n", home);
  fprintf(ox->file.f, "<RESULT>\n");

  output_xml_begin(ox, tree->tree->head->object);

  if ( tree->operator != NULL ) {
    output_xml_indent(ox);
    fprintf(ox->file.f, "<OPERATOR>%s</OPERATOR>\n", tree->operator);
  }

  if ( tree->release != NULL ) {
    output_xml_indent(ox);
    fprintf(ox->file.f, "<RELEASE>%s</RELEASE>\n", tree->release);
  }

  ox->cur_obj = NULL;

  return 0;
}


static tree_object_t *output_xml_common_ancestor(tree_object_t *obj1, tree_object_t *obj2)
{
  while ( obj1 != NULL ) {
    tree_object_t *cur = obj2;

    obj1 = obj1->parent_seq;

    while ( cur != NULL ) {
      cur = cur->parent_seq;
      if ( cur == obj1 )
        return cur;
    }
  }

  return NULL;
}


static void output_xml_write_sequence(tree_object_t *object, output_xml_t *ox)
{
  output_xml_begin(ox, object);
}


int output_xml_case(output_xml_t *ox, tree_object_t *object)
{
  tree_object_t *anc;
  tree_object_t *cur;
  GList *list;

  if ( ox == NULL )
    return -1;
  if ( ox->file.f == NULL )
    return -2;

  /* Only Test Cases */
  if ( object->type != TYPE_CASE )
    return 0;

  /* Close stdio section if any */
  output_xml_stdio_close(ox);

  /* Close previous test case */
  if ( ox->cur_obj != NULL ) {
    output_xml_end(ox, ox->cur_obj);
  }

  /* Retrieve the common ancestor between this object and the previous one */
  anc = output_xml_common_ancestor(ox->cur_obj, object);
  if ( anc == NULL )
    anc = object->parent_item->parent_tree->head->object;

  //fprintf(stderr, "-- Common ancestor between %s and %s is %s\n",
  //        ox->cur_obj ? ox->cur_obj->parent_item->name : "(none)",
  //        object->parent_item->name, anc->parent_item->name);

  /* Go up to common ancestor */
  if ( ox->cur_obj != NULL ) {
    cur = ox->cur_obj->parent_seq;
    while ( cur != anc ) {
      output_xml_end(ox, cur);
      cur = cur->parent_seq;
    }
  }

  list = NULL;
  cur = object->parent_seq;
  while ( cur != anc ) {
    list = g_list_prepend(list, cur);
    cur = cur->parent_seq;
  }
  g_list_foreach(list, (GFunc) output_xml_write_sequence, ox);
  g_list_free(list);

  output_xml_begin(ox, object);
  ox->cur_obj = object; 

  return ftell(ox->file.f);
}


int output_xml_validated(output_xml_t *ox, validate_t *v)
{
  if ( ox == NULL )
    return -1;
  if ( ox->file.f == NULL )
    return -2;

  /* Close stdio section if any */
  output_xml_stdio_close(ox);

  output_xml_indent(ox);
  fprintf(ox->file.f, "<VALIDATED md5sum=\"%s\" localtime=\"%s\" operator=\"%s\">%d</VALIDATED>\n",
          v->md5sum, v->date, v->operator, v->level);

  return 0;
}


int output_xml_verdict(output_xml_t *ox, int verdict, int criticity)
{
  if ( ox == NULL )
    return -1;
  if ( ox->file.f == NULL )
    return -2;

  /* Close stdio section if any */
  output_xml_stdio_close(ox);

  output_xml_indent(ox);
  fprintf(ox->file.f, "<VERDICT");
  output_xml_date(ox);

  if ( criticity > 0 )
    fprintf(ox->file.f, " criticity=\"%d\"", criticity);
  fprintf(ox->file.f, ">%d</VERDICT>\n", verdict);

  fflush(ox->file.f);

  return 0;
}


/*---------------------------------------------------*/
/* Output XML Read operations                        */
/*---------------------------------------------------*/

static int output_xml_channel(char *str)
{
  int channel = OUTPUT_CHANNEL_NONE;

  if ( str != NULL ) {
    if ( strcmp(str, "in") == 0 )
      channel = OUTPUT_CHANNEL_STDIN;
    else if ( strcmp(str, "out") == 0 )
      channel = OUTPUT_CHANNEL_STDOUT;
    else if ( strcmp(str, "err") == 0 )
      channel = OUTPUT_CHANNEL_STDERR;
    else if ( strcmp(str, "in_title") == 0 )
      channel = OUTPUT_CHANNEL_IN_TITLE;
    else if ( strcmp(str, "in_header") == 0 )
      channel = OUTPUT_CHANNEL_IN_HEADER;
    else if ( strcmp(str, "in_verdict") == 0 )
      channel = OUTPUT_CHANNEL_IN_VERDICT;
  }

  return channel;
}


int output_xml_read(output_xml_t *ox, output_case_t *ocase,
                    output_file_func_t *func, void *arg,
                    char *filter)
{
  FILE *f;
  int size = 0;
  char *buf = NULL;
  int finished = 0;
  int channel = OUTPUT_CHANNEL_NONE;

  if ( ox == NULL )
    return -1;
  if ( func == NULL )
    return 0;
  if ( ox->file.type != OUTPUT_FILE_XML )
    return -1;
  if ( ox->file.name == NULL )
    return -1;

  /* Check for empty Test Case */
  if ( (ocase != NULL) && (ocase->offset == 0) )
    return 0;

  /* Open output dump file */
  if ( (f = fopen(ox->file.name, "r")) == NULL )
    return 0;

  /* Seek Test Case position in output dump file */
  if ( ocase != NULL )
    fseek(f, ocase->offset, SEEK_SET);

  /* Process each line until we meet another test case */
  while ( (!feof(f)) && (! finished) ) {
    char *s1, *s2;

    if ( fgets2(f, &buf, &size) < 0 )
      break;

    s1 = strskip_spaces(buf);

    /* An XML tag */
    if ( s1[0] == '<' ) {
      s1++;

      switch ( s1[0] ) {
      case '/':
        channel = OUTPUT_CHANNEL_NONE;
        break;

      case 'S':
        s1++;
        if ( strncmp(s1, "TDIO ", 5) )
          break;
        s1 += 5;

        while ( (*s1 != NUL) && (*s1 <= ' ') )
          s1++;

        /* Seek channel start quote */
        if ( strncmp(s1, "channel=\"", 9) )
          break;
        s1 += 9;

        /* Seek channel stop quote */
        s2 = s1;
        while ( (*s2 != NUL) && (*s2 != '"') )
          s2++;
        *s2 = NUL;

        channel = output_xml_channel(s1);
        break;

      case 'C':
        if ( strncmp(s1+1, "ASE ", 4) == 0 )
          finished = 1;
        break;
      }
    }

    /* Output characters */
    else if ( channel != OUTPUT_CHANNEL_NONE ) {
      strxml(buf);

      if ( filter == NULL ) {
        func(arg, channel, buf);
      }
      else {
        int filter_len = strlen(filter);
        if ( strncmp(buf, filter, filter_len) == 0 )
          func(arg, channel, buf + filter_len);
      }
    }
  }

  /* Free work buffer */
  if ( buf != NULL )
    free(buf);

  fclose(f);

  return 0;
}


/*---------------------------------------------------*/
/* Output merge with log file                        */
/*---------------------------------------------------*/

int output_xml_merge(output_xml_t *ox, char *log_name)
{
  FILE *fout;
  char *buf = NULL;
  int size = 0;
  long offset;
  int ret = 0;

  if ( ox == NULL )
    return -1;
  if ( log_name == NULL )
    return -1;

  /* Open output file */
  fout = fopen(ox->file.name, "r+");
  if ( fout == NULL )
    return -1;

  fseek(fout, -20, SEEK_END);

  /* Reach end of output file */
  offset = -1;
  while ( (offset < 0) && (!feof(fout)) ) {
    char *line;

    offset = ftell(fout);

    if ( fgets2(fout, &buf, &size) < 0 )
      break;

    line = strskip_spaces(buf);
    if ( strncmp(line, "</LOG", 5) == 0 )
      offset = 0;
    else if ( strcmp(line, "</RESULT>") != 0 )
      offset = -1;
  }

  /* Process merge if </RESULT> is reached and no <LOG> section is present */
  if ( offset > 0 ) {
    if ( fseek(fout, offset, SEEK_SET) ) {
      fprintf(stderr, "[LOG-MERGE] fseek(%s): %s\n", ox->file.name, strerror(errno));
      ret = -1;
    }
    else {
      FILE *flog = fopen(log_name, "r");

      if ( flog == NULL ) {
        //fprintf(stderr, "[LOG-MERGE] fopen(%s): %s\n", log_name, strerror(errno));
        ret = -1;
      }
      else {
        char buf[BUFSIZ];

        while ( (ret == 0) && (! feof(flog)) ) {
          int count = fread(buf, sizeof(char), BUFSIZ, flog);
          if ( count < 0 ) {
            fprintf(stderr, "[LOG-MERGE] fread(%s): %s\n", log_name, strerror(errno));
            ret = -1;
	    break;
          }

	  if ( fwrite(buf, sizeof(char), count, fout) < 0 ) {
	    fprintf(stderr, "[LOG-MERGE] fwrite(%s): %s\n", ox->file.name, strerror(errno));
	    ret = -1;
	    break;
	  }
        }

        fprintf(fout, "</RESULT>\n");
        fclose(flog);
      }
    }
  }

  /* Close output file */
  fclose(fout);

  /* Free buffer */
  if ( buf != NULL )
    free(buf);

  return ret;
}
