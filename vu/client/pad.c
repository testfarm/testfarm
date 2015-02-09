/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* Color padding                                                            */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 03-AUG-2007                                                    */
/****************************************************************************/

/*
 * $Revision: 923 $
 * $Date: 2008-02-04 09:54:32 +0100 (lun., 04 févr. 2008) $
 */

#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <glib.h>

#include "frame_display.h"
#include "pad_list.h"
#include "fuzz.h"
#include "pad.h"


pad_t *pad_alloc(frame_display_t *display)
{
  pad_t *pad = (pad_t *) malloc(sizeof(pad_t));
  pad->display = display;
  pad->pads = NULL;
  return pad;
}


void pad_destroy(pad_t *pad)
{
  pad->display = NULL;
  pad_remove(pad, NULL);
  free(pad);
}


pad_list_t *pad_add(pad_t *pad, char *id,
		    frame_t *frame, frame_geometry_t *window,
		    unsigned long color_value, fuzz_t *fuzz,
		    unsigned int gap,
		    unsigned int min_width, unsigned int min_height)
{
  pad_list_t *list;

  list = pad_retrieve(pad, id);
  if ( list != NULL ) {
    pad->pads = g_list_remove(pad->pads, list);
    pad_list_destroy(list);
  }

  list = pad_list_add(id,
		      frame, window,
		      color_value, fuzz,
		      gap,
		      min_width, min_height);

  if ( list != NULL ) {
    pad->pads = g_list_append(pad->pads, list);
  }

  return list;
}


pad_list_t *pad_retrieve(pad_t *pad, char *id)
{
  GList *l = pad->pads;

  while ( l != NULL ) {
    pad_list_t *list = l->data;

    if ( strcmp(list->id, id) == 0 )
      return list;

    l = l->next;
  }

  return NULL;
}


int pad_remove(pad_t *pad, char *id)
{
  if ( id == NULL ) {
    g_list_foreach(pad->pads, (GFunc) pad_list_destroy, NULL);
    g_list_free(pad->pads);
    pad->pads = NULL;
  }
  else {
    pad_list_t *list = pad_retrieve(pad, id);
    if ( list == NULL )
      return -1;

    pad->pads = g_list_remove(pad->pads, list);
    pad_list_destroy(list);
  }

  return 0;
}


typedef struct {
  char *hdr;
  frame_display_t *display;
} pad_show_t;

static void pad_show_item(pad_list_t *list, pad_show_t *data)
{
  frame_geometry_t *gtab;
  int nmemb;

  gtab = pad_list_tab(list, &nmemb);
  frame_display_pad_show(data->display, gtab, nmemb);
  free(gtab);

  pad_list_show(list, data->hdr);
}

void pad_show(pad_t *pad, char *id, char *hdr)
{
  pad_show_t data = {
    hdr: hdr,
    display: pad->display
  };

  if ( id == NULL ) {
    g_list_foreach(pad->pads, (GFunc) pad_show_item, &data);
  }
  else {
    pad_list_t *list = pad_retrieve(pad, id);

    if ( list != NULL )
      pad_show_item(list, &data);
  }
}
