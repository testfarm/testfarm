/****************************************************************************/
/* TestFarm                                                                 */
/* Data Logger interface : logical link management                          */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 05-APR-2000                                                    */
/****************************************************************************/

/*
 * $Revision: 1220 $
 * $Date: 2011-12-04 17:01:50 +0100 (dim., 04 déc. 2011) $
 */

#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <errno.h>
#include <unistd.h>

#include "log.h"
#include "sub.h"
#include "tcpip.h"
#include "link.h"


#ifdef ENABLE_GLIB
#include <glib.h>
static int use_glib = 0;
#endif

link_src_desc_t link_src_desc = {
  help_str0: "",
  help_str1: "",
  parse: NULL,
  clear: NULL,
  show: NULL,
  unref: NULL,
  send_data: NULL,
};


/*******************************************************/
/* Link classes                                        */
/*******************************************************/

#define LINK_CLASS_N 4

static link_class_t *link_class[LINK_CLASS_N];


int link_class_register(link_class_t *lclass)
{
  int i;

  for (i = 0; i < LINK_CLASS_N; i++) {
    if ( link_class[i] == NULL ) {
      link_class[i] = lclass;
      return i;
    }
  }

  return -1;
}


/*******************************************************/
/* Link items                                          */
/*******************************************************/

#define LINK_NB 16
static link_item_t link_tab[LINK_NB];


static void link_item_clear(link_item_t *link)
{
  link->name = NULL;
  link->class = NULL;
  link->ptr = NULL;
  link->fd_in = -1;
  link->fd_out = -1;
#ifdef ENABLE_GLIB
  link->io_channel = NULL;
  link->io_tag = 0;
#endif /* ENABLE_GLIB */
  link->tstamp = 0;
  link->unref_fn = NULL;
  link->unref_arg = NULL;
  if ( link_src_desc.clear != NULL )
    link_src_desc.clear(&(link->src));
}


#ifdef USE_SELECT
static void link_item_fd_set(link_item_t *link, link_fds_t *lfds)
{
  if ( link->class == NULL )
    return;
  if ( link->class->fd_set == NULL )
    return;
  link->class->fd_set(link, lfds);
}
#endif


link_item_t *link_retrieve(char *name)
{
  link_item_t *link;
  int i;

  link = NULL;
  for (i = 0; (i < LINK_NB) && (link == NULL); i++) {
    link = &(link_tab[i]);
    if ( link->name == NULL )
      link = NULL;
    else if ( strcmp(name, link->name) != 0 )
      link = NULL;
  }

  return link;
}


void link_scan(link_func_t *handler, void *arg)
{
  link_item_t *link = link_tab;
  int i;

  for (i = 0; i < LINK_NB; i++) {
    if ( link->name != NULL )
      handler(link, arg);
    link++;
  }
}


/*******************************************************/
/* Link setup                                          */
/*******************************************************/

link_item_t *link_new(char *name)
{
  link_item_t *link;
  int i;

  /* Find new entry */
  link = NULL;
  for (i = 0; (i < LINK_NB) && (link == NULL); i++) {
    link = &(link_tab[i]);
    if ( link->name != NULL )
      link = NULL;
  }

  if ( link == NULL )
    return NULL;

  /* Setup new entry */
  link_item_clear(link);
  link->name = strdup(name);

  return link;
}


void link_set_unref(link_item_t *link, link_func_t *fn, void *arg)
{
  link->unref_fn = fn;
  link->unref_arg = arg;
}


void link_free(link_item_t *link)
{
  /* Kill sub-processing */
  link_kill(link);

  /* Call link unref handler */
  if ( link->unref_fn != NULL )
    link->unref_fn(link, link->unref_arg);

  /* Call source unref handler */
  if ( link_src_desc.unref != NULL )
    link_src_desc.unref(&(link->src));

  /* Free link identifier */
  if ( link->name != NULL ) {
    free(link->name);
    link->name = NULL;
  }
}


#ifdef ENABLE_GLIB

static gboolean link_read_event(GIOChannel *source, GIOCondition condition, link_item_t *link)
{
  int size = 0;

  /* Read data from link connection */
  if ( condition & G_IO_IN )
    size = link_read(link);

  /* Check for remote disconnection */
  if ( size == 0 ) {
    link_disconnect(link);
    return FALSE;
  }

  return TRUE;
}

#endif /* ENABLE_GLIB */


int link_connect(link_item_t *link, char *argv[], char **errmsg)
{
  link_class_t *class = NULL;
  char *argv0 = argv[0];
  char *addr;
  int ret;

  /* Check the link is not already connected */
  if ( link->class != NULL ) {
    *errmsg = "Subprocess connection already active on this link";
    return -1;
  }

  /* Retrieve connection method */
  addr = strstr(argv0, "://");
  if ( addr != NULL ) {
    char c;
    int i;

    c = *addr;
    *addr = '\0';

    for (i = 0; (i < LINK_CLASS_N) && (link_class[i] != NULL); i++) {
      if ( strcmp(argv0, link_class[i]->method) == 0 ) {
	class = link_class[i];
	break;
      }
    }

    *addr = c;
    addr += 3;
  }
  else {
    class = link_class[0];
    addr = argv0;
  }

  if ( class == NULL ) {
    *errmsg = "Unknown subprocess connection method";
    return -1;
  }

  //fprintf(stderr, "-- class=%s addr='%s'\n", class->method, addr);

  /* Update argument list */
  argv[0] = addr;

  /* Perform connection */
  ret = class->connect(link, argv, errmsg);
  if ( ret == 0 )
    link->class = class;

#ifdef ENABLE_GLIB
  /* Setup read event handling */
  if ( use_glib && (link->fd_in >= 0) ) {
    link->io_channel = g_io_channel_unix_new(link->fd_in);
    link->io_tag = g_io_add_watch(link->io_channel, G_IO_IN | G_IO_HUP,
				  (GIOFunc) link_read_event, link);
  }
#endif /* ENABLE_GLIB */

  /* Restore argument list */
  argv[0] = argv0;

  return ret;
}


