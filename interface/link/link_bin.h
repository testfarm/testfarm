/****************************************************************************/
/* TestFarm                                                                 */
/* Interface logical link management - FIFO/PORT data source                */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 06-APR-2007                                                    */
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
