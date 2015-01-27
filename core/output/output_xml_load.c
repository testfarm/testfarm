/****************************************************************************/
/* TestFarm                                                                 */
/* Output XML file load                                                     */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 04-FEB-2004                                                    */
/****************************************************************************/

/*
 * $Revision: 772 $
 * $Date: 2007-10-11 15:58:52 +0200 (jeu., 11 oct. 2007) $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <libxml/parser.h>

#include "useful.h"
#include "output.h"
#include "output_node.h"
#include "output_xml.h"
#include "output_xml_load.h"


#define _IDLE        0
#define _ROOT        1
#define _SEQ         2
#define _CASE        3
#define _STDIO       4
#define _VERDICT     5
#define _DESCRIPTION 6
#define _REFERENCE   7
#define _OUTPUT      8
#define _OPERATOR    9
#define _RELEASE     10
#define _VALIDATED   11
#define _UNKNOWN     12

#define CTX(_ptr_) ((ctx_t *)(_ptr_))

typedef struct {
  int element;
  output_sequence_t *oseq;
  output_case_t *ocase;
} stack_item_t;

typedef struct {
  output_t *out;
  output_sequence_t *oseq;
  output_case_t *ocase;

  long offset;

  stack_item_t *stack;
  int stack_size;
  int stack_ptr;
  int finished;
} ctx_t;


static int state(ctx_t *ctx)
{
  return (ctx->stack_ptr >= 0) ? ctx->stack[ctx->stack_ptr].element : _IDLE;
}


static int parent(ctx_t *ctx)
{
  return (ctx->stack_ptr > 0) ? ctx->stack[ctx->stack_ptr-1].element : _IDLE;
}

static void push(ctx_t *ctx, int v)
{
  (ctx->stack_ptr)++;

  if ( ctx->stack_ptr >= ctx->stack_size ) {
    ctx->stack_size += 10;
    ctx->stack = (stack_item_t *) realloc(ctx->stack, ctx->stack_size * sizeof(stack_item_t));
  }

  if ( ctx->stack_ptr >= 0 ) {
    ctx->stack[ctx->stack_ptr].element = v;
    ctx->stack[ctx->stack_ptr].oseq = CTX(ctx)->oseq;
    ctx->stack[ctx->stack_ptr].ocase = CTX(ctx)->ocase;
  }
}

static void pop(ctx_t *ctx)
{
  (ctx->stack_ptr)--;
  if ( ctx->stack_ptr >= 0 ) {
    CTX(ctx)->oseq = ctx->stack[ctx->stack_ptr].oseq;
    CTX(ctx)->ocase = ctx->stack[ctx->stack_ptr].ocase;
  }
  else {
    CTX(ctx)->oseq = NULL;
    CTX(ctx)->ocase = NULL;
  }
}


static char *get_attr(xmlChar **attrs, char *attr)
{
  if ( attrs == NULL )
    return NULL;

  while ( *attrs != NULL ) {
    if ( strcmp((char *) attrs[0], attr) == 0 )
      return (char *) attrs[1];
    attrs += 2;
  }

  return "";
}


static void _startDocument(void *ctx)
{
  push(CTX(ctx), _ROOT);
}


static void _endDocument(void *ctx)
{
  pop(CTX(ctx));
  CTX(ctx)->finished = 1;
}


static void _startElement(void *ctx, const xmlChar *name, const xmlChar **attrs)
{
  output_t *out = CTX(ctx)->out;
  output_sequence_t *oseq = CTX(ctx)->oseq;
  output_case_t *ocase = CTX(ctx)->ocase;

  if ( strcmp((char *) name, "SEQ") == 0 ) {
    /* Get Sequence attributes */
    char *id = get_attr((xmlChar **) attrs, "id");
    char *scenario = get_attr((xmlChar **) attrs, "scenario");

    /* Create new sequence if not already done */
    if ( output_sequence_find(out->sequences, id) == NULL ) {
      CTX(ctx)->oseq = oseq = output_sequence_alloc(id, NULL, (strcmp(scenario, "yes") == 0));
      out->sequences = g_list_append(out->sequences, oseq);
    }

    push(CTX(ctx), _SEQ);
  }
  else if ( strcmp((char *) name, "CASE") == 0 ) {
    /* Get Test Case attributes */
    char *id = get_attr((xmlChar **) attrs, "id");
    char *date = get_attr((xmlChar **) attrs, "date");
    char *criticity = get_attr((xmlChar **) attrs, "criticity");

    /* Create new Test Case */
    CTX(ctx)->ocase = ocase = output_case_alloc(id, NULL);
    ocase->offset = CTX(ctx)->offset;
    out->cases = g_list_append(out->cases, ocase);

    /* Attach Test Case to parent Sequence */
    if ( oseq != NULL ) {
      ocase->seq = oseq;
      oseq->cases = g_list_append(oseq->cases, ocase);
    }

    /* Set Test Case execution info */
    sscanf(date, "%lld", &(ocase->begin));
    ocase->info.criticity = atoi(criticity);

    push(CTX(ctx), _CASE);
  }
  else if ( strcmp((char *) name, "STDIO") == 0 ) {
    push(CTX(ctx), _STDIO);
  }
  else if ( strcmp((char *) name, "VALIDATED") == 0 ) {
    /* Get Test Case attributes */
    char *md5sum = get_attr((xmlChar **) attrs, "md5sum");
    char *localtime = get_attr((xmlChar **) attrs, "localtime");
    char *operator = get_attr((xmlChar **) attrs, "operator");

    if ( ocase->validate == NULL ) {
      validate_t *v = validate_alloc();

      out->validates = g_list_append(out->validates, v);
      ocase->validate = v;

      v->md5sum = strdup(md5sum);
      v->date = strdup(localtime);
      v->operator = strdup(operator);
    }

    push(CTX(ctx), _VALIDATED);
  }
  else if ( strcmp((char *) name, "VERDICT") == 0 ) {
    /* Get Verdict attributes */
    char *date = get_attr((xmlChar **) attrs, "date");
    char *criticity = get_attr((xmlChar **) attrs, "criticity");

    if ( ocase != NULL ) {
      sscanf(date, "%lld", &(ocase->end));
      ocase->info.elapsed = ocase->end - ocase->begin;
      if ( *criticity != '\0' )
        ocase->info.criticity = atoi(criticity);
    }

    push(CTX(ctx), _VERDICT);
  }
  else if ( strcmp((char *) name, "DESCRIPTION") == 0 ) {
    push(CTX(ctx), _DESCRIPTION);
  }
  else if ( strcmp((char *) name, "REFERENCE") == 0 ) {
    push(CTX(ctx), _REFERENCE);
  }
  else if ( strcmp((char *) name, "OUTPUT") == 0 ) {
    /* Get Document attributes */
    char *id = get_attr((xmlChar **) attrs, "id");
    char *date = get_attr((xmlChar **) attrs, "date");
    char *nscenario = get_attr((xmlChar **) attrs, "nscenario");
    char *ncase = get_attr((xmlChar **) attrs, "ncase");

    /* Set tree name */
    output_tree_name(&(out->tree), id);

    /* Set initial date */
    sscanf(date, "%lld", &(out->tree.begin));

    /* Set total number of Scenarii and Test Cases */
    out->stat.nscenario = atoi(nscenario);
    out->stat.ncase = atoi(ncase);

    push(CTX(ctx), _OUTPUT);
  }
  else if ( strcmp((char *) name, "OPERATOR") == 0 ) {
    push(CTX(ctx), _OPERATOR);
  }
  else if ( strcmp((char *) name, "RELEASE") == 0 ) {
    push(CTX(ctx), _RELEASE);
  }
  else {
    push(CTX(ctx), _UNKNOWN);
  }
}


