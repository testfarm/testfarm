/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* OCR Agent for text patterns                                              */
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

#define __TVU_OCR_C__

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <glib.h>
#include <libxml/parser.h>

#include "frame_geometry.h"
#include "agent.h"

typedef struct ocr_s ocr_t;
typedef struct ocr_ctx_s ocr_ctx_t;

#include "ocr.h"


#define dprintf(args...) //fprintf(stderr, "[DEBUG:OCR] " args);

#define OCR_LIB_DIR          "/usr/lib/testfarm-vu/ocr"
#define OCR_DEFAULT          "gocr"
#define OCR_LAUNCHER_SUFFIX  ".sh"


typedef struct {
  char *buf;              /* Character string */
  frame_geometry_t *map;  /* Character geometry mapping */
  int len;                /* Number of valid characters */
  int size;               /* Number of allocated elements */
} ocr_text_t;


typedef enum {
  OCR_STATE_IDLE=0,
  OCR_STATE_READ,
  OCR_STATE_END
} ocr_xml_state_t;

struct ocr_ctx_s {
  ocr_xml_state_t state;
  unsigned long addr;
  frame_geometry_t box;
  ocr_text_t text;
  ocr_reply_t *reply_func;
  void *reply_data;
};

#define CTX(_ptr_) ((ocr_ctx_t *)(_ptr_))


struct ocr_s {
  char *agent_name;               /* OCR agent name */
  char *agent_launcher;           /* OCR agent launcher script */
  int agent_argc;
  char **agent_argv;
  int shmid;                /* Frame buffer shmid */
  agent_t *agent;           /* OCR agent */
  xmlParserCtxtPtr xml;     /* OCR result parsing context */
  ocr_ctx_t ctx;
};


static void ocr_text_append(ocr_ctx_t *ctx, char c, frame_geometry_t *g)
{
  /* Grow text buffers as needed */
  while ( (ctx->text.len+1) >= ctx->text.size ) {
    ctx->text.size += 16;
    ctx->text.buf = (char *) realloc(ctx->text.buf, sizeof(char) * ctx->text.size);
    ctx->text.map = (frame_geometry_t *) realloc(ctx->text.map, sizeof(frame_geometry_t) * ctx->text.size);
  }

  ctx->text.buf[ctx->text.len] = c;
  ctx->text.map[ctx->text.len] = *g;
  ctx->text.len++;
}


