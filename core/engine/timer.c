/****************************************************************************/
/* TestFarm                                                                 */
/* Test Engine - Timer messaging                                            */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 19-NOV-1999                                                    */
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
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <sys/time.h>

#include "useful.h"
#include "shell.h"
#include "timer.h"

#define TIMER_ONE_MS     1
#define TIMER_ONE_SECOND 1000
#define TIMER_ONE_MINUTE (TIMER_ONE_SECOND*60)
#define TIMER_ONE_HOUR   (TIMER_ONE_MINUTE*60)


/*-----------------------------------------------------------*/
/* Time argument evaluation                                  */
/*-----------------------------------------------------------*/

int timer_value(shell_t *shell, char *s_value, char *s_unit, long *pt)
{
  char *err;
  long t;

  /* Get timeout value */
  t = strtol(s_value, &err, 0);
  if ( t < 0 ) {
    shell_error(shell, "Illegal expression '%s' for timeout value\n", s_value);
    return -1;
  }

  err = strskip_spaces(err);
  if ( *err != NUL ) {
    if ( s_unit != NULL ) {
      shell_error(shell, "Illegal expression '%s' for timeout value\n", s_value);
      return -1;
    }
    s_unit = err;
  }

  /* Time unit qualifier ? */
  if ( s_unit != NULL ) {
    /* Adjust with given time unit */
    if ( strcmp(s_unit, "h") == 0 )
      t *= TIMER_ONE_HOUR;
    else if ( strcmp(s_unit, "min") == 0 )
      t *= TIMER_ONE_MINUTE;
    else if ( strcmp(s_unit, "s") == 0 )
      t *= TIMER_ONE_SECOND;
    else if ( strcmp(s_unit, "ms") == 0 )
      t *= TIMER_ONE_MS;
    else {
      shell_error(shell, "Illegal time unit '%s'. Please choose among {h,min,s,ms}.\n", s_unit);
      return -1;
    }
  }

  *pt = t;
  return 0;
}


/*-----------------------------------------------------------*/
/* Time value to struct timeval conversion                   */
/*-----------------------------------------------------------*/

void timer_tv(struct timeval *tv, unsigned long t)
{
  tv->tv_sec = t / TIMER_ONE_SECOND;
  tv->tv_usec = (t % TIMER_ONE_SECOND) * 1000;
}


#if 0
/*-----------------------------------------------------------*/
/* Timers bank management                                    */
/*-----------------------------------------------------------*/

typedef struct {
  char *id;
  struct itimerval it;
} timer_t;


timer_t *timer_tab = NULL;
int timer_tab_count = 0;


static timer_t *timer_alloc(char *id)
{
  timer_t *t = NULL;
  int i;

  /* Find a free entry in timer table */
  for (i = 0; (i < timer_tab_count) && (t == NULL); i++) {
    t = &(timer_tab[i]);
    if ( t->id != NULL )
      t = NULL;
  }

  /* No free entry: allocate a new one */
  if ( t == NULL ) {
    timer_tab = (timer_t *) realloc(timer_tab, sizeof(timer_t) * (timer_tab_count+1));
    t = &(timer_tab[timer_tab_count++]);
  }

  /* Setup new timer item */
  t->id = (id == NULL) ? NULL : strdup(id);

  return t;
}


static void timer_free(timer_t *t)
{
  if ( t == NULL )
    return;

  if ( t->id != NULL ) {
    free(t->id);
    t->id = NULL;
  }
}


static timer_t *timer_retrieve(char *id)
{
  timer_t *t = NULL;
  int i;

  if ( timer_tab == NULL )
    return NULL;

  for (i = 0; (i < timer_tab_count) && (t == NULL); i++) {
    t = &(timer_tab[i]);
    if ( (t->id == NULL) || (strcmp(t->id, id) != 0) )
      t = NULL;
  }

  return t;
}


int timer_set(char *id, unsigned long v)
{
  timer_t *t = timer_retrieve(id);

  /* Timer not found: allocate a new one */
  if ( t == NULL )
    t = timer_alloc(id);

  /* (Re)Start timer */
  timer_tv(&(t->it.it_interval), 0);
  timer_tv(&(t->it.it_value), v);
  if ( setitimer(ITIMER_REAL, &(t->it), NULL) == -1 )
    return -1;

  return 0;
}


unsigned long timer_get(char *id)
{
  timer_t *t = timer_retrieve(id);

  /* Check timer actually exists */
  if ( t == NULL )
    return -1;

  /* Fetch timer progress */
  /* TODO */

  return 0;
}


int timer_init(void)
{
  timer_tab = NULL;
  timer_tab_count = 0;

  return 0;
}


void timer_done(void)
{
  int i;

  for (i = 0; i < timer_tab_count; i++) {
    timer_t *t = &(timer_tab[i]);
    
    /* Cancel timer */
    t->it.it_interval.tv_sec = 0;
    t->it.it_interval.tv_usec = 0;
    t->it.it_value.tv_sec = 0;
    t->it.it_value.tv_usec = 0;
    setitimer(ITIMER_REAL, &(t->it), NULL);

    /* Free timer entry */
    timer_free(t);
  }
  free(timer_tab);

  timer_tab = NULL;
  timer_tab_count = 0;
}

#endif