static void _endElement(void *ctx, const xmlChar *name)
{
  pop(CTX(ctx));
  if ( strcmp((char *) name, "OUTPUT") == 0 )
    CTX(ctx)->finished = 1;
}


static void _characters(void *ctx, const xmlChar *value, int len)
{
  output_t *out = CTX(ctx)->out;
  output_sequence_t *oseq = CTX(ctx)->oseq;
  output_case_t *ocase = CTX(ctx)->ocase;
  char str[len+1];
  char *p;

  /* Remove trailing blanks */
  while ( (len > 0) && (value[len-1] <= ' ') )
    len--;

  /* Ignore blank lines */
  if ( len <= 0 )
    return;

  /* Remove leading blanks */
  p = (char *) value;
  while ( (len > 0) && (*p <= ' ') ) {
    p++;
    len--;
  }

  /* Get element content */
  memcpy(str, p, len);
  str[len] = NUL;
  strxml(str);

  switch ( state(CTX(ctx)) ) {
  case _VERDICT:
    if ( ocase != NULL ) {
      long elapsed_time = ocase->end - out->tree.begin;
      ocase->info.verdict = atoi(str);
      output_stat_elapsed(&(out->stat), elapsed_time), 
      output_stat_verdict(&(out->stat), ocase->info.verdict, ocase->info.criticity);
    }
    break;

  case _DESCRIPTION:
    switch ( parent(CTX(ctx)) ) {
    case _OUTPUT:
      out->tree.description = strdup(str);
      break;
    case _SEQ:
      if ( oseq != NULL )
        output_info_description(&(oseq->info), str);
      break;
    case _CASE:
      if ( ocase != NULL )
        output_info_description(&(ocase->info), str);
      break;
    }
    break;

  case _REFERENCE:
    switch ( parent(CTX(ctx)) ) {
    case _OUTPUT:
      out->tree.reference = strdup(str);
      break;
    case _SEQ:
      if ( oseq != NULL )
        output_info_reference(&(oseq->info), str);
      break;
    case _CASE:
      if ( ocase != NULL )
        output_info_reference(&(ocase->info), str);
      break;
    }
    break;

  case _OPERATOR:
    switch ( parent(CTX(ctx)) ) {
    case _OUTPUT:
      output_tree_operator(&(out->tree), str);
      break;
    }
    break;

  case _RELEASE:
    switch ( parent(CTX(ctx)) ) {
    case _OUTPUT:
      output_tree_release(&(out->tree), str);
      break;
    }
    break;

  case _VALIDATED:
    if ( ocase != NULL ) {
      int level = atoi(str);

      ocase->info.validated = (level == validate_max);
      if ( ocase->validate != NULL )
        ocase->validate->level = level;
    }
    break;
  }
}


