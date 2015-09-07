/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* Input event recorder                                                     */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 01-APR-2007                                                    */
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
#include <sys/time.h>

#include "keysyms.h"
#include "scroll.h"
#include "record.h"


static FILE *record_f = NULL;
static int record_async = 0;
static unsigned long long record_t0 = 0;
static unsigned long long record_t1 = 0;

static int record_x_shown = 0;
static int record_y_shown = 0;
static unsigned char record_buttons_shown = 0;


void record_date(void)
{
  struct timeval tv;

  if ( record_async )
    return;

  gettimeofday(&tv, NULL);
  record_t1 = (((unsigned long long) tv.tv_sec) * 1000) + (tv.tv_usec / 1000);
}


void record_delay(void)
{
  if ( record_f == NULL )
    return;

  if ( record_async )
    return;

  if ( record_t0 != record_t1 ) {
    if ( record_t0 != 0 )
      fprintf(record_f, "sleep %llu\n", record_t1 - record_t0);
    record_t0 = record_t1;
  }
}


void record_key(unsigned long keyval, unsigned char down)
{
  char *action;
  keysyms_t *keysym;

  if ( record_f == NULL )
    return;

  record_date();

  record_delay();

  action = down ? "press":"release";
  keysym = keysyms_retrieve_by_code(keyval);

  if ( keysym != NULL ) {
    if ( (keysym->class != NULL) && (keysym->class->enable == 0) )
      fprintf(record_f, "kp enable %s\n", keysym->class->name);
    fprintf(record_f, "kp %s %s\n", action, keysym->sym);
  }
  else {
    fprintf(record_f, "kp %s 0x04%lX\n", action, keyval);
  }
}


int record_position(int x, int y)
{
  if ( record_f == NULL )
    return 0;

  if ( (x == record_x_shown) && (y == record_y_shown) )
    return 0;

  record_date();

  fprintf(record_f, "mouse move x=%d y=%d\n", x, y);
  record_x_shown = x;
  record_y_shown = y;

  return 1;
}


void record_buttons(unsigned char buttons)
{
  int i;

  if ( record_f == NULL )
    return;

  record_date();

  record_delay();

  for (i = 0; i < 8; i++) {
    unsigned char mask = buttons & (1 << i);
    if ( (record_buttons_shown & (1 << i)) != mask )
      fprintf(record_f, "mouse %s %d\n", mask ? "press":"release", i+1);
  }

  record_buttons_shown = buttons;
}


void record_scroll(unsigned char direction)
{
  char *sdir = scroll_str(direction);

  if ( sdir == NULL )
    return;
  if ( record_f == NULL )
    return;

  record_date();

  record_delay();

  fprintf(record_f, "mouse scroll %s\n", sdir);
}


int record_set_async(int async)
{
  if ( async >= 0 )
    record_async = async;
  return record_async;
}


void record_init(FILE *f)
{
  record_f = f;
  record_async = 0;
  record_t0 = 0;
  record_t1 = 0;
}
