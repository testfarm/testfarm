/****************************************************************************/
/* TestFarm                                                                 */
/* Test Engine - Peripheral link list management                            */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 17-AUG-1999                                                    */
/****************************************************************************/

/*
 * $Revision: 662 $
 * $Date: 2007-07-26 18:44:48 +0200 (jeu., 26 juil. 2007) $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>

#include "useful.h"
#include "debug.h"
#include "periph.h"


int periph_fd_set(periph_t *periph, fd_set *fds)
{
  int i;

    /* Cleanup peripheral entries */
  for (i = 0; i < periph->fd_count; i++) {
    periph_item_cleanup(periph->fd_tab[i]);
  }

  /* Clear fd set */
  FD_ZERO(fds);

  /* Setup fd set with open devices */
  for (i = 0; i < periph->fd_count; i++) {
    if ( periph->fd_tab[i] != NULL ) {
      if ( periph_item_cleanup(periph->fd_tab[i]) ) {
        periph->fd_tab[i] = NULL;
      }
      else {
        FD_SET(i, fds);
      }
    }
  }

  return periph->fd_count;
}


periph_item_t *periph_fd_retrieve(periph_t *periph, int fd)
{
  if ( fd >= periph->fd_count )
    return NULL;

  return periph->fd_tab[fd];
}


static unsigned int periph_hash_index(char c)
{
  if ( (c < PERIPH_HASH_MIN) || (c > PERIPH_HASH_MAX) )
    c = PERIPH_HASH_OTHER;
  return (unsigned int) (c - PERIPH_HASH_MIN);
}


void periph_new(periph_t *periph, periph_item_t *item)
{
  periph_item_t **entry;
  periph_item_t *last;

  if ( item == NULL )
    return;

  entry = &(periph->link_hash[periph_hash_index(item->id[0])]);
  last = *entry;

  if ( last != NULL ) {
    while ( last->next != NULL )
      last = last->next;
    last->next = item;
  }
  else {
    *entry = item;
  }
}


periph_item_t *periph_retrieve(periph_t *periph, char *id)
{
  periph_item_t *item = periph->link_hash[periph_hash_index(id[0])];

  while ( item != NULL ) {
    if ( strcmp(item->id, id) == 0 )
      return item;
    item = item->next;
  }

  return NULL;
}


int periph_close(periph_t *periph, periph_item_t *item)
{
  int i;
  int count;

  /* Shortcut if connections are actually open */
  if ( periph->fd_tab == NULL )
    return -1;

  if ( item == NULL ) {
    debug("Closing all peripherals...\n");

    /* Hangup all open peripherals */
    for (i = 0; i < periph->fd_count; i++) {
      if ( periph->fd_tab[i] != NULL ) {
        periph_item_hangup(periph->fd_tab[i]);
      }
    }

    /* Wait for smooth termination of processes */
    usleep(50000);

    /* Close all open peripherals */
    for (i = 0; i < periph->fd_count; i++)
      if ( periph->fd_tab[i] != NULL ) {
        periph_item_close(periph->fd_tab[i]);
        periph->fd_tab[i] = NULL;
      }
  }
  else {
    debug("Closing peripheral '%s'...\n", item->id);

    /* Close peripheral */
    count = 0;
    for (i = 0; i < periph->fd_count; i++)
      if ( periph->fd_tab[i] == item ) {
        periph_item_close(item);
        periph->fd_tab[i] = NULL;
        count++;
      }

    if ( count == 0 )
      return -1;
  }

  return 0;
}


int periph_open(periph_t *periph, periph_item_t *item)
{
  int fd;

  /* Open new connection */
  if ( (fd = periph_item_open(item)) == -1 ) {
    debug("Connection to peripheral '%s' failed\n", item->id);
    return -1;
  }

  /* Extend file descriptors table if necessary */
  if ( fd >= periph->fd_count ) {
    periph->fd_tab = realloc(periph->fd_tab, sizeof(periph_item_t *) * (fd+1));
    while ( periph->fd_count <= fd )
      periph->fd_tab[(periph->fd_count)++] = NULL;
  }

  /* Setup new entry in file descriptor table */
  periph->fd_tab[fd] = item;

  debug("Connection to peripheral '%s' established (fd=%d)\n", item->id, fd);

  return 0;
}


int periph_connected(periph_t *periph)
{
  return (periph->fd_count > 0);
}


void periph_info(periph_t *periph, char *hdr, void (*method)(char *))
{
  int i = 0;

  for (i = 0; i < PERIPH_HASH_SIZE; i++) {
    periph_item_t *item = periph->link_hash[i];
    while ( item != NULL ) {
      method(hdr);
      method(periph_item_info(item));
      method("\n");

      item = item->next;
    }
  }
}


periph_t *periph_init(void)
{
  periph_t *periph;
  int i;

  /* Allocate peripheral link list descriptor */
  periph = (periph_t *) malloc(sizeof(periph_t));
  for (i = 0; i < PERIPH_HASH_SIZE; i++)
    periph->link_hash[i] = NULL;
  periph->fd_tab = NULL;
  periph->fd_count = 0;

  return periph;
}


void periph_done(periph_t *periph)
{
  int i;

  if ( periph == NULL )
    return;

  debug("Releasing peripheral management...\n");

  /* Close connections */
  if ( periph->fd_tab != NULL )
    periph_close(periph, NULL);

  /* Close connections and free descriptors */
  for (i = 0; i < PERIPH_HASH_SIZE; i++) {
    periph_item_t *item = periph->link_hash[i];
    while ( item != NULL ) {
      periph_item_t *next = item->next;
      periph_item_done(item);
      item = next;
    }
  }

  if ( periph->fd_tab != NULL )
    free(periph->fd_tab);

  free(periph);
}