int ocr_get_text_geometry(ocr_ctx_t *ctx, int offset, int len, frame_geometry_t *g)
{
  ocr_text_t *text = &(ctx->text);
  frame_geometry_t g0, g1;
  int y0, y1;
  int i;

  /* A valid text buffer should exist */
  if ( text->len == 0 )
    return -1;

  /* Requested substring should begin within the text buffer.
     Return the whole line box geometry if not. */
  if ( (offset < 0) || (offset >= text->len) ) {
    *g = ctx->box;
    return 0;
  }

  /* Clip requested substring to fit into the text buffer */
  i = text->len - offset;
  if ( len > i )
    len = i;

  g0 = text->map[offset];
  g1 = text->map[offset+len-1];

  g->x = g0.x;
  g->width = g1.x + g1.width - g0.x;

  y0 = g0.y;
  y1 = -1;
  for (i = 0; i < len; i++) {
    int yy0 = text->map[offset+i].y;
    int yy1 = yy0 + text->map[offset+i].height;

    if ( y0 > yy0 )
      y0 = yy0;
    if ( y1 < yy1 )
      y1 = yy1;
  }

  g->y = y0;
  g->height = y1 - y0;

  return 0;
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


static void _startElement(void *ctx, const xmlChar *name, const xmlChar **attrs)
{
  dprintf("START ELEMENT name='%s'\n", name);

  if ( strcmp((char *) name, "document") == 0 ) {
    CTX(ctx)->state = OCR_STATE_READ;
    CTX(ctx)->addr = strtoul(get_attr((xmlChar **) attrs, "id"), NULL, 16);
    CTX(ctx)->box = frame_geometry_null;
    CTX(ctx)->text.len = 0;
  }
  else if ( CTX(ctx)->state == OCR_STATE_READ ) {
    char *geometry = get_attr((xmlChar **) attrs, "geometry");
    frame_geometry_t g = frame_geometry_null;

    /* Get element geometry attribute (if any) */
    if ( (geometry != NULL) && (*geometry != '\0') ) {
      frame_geometry_parse(geometry, &g);
      dprintf("     geometry=\"%s\"=%ux%u+%d+%d\n", geometry, g.width, g.height, g.x, g.y);
    }

    if ( strcmp((char *) name, "line") == 0 ) {
      CTX(ctx)->box = g;
      CTX(ctx)->text.len = 0;
    }
    else {
      /* Ignore delirious line boundaries */
      if ( (CTX(ctx)->box.width > 0) && (CTX(ctx)->box.height > 0) ) {
	if ( strcmp((char *) name, "box") == 0 ) {
	  char *value = get_attr((xmlChar **) attrs, "value");
	  if ( strlen(value) == 1 ) {
	    /* Ignore leading unknown characters */
	    if ( (CTX(ctx)->text.len > 0) || (*value != '_' ) )
	      ocr_text_append(CTX(ctx), *value, &g);
	  }
	}
	else if ( strcmp((char *) name, "space") == 0 ) {
	  /* Ignore leading space characters */
	  if ( CTX(ctx)->text.len > 0 ) {
	    ocr_text_append(CTX(ctx), ' ', &g);
	  }
	}
      }
    }
  }
}


static void _endElement(void *ctx, const xmlChar *name)
{
  dprintf("END ELEMENT name='%s'\n", name);

  if ( strcmp((char *) name, "document") == 0 ) {
    /* Report end of text */
    if ( CTX(ctx)->reply_func != NULL )
      CTX(ctx)->reply_func(CTX(ctx)->addr, NULL, ctx, CTX(ctx)->reply_data);

    CTX(ctx)->state = OCR_STATE_END;
    CTX(ctx)->addr = 0;
  }
  else if ( strcmp((char *) name, "line") == 0 ) {
    if ( (CTX(ctx)->state == OCR_STATE_READ) && (CTX(ctx)->text.len > 0) ) {
      CTX(ctx)->text.buf[CTX(ctx)->text.len] = '\0';

      /* Report buffer for text pattern matching */
      if ( CTX(ctx)->reply_func != NULL )
	CTX(ctx)->reply_func(CTX(ctx)->addr, CTX(ctx)->text.buf, ctx, CTX(ctx)->reply_data);
    }
  }
}


static xmlSAXHandler handlers = {
  startElement:  _startElement,
  endElement:    _endElement,
};


static void ocr_terminate(ocr_t *ocr)
{
  if ( ocr->agent != NULL ) {
    agent_destroy(ocr->agent);
    ocr->agent = NULL;
  }
}


static void ocr_callback(ocr_t *ocr, char *buf, int size)
{
  if ( buf != NULL ) {
    dprintf("READ '%s'\n", buf);

    /* Create XML parser context if a new page is coming */
    if ( ocr->xml == NULL )
      ocr->xml = xmlCreatePushParserCtxt(&handlers, &(ocr->ctx), buf, size, "ocr-output");
    else
      xmlParseChunk(ocr->xml, buf, size, 0);

    if ( (ocr->xml != NULL) && (ocr->ctx.state == OCR_STATE_END) ) {
      /* End document parsing */
      xmlParseChunk(ocr->xml, buf, 0, 1);

      /* Free XML SAX parser */
      xmlFreeParserCtxt(ocr->xml);
      ocr->xml = NULL;

      ocr->ctx.state = OCR_STATE_IDLE;
    }
  }
  else {
    ocr_terminate(ocr);
  }
}


static int ocr_agent_spawn(ocr_t *ocr, int shmid)
{
  char *argv[ocr->agent_argc+3];
  int argc;
  char argv_P[20];
  int i;

  /* Build OCR agent arguments */
  snprintf(argv_P, sizeof(argv_P), "%d", shmid);

  argc = 0;
  argv[argc++] = ocr->agent_launcher;
  argv[argc++] = argv_P;
  for (i = 0; i < ocr->agent_argc; i++)
    argv[argc++] = ocr->agent_argv[i];
  argv[argc] = NULL;

  /* Spawn OCR agent subprocess */
  ocr->agent = agent_create(argv, (agent_callback_t *) ocr_callback, ocr);

  /* Sleep a while to let OCR agent spawn */
  usleep(10*1000);

  return 0;
}


static void ocr_clear_agent(ocr_t *ocr)
{
  int i;

  if ( ocr->agent_name != NULL )
    free(ocr->agent_name);
  ocr->agent_name = NULL;

  if ( ocr->agent_launcher != NULL )
    free(ocr->agent_launcher);
  ocr->agent_launcher = NULL;

  if ( ocr->agent_argv != NULL ) {
    for (i = 0; i < ocr->agent_argc; i++)
      free(ocr->agent_argv[i]);
    free(ocr->agent_argv);
  }
  ocr->agent_argv = NULL;
  ocr->agent_argc = 0;
}


int ocr_set_agent(ocr_t *ocr, char *ocr_agent, int ocr_argc, char *ocr_argv[])
{
  char launcher[256];
  int i;

  /* Check OCR agent launcher exists */
  if ( ocr_agent != NULL ) {
    snprintf(launcher, sizeof(launcher), OCR_LIB_DIR G_DIR_SEPARATOR_S "%s" OCR_LAUNCHER_SUFFIX, ocr_agent);
    if ( access(launcher, X_OK) ) {
      fprintf(stderr, "Cannot find OCR agent launcher '%s': %s\n", launcher, strerror(errno));
      return -1;
    }
  }

  ocr_clear_agent(ocr);

  ocr->agent_name = strdup(ocr_agent);
  ocr->agent_launcher = strdup(launcher);

  if ( ocr_argv != NULL ) {
    ocr->agent_argc = ocr_argc;
    ocr->agent_argv = calloc(ocr_argc+1, sizeof(char *));

    for (i = 0; i < ocr_argc; i++)
      ocr->agent_argv[i] = strdup(ocr_argv[i]);
  }

  return 0;
}


char *ocr_get_agent(ocr_t *ocr)
{
  char *buf, *p;
  int len;
  int i;

  len = strlen(ocr->agent_name);
  for (i = 0; i < ocr->agent_argc; i++)
    len += (strlen(ocr->agent_argv[i]) + 1);

  p = buf = malloc(len+1);

  p += sprintf(p, "%s", ocr->agent_name);
  for (i = 0; i < ocr->agent_argc; i++)
    p += sprintf(p, " %s", ocr->agent_argv[i]);

  return buf;
}


ocr_t *ocr_alloc(int shmid)
{
  ocr_t *ocr;

  /* Alloc new OCR agent descriptor */
  ocr = malloc(sizeof(ocr_t));
  memset(ocr, 0, sizeof(ocr_t));

  /* Set default OCR agent */
  ocr_set_agent(ocr, OCR_DEFAULT, 0, NULL);
  ocr->shmid = shmid;

  return ocr;
}


void ocr_destroy(ocr_t *ocr)
{
  /* Clear data reporting callback */
  ocr->ctx.reply_func = NULL;
  ocr->ctx.reply_data = NULL;

  /* Kill OCR agent */
  ocr_terminate(ocr);

  /* Free text buffers */
  if ( ocr->ctx.text.buf != NULL ) {
    free(ocr->ctx.text.buf);
    ocr->ctx.text.buf = NULL;
  }
  if ( ocr->ctx.text.map != NULL ) {
    free(ocr->ctx.text.map);
    ocr->ctx.text.map = NULL;
  }
  ocr->ctx.text.size = 0;

  /* Free agent specification */
  ocr_clear_agent(ocr);
}


int ocr_enable(ocr_t *ocr, int state)
{
  int ret = 0;

  if ( (ocr->shmid > 0) && state ) {
    if ( ocr->agent == NULL )
      ret = ocr_agent_spawn(ocr, ocr->shmid);
  }
  else {
    ocr_terminate(ocr);
  }

  return ret;
}


int ocr_request(ocr_t *ocr,
		unsigned long addr, frame_geometry_t *g, int inverse,
		ocr_reply_t *reply, void *data)
{
  char buf[80];
  int size;

  /* Set data reporting callback */
  ocr->ctx.reply_func = reply;
  ocr->ctx.reply_data = data;

  /* Construct request message */
  size = snprintf(buf, sizeof(buf), "id=%lX geometry=%s%s\n",
		  addr, frame_geometry_str(g),
		  inverse ? " inv":"");
  dprintf("WRITE '%s'", buf);

  return agent_request(ocr->agent, buf, size);
}
