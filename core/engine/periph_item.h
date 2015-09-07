/****************************************************************************/
/* TestFarm                                                                 */
/* Test Engine - Peripheral link management                                 */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 17-AUG-1999                                                    */
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

#ifndef __TESTFARM_PERIPH_ITEM_H__
#define __TESTFARM_PERIPH_ITEM_H__

#include <stdlib.h>
#include <unistd.h>
#include <termios.h>

#undef USE_PIPE

typedef enum {
  PERIPH_NULL=0,
  PERIPH_PROC,
  PERIPH_TCP,
  PERIPH_SERIAL,
#ifdef USE_PIPE
  PERIPH_PIPE,
#endif
  PERIPH_STDIO,
  PERIPH_N
} periph_item_type_t;


#define PERIPH_FLAG_NOHEADER 1

typedef struct periph_item_s periph_item_t;

typedef void periph_item_closed_t(periph_item_t *item, void *arg);

struct periph_item_s {
  char *id;
  periph_item_type_t type;
  int flags;
  char *blabla;
  union {
    struct {
      char *command;
      char *argv_buf;
      char **argv;
      pid_t pid;
    } proc;
    struct {
      char *host;
      int port;
    } tcp;
    struct {
      char *device;
      struct termios termio_save;
    } serial;
    struct {
      char *input;
      char *output;
    } pipe;
  } dev;
  int fd_in;
  int fd_out;
  char *buf;
  int size;
  int index;

  periph_item_closed_t *closed;
  void *closed_arg;

  periph_item_t *next;
};


extern periph_item_t *periph_item_init(char *argv[] /* {"id", "type", "dev", "flags", "comment", ...} */, periph_item_t *item);
extern int periph_item_open(periph_item_t *item);
extern void periph_item_hangup(periph_item_t *item);
extern void periph_item_close(periph_item_t *item);
extern void periph_item_done(periph_item_t *item);
extern int periph_item_connected(periph_item_t *item);
extern int periph_item_read(periph_item_t *item);
extern int periph_item_write(periph_item_t *item, char *buf, int size);
extern int periph_item_cleanup(periph_item_t *item);
extern char *periph_item_info(periph_item_t *item);

extern void periph_item_closed_set(periph_item_t *item, periph_item_closed_t *closed, void *closed_arg);
extern void periph_item_closed_raise(periph_item_t *item);

#endif /* __TESTFARM_PERIPH_ITEM_H__ */
