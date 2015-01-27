/****************************************************************************/
/* TestFarm                                                                 */
/* Test Engine - Peripheral link list management                            */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 17-AUG-1999                                                    */
/****************************************************************************/

/*
 * $Revision: 42 $
 * $Date: 2006-06-03 15:30:01 +0200 (sam., 03 juin 2006) $
 */

#ifndef __PERIPH_H__
#define __PERIPH_H__

#include "periph_item.h"


#define PERIPH_HASH_MIN   '0'
#define PERIPH_HASH_MAX   'Z'
#define PERIPH_HASH_OTHER (PERIPH_HASH_MAX + 1)
#define PERIPH_HASH_SIZE  (PERIPH_HASH_OTHER - PERIPH_HASH_MIN + 1)

typedef struct {
  periph_item_t *link_hash[PERIPH_HASH_SIZE];
  periph_item_t **fd_tab;
  int fd_count;
} periph_t;


extern periph_t *periph_init(void);
extern void periph_done(periph_t *periph);
extern void periph_new(periph_t *periph, periph_item_t *item);
extern periph_item_t *periph_retrieve(periph_t *periph, char *id);
extern int periph_close(periph_t *periph, periph_item_t *item);
extern int periph_open(periph_t *periph, periph_item_t *item);
extern int periph_connected(periph_t *periph);
extern int periph_fd_set(periph_t *periph, fd_set *fds);
extern periph_item_t *periph_fd_retrieve(periph_t *periph, int fd);

typedef void periph_info_method_t(char *);
extern void periph_info(periph_t *periph, char *hdr, periph_info_method_t *method);

#endif /* __PERIPH_H__ */