void link_kill(link_item_t *link)
{
  link_class_t *class = link->class;

#ifdef ENABLE_GLIB
  /* Stop read event handling */
  if ( link->io_tag > 0 ) {
    g_source_remove(link->io_tag);
    link->io_tag = 0;
  }

  if ( link->io_channel != NULL ) {
    g_io_channel_unref(link->io_channel);
    link->io_channel = NULL;
  }
#endif /* ENABLE_GLIB */

  /* Kill link post-processing */
  if ( (class != NULL) && (class->kill != NULL) )
    class->kill(link);

  link->class = NULL;
  link->fd_out = -1;
  link->fd_in = -1;
  link->ptr = NULL;
}


static char *link_addr(link_item_t *link)
{
  char addr_str[256];
  link_class_t *class = link->class;

  if ( class != NULL ) {
    int len = snprintf(addr_str, sizeof(addr_str), "%s://", class->method);
    
    if ( class->addr != NULL ) {
      char *str = class->addr(link);
      len += snprintf(addr_str+len, sizeof(addr_str)-len, "%s", str);
      free(str);
    }
  }
  else {
    addr_str[0] = '\0';
  }

  return strdup(addr_str);
}


void link_info(link_item_t *link, char *hdr)
{
  char *str;

  if ( link == NULL )
    return;
  if ( link->class == NULL )
    return;

  str = link_addr(link);
  printf("%s%s %s\n", hdr, link->name, str);
  free(str);
}


void link_report(link_item_t *link, char *hdr)
{
  char *str;

  /* Link name */
  printf("%s%s ", hdr, link->name);

  /* Link source */
  if ( link_src_desc.show != NULL )
    link_src_desc.show(&(link->src));

  /* Subprocess */
  str = link_addr(link);
  printf(" %s\n", str);
  free(str);
}


int link_parse(link_item_t *link, int argc, char **argv)
{
  int ret = 0;

  if ( link_src_desc.parse != NULL )
    ret = link_src_desc.parse(&(link->src), argc, argv);

  return ret;
}


/*******************************************************/
/* Link i/o operations                                 */
/*******************************************************/

#ifdef USE_SELECT
int link_fd_set(fd_set *rd)
{
  link_fds_t lfds;

  lfds.rd = rd;
  lfds.max = 0;

  link_scan((link_func_t *) link_item_fd_set, &lfds);

  return lfds.max;
}


int link_fd_isset(link_item_t *link, fd_set *fds)
{
  link_class_t *class = link->class;

  if ( class == NULL )
    return -1;
  if ( class->fd_isset == NULL )
    return -1;

  return class->fd_isset(link, fds);
}
#endif


void link_disconnect(link_item_t *link)
{
  link_info(link, log_hdr_(TAG_DONE));
  link_kill(link);
}


int link_read(link_item_t *link)
{
  char *hdr;
  char buf[BUFSIZ];
  int size;
  int total;

  /* Prepare header */
  hdr = log_hdr(link->tstamp, link->name);
  total = 0;

  /* Get data from TCP/IP pipe */
  while ( (size = read(link->fd_in, buf, BUFSIZ-1)) > 0 ) {
    buf[size] = '\0';
    log_dump(hdr, buf);
    total += size;
  }

  if ( size < 0 ) { 
    if ( (errno == EAGAIN) || (errno == EINTR) ) {
      total = -1;
    }
    else {
      fprintf(stderr, "*PANIC* Error reading link subprocess: %s\n", strerror(errno));
      total = 0;
    }
  }

  return total;
}


/*******************************************************/
/* Link global initialization                          */
/*******************************************************/

int link_init(void)
{
  int i;

  /* Clear link subprocess classes table */
  for (i = 0; i < LINK_CLASS_N; i++)
    link_class[i] = NULL;

  /* Clear links table */
  for (i = 0; i < LINK_NB; i++)
    link_item_clear(&(link_tab[i]));

  /* Setup link subprocess classes */
  if ( sub_init() == -1 )
    return -1;
  if ( tcpip_init() == -1 )
    return -1;

  return 0;
}


void link_done(char *hdr)
{
  int i;

  /* Terminate subprocess management */
  sub_done();
  tcpip_done();

  /* Free link tables */
  for (i = 0; i < LINK_NB; i++) {
    link_item_t *link = &(link_tab[i]);
    link_info(link, hdr);
    link_free(link);
  }
}


#ifdef ENABLE_GLIB

void link_use_glib(void)
{
  use_glib = 1;
  sub_use_glib();
}

#endif /* ENABLE_GLIB */
