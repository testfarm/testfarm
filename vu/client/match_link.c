/****************************************************************************/
/* TestFarm                                                                 */
/* Virtual User subprocess link management                                  */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 11-APR-2007                                                    */
/****************************************************************************/

/*
 * $Revision: 1232 $
 * $Date: 2012-04-08 16:17:42 +0200 (dim., 08 avril 2012) $
 */


#include <stdio.h>
#include <string.h>
#include <unistd.h>

#ifndef ENABLE_GLIB
#define ENABLE_GLIB
#endif

#include "tstamp.h"
#include "log.h"
#include "link.h"
#include "link_txt.h"

#include "match_link.h"


typedef struct {
  char *id;
  tstamp_t tstamp;
  char *str;
  int len;
  int count;
} match_link_dump_t;


static void match_link_dump_item(link_item_t *link, match_link_dump_t *desc)
{
  if ( ! link_txt_attached(&(link->src.txt), desc->id) )
    return;
  (desc->count)++;

  /* Update link time stamp */
  link->tstamp = desc->tstamp;

  /* Send message to attached link subprocess */
  if ( link->fd_out != -1 ) {
    write(link->fd_out, desc->str, desc->len);
  }

  /* No attached link : display message directly */
  else {
    log_dump(log_hdr(link->tstamp, link->name), desc->str);
  }
}


int match_link_dump(char *id, tstamp_t tstamp, char *str)
{
  match_link_dump_t desc;

  /* Send message to all attached links */
  desc.id = id;
  desc.tstamp = tstamp;
  desc.str = str;
  desc.len = strlen(str);
  desc.count = 0;
  link_scan((link_func_t *) match_link_dump_item, &desc);

  return desc.count;
}
