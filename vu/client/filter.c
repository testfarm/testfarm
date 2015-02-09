/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* Filter item                                                              */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 29-AUG-2007                                                    */
/****************************************************************************/

/*
 * $Revision: 904 $
 * $Date: 2008-01-15 14:10:42 +0100 (mar., 15 janv. 2008) $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>

#include "frame_rgb.h"

#include "error.h"
#include "filter_thresh.h"
#include "filter_ext.h"
#include "filter.h"


static GList *filter_classes = NULL;


void filter_init(void)
{
  /* Register built-in RGB thresholding filter */
  filter_thresh_init();

  /* Register external filter agents */
  filter_ext_init();
}


void filter_done(void)
{
  /* Shut down external filter agents */
  filter_ext_done();

  /* Shut down RGB thresholding filter */
  filter_thresh_done();
}


void filter_register(filter_class_t *class)
{
  filter_classes = g_list_append(filter_classes, class);
}


filter_t *filter_alloc(char *class_id, GList *options)
{
  filter_class_t *class = NULL;
  filter_t *filter = NULL;
  GList *l;

  l = filter_classes;
  while ( (l != NULL) && (class == NULL) ) {
    class = l->data;
    if ( strcmp(class->id, class_id) ) {
      class = NULL;
    }

    l = g_list_next(l);
  }

  if ( class != NULL ) {
    filter = class->alloc(class_id, options);
    filter->class = class;
    filter->index = -1;
  }
  else {
    error(NULL, "Unknown filter class '%s'", class_id);
  }

  return filter;
}


void filter_destroy(filter_t *filter)
{
  filter_class_t *class = filter->class;

  if ( class->destroy != NULL )
    class->destroy(filter);
  else
    free(filter);
}


void filter_show(filter_t *filter, char *hdr)
{
  filter_class_t *class = filter->class;
  char header[strlen(hdr) + strlen(class->id) + 32];
  char trailer[32];
  int len;

  snprintf(header, sizeof(header), "%s%d %s ", hdr, filter->index, class->id);

  len = 0;
  if ( filter->dt > 0 )
    len += snprintf(trailer, sizeof(trailer)-2, " dt=%llu.%03llums", filter->dt/1000, filter->dt%1000);
  trailer[len++] = '\n';
  trailer[len] = '\0';

  if ( class->show != NULL ) {
    class->show(filter, header, trailer);
  }
  else {
    fputs(header, stdout);
    fputs(trailer, stdout);
  }
}


void filter_show_classes(void)
{
  GList *l = filter_classes;

  while ( l ) {
    filter_class_t *class = l->data;
    printf(" %s", class->id);
    l = g_list_next(l);
  }
}


void filter_set_callback(filter_t *filter, filter_callback_func callback, void *ctx)
{
  filter->callback = callback;
  filter->ctx = ctx;
}


void filter_applied(filter_t *filter)
{
  /* Compute filter processing time */
  filter->dt = tstamp_get() - filter->t0;

  /* Acknowledge filter completion */
  if ( filter->callback != NULL ) {
    filter->callback(filter->ctx, filter);
  }
}


void filter_apply(filter_t *filter, frame_hdr_t *frame)
{
  filter_class_t *class = filter->class;

  /* Set filter processing start date */
  filter->t0 = tstamp_get();

  class->apply(filter, frame);
}