static xmlSAXHandler handlers = {
  startDocument: _startDocument,
  endDocument:   _endDocument,
  startElement:  _startElement,
  endElement:    _endElement,
  characters:    _characters,
};


output_t *output_xml_load(char *filename)
{
  FILE *f;
  ctx_t ctx;
  xmlParserCtxtPtr xml;
  int size = 0;
  char *buf = NULL;
  int len;

  /* Open output dump file for read */
  if ( (f = fopen(filename, "r")) == NULL )
    return NULL;

  /* Load XML first line */
  len = fgets2(f, &buf, &size);
  if ( len < 0 ) {
    fclose(f);
    return NULL;
  }

  /* Allocate output file descriptor */
  ctx.out = output_alloc(NULL);
  ctx.oseq = NULL;
  ctx.ocase = NULL;
  ctx.offset = 0;
  ctx.stack = NULL;
  ctx.stack_size = 0;
  ctx.stack_ptr = -1;
  ctx.finished = 0;

  /* Set file name */
  output_set_filename(ctx.out, filename);

  /* Allocate XML SAX parser */
  xml = xmlCreatePushParserCtxt(&handlers, &ctx, buf, len, filename);

  /* Parse output dump file headers */
  while ( (! feof(f)) && (! ctx.finished) ) {
    /* Load XML file line */
    len = fgets2(f, &buf, &size);
    if ( len < 0 )
      break;

    /* Get current line offset */
    ctx.offset = ftell(f);

    if ( state(&ctx) == _STDIO ) {
      char *s = strskip_spaces(buf);
      if ( *s != '<' ) {
        len = 0;
      }
    }

    if ( len > 0 )
      xmlParseChunk(xml, buf, len, 0);
  }

  /* End XML parsing */
  xmlParseChunk(xml, buf, 0, 1);

  /* Free XML SAX parser */
  xmlFreeParserCtxt(xml);

  /* Free work buffer */
  if ( buf != NULL )
    free(buf);

  /* Close output dump file */
  fclose(f);

  return ctx.out;
}
