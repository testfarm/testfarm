/****************************************************************************/
/* TestFarm                                                                 */
/* Virtual User subprocess link management                                  */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 11-APR-2007                                                    */
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
