/****************************************************************************/
/* TestFarm                                                                 */
/* Interface logical link management - FIFO/PORT data source                */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 06-APR-2007                                                    */
/****************************************************************************/

/*
 * $Revision: 733 $
 * $Date: 2007-09-22 21:45:49 +0200 (sam., 22 sept. 2007) $
 */

#ifndef __INTERFACE_LINK_BIN_H__
#define __INTERFACE_LINK_BIN_H__

#define LINK_MODE_BITS 0x00FF
#define LINK_MODE_PORT 0x0100
#define LINK_MODE_FIFO 0x0200
#define LINK_MODE_INV  0x0800
#define LINK_MODE_HEX  0x1000

typedef struct {
  unsigned int mode;
  unsigned long mask;
  unsigned long equal;
  unsigned long state;
} link_src_bin_t;

extern int link_bin_init(void);

#endif /* __INTERFACE_LINK_BIN_H__ */
