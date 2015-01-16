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

#ifndef __INTERFACE_LINK_H__
#define __INTERFACE_LINK_H__

#define USE_SELECT

#include <sys/types.h>

#include "tstamp.h"
#include "link_bin.h"

#ifdef ENABLE_GLIB
#include "link_txt.h"
#endif

typedef struct link_item_s link_item_t;


/*******************************************************/
/* Link classes                                        */
/*******************************************************/

#ifdef USE_SELECT
typedef struct {
  fd_set *rd;
  int max;
} link_fds_t;

typedef void link_fd_set_func_t(link_item_t *link, link_fds_t *lfds);
typedef int link_fd_isset_func_t(link_item_t *link, fd_set *fds);
#endif

typedef char *link_addr_func_t(link_item_t *link);
typedef int link_connect_func_t(link_item_t *link, char *argv[], char **errmsg);
typedef void link_kill_func_t(link_item_t *link);

typedef struct {
  char *method;
  link_addr_func_t *addr;
#ifdef USE_SELECT
  link_fd_set_func_t *fd_set;
  link_fd_isset_func_t *fd_isset;
#endif
  link_connect_func_t *connect;
  link_kill_func_t *kill;
} link_class_t;


int link_class_register(link_class_t *lclass);


/*******************************************************/
/* Link items                                          */
/*******************************************************/

typedef void link_func_t(link_item_t *item, void *arg);

typedef union {
  link_src_bin_t bin;
#ifdef ENABLE_GLIB
  link_src_txt_t txt;
#endif
} link_src_t;

struct link_item_s {
  char *name;
  link_class_t *class;
  int fd_in;
  int fd_out;
#ifdef ENABLE_GLIB
  GIOChannel *io_channel;
  guint io_tag;
#endif
  void *ptr;
  tstamp_t tstamp;
  link_func_t *unref_fn;
  void *unref_arg;
  link_src_t src;
};


extern void link_info(link_item_t *link, char *hdr);
extern void link_report(link_item_t *link, char *hdr);
extern int link_parse(link_item_t *link, int argc, char **argv);

extern link_item_t *link_new(char *name);
extern void link_set_unref(link_item_t *link, link_func_t *fn, void *arg);
extern void link_free(link_item_t *item);
extern link_item_t *link_retrieve(char *name);

extern int link_connect(link_item_t *link, char *argv[], char **errmsg);
extern void link_kill(link_item_t *link);

extern void link_scan(link_func_t *handler, void *arg);

extern int link_init(void);
extern void link_done(char *hdr);

#ifdef ENABLE_GLIB
extern void link_use_glib(void);
#endif

#ifdef USE_SELECT
extern int link_fd_set(fd_set *rd);
extern int link_fd_isset(link_item_t *link, fd_set *fds);
#endif

extern int link_read(link_item_t *link);
extern void link_disconnect(link_item_t *link);


/*******************************************************/
/* Link source description                             */
/*******************************************************/

typedef struct {
  char *help_str0;
  char *help_str1;
  int (*parse)(link_src_t *src, int argc, char **argv);
  void (*clear)(link_src_t *src);
  void (*show)(link_src_t *src);
  void (*unref)(link_src_t *src);
  int (*send_data)(link_src_t *src, char **argv, void **pbuf);
} link_src_desc_t;

extern link_src_desc_t link_src_desc;

#endif /* __INTERFACE_LINK_H__ */
