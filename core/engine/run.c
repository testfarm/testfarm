/****************************************************************************/
/* TestFarm                                                                 */
/* Test Engine - Runtime processing                                         */
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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/time.h>
#include <glib.h>

#include "useful.h"
#include "debug.h"
#include "shell.h"
#include "periph.h"
#include "trig.h"
#include "listener.h"
#include "result.h"
#include "timer.h"
#include "nibble.h"
#include "run.h"

#undef DEBUG_SELECT

#define PROMPT  "ENGINE> "

#define LF '\n'
#define _LF "\n"

#define TAG_WITH  "WITH   "
#define TAG_CLOSE "CLOSE  "

static periph_t *run_periph = NULL;
static shell_t *run_shell = NULL;
static periph_item_t *run_with = NULL;
static long run_timeout = -1;

static FILE *run_wait_output = NULL;
static long run_wait_timeout = -1;


/*-----------------------------------------------*/
/* Prompting management                          */
/*-----------------------------------------------*/

static void run_prompt_cr(shell_t *shell)
{
  if ( shell->interactive ) {
    printf("\r");
    fflush(stdout);
  }
}

static void run_prompt(shell_t *shell)
{
  if ( shell->interactive ) {
    printf("\r"PROMPT);
    fflush(stdout);
  }
}


/*-----------------------------------------------*/
/* Waiting management                            */
/*-----------------------------------------------*/

static int run_exec_with_periph(shell_t *shell, char *s);


static int run_wait_event_disconnect(periph_item_t *item, shell_t *shell)
{
  /* Close peripheral connection */
  if ( periph_close(run_periph, item) == -1 ) {
    shell_error(shell, "!! PANIC !! Cannot close peripheral '%s'\n", item->id);
    return -1;
  }

  /* Report connections status for newly connected peripherals */
  run_prompt_cr(shell);
  result_dump_engine(TAG_CLOSE, periph_item_info(item));

  /* Remove peripheral from WITH specifications */
  if ( item == run_with ) {
    run_with = NULL;
    result_dump_engine(TAG_WITH, "NULL");
  }

  run_prompt(shell);
  return 0;
}


static int run_wait_event_process(shell_t *shell, int fd, char **pbuf)
{
  periph_item_t *item = periph_fd_retrieve(run_periph, fd);
  char *buf;

  /* Should never occur... */
  if ( item == NULL ) {
    shell_error(shell, "!! PANIC !! Unable to retrieve peripheral from fd=%d\n", fd);
    return -1;
  }

#ifdef DEBUG_SELECT
  if ( debug_flag ) { /* For speedup in non-debug mode */
    debug("Something to read from '%s' (fd=%d)\n", item->id, fd);
  }
#endif

  /* Fetch data from peripheral: buf will be set to NULL
     if no complete message is available */
  run_prompt_cr(shell);
  if ( result_dump_periph(item, &buf) == -1 )
    return -1;
  run_prompt(shell);

  /* Check awaited patterns if a complete message arrived */
  if ( buf != NULL ) {
    trig_update(item, buf);
  }

  if ( pbuf != NULL )
    *pbuf = buf;

  return 0;
}


static void run_wait_event_unblock(shell_t *shell, char *tag, listener_t *listener)
{
  char *s = listener_raised(listener);

  if ( debug_flag ) { /* For speedup in non-debug mode */
    debug("Wait conditions unblocked: %s\n", s);
  }

  /* Replicate message to WAIT synchronization output */
  if ( run_wait_output != NULL ) {
    fputs(s, run_wait_output);
    fputs(_LF, run_wait_output);
  }

  /* Dump event to result file */
  run_prompt_cr(shell);

  result_puts(result_header_engine(tag));
  result_puts("CONTINUE ");
  result_puts(s);
  result_puts("\n");

  run_prompt(shell);

  free(s);
}


static void run_wait_event_timeout(shell_t *shell, char *tag)
{
  /* A wait timeout occured */
  if ( debug_flag ) { /* For speedup in non-debug mode */
    debug("Timeout!\n");
  }

  /* Send blank message to WAIT synchronization output */
  if ( run_wait_output != NULL ) {
    fputs(_LF, run_wait_output);
  }

  /* Dump event to result file */
  run_prompt_cr(shell);
  result_dump_engine(tag, "TIMEOUT %ld.%03ld s", run_wait_timeout / 1000, run_wait_timeout % 1000);
  run_prompt(shell);
}


static long run_wait_event(shell_t *shell, char *tag, struct timeval *ptv)
{
  fd_set rd;
  int max;
  int fd;
  int count;
  char *msg;

  /* Wait for incoming data */
  max = 0;
  do {
    /* Build fd set */
    max = periph_fd_set(run_periph, &rd);

    count = select(max, &rd, NULL, NULL, ptv);
    if ( (count == -1) && (errno != EINTR) ) {
      shell_error(shell, "Select i/o error: %s\n", strerror(errno));
      return -1;
    }
  } while ( count == -1 );

  /* Timeout ? */
  if ( count == 0 )
    return 0;

#ifdef DEBUG_SELECT
  if ( debug_flag ) { /* For speedup in non-debug mode */
    debug("Select returned something to read (%d)\n", count);
  }
#endif

  /* Process file descriptors that need to be read */
  for (fd = 0; fd < max; fd++) {
    if ( FD_ISSET(fd, &rd) ) {
      if ( run_wait_event_process(shell, fd, &msg) )
        return -1;
    }
  }

  return 1;
}



static int run_wait_input(shell_t *shell, int input_fd)
{
  fd_set rd;
  int max;
  int fd;
  int count;

  max = 0;
  do {
    /* Wait for incoming events */
    max = periph_fd_set(run_periph, &rd);

    /* Wait for incoming commands */
    FD_SET(input_fd, &rd);
    if ( input_fd >= max )
      max = input_fd + 1;

    count = select(max, &rd, NULL, NULL, NULL);
    if ( (count == -1) && (errno != EINTR) ) {
      shell_error(shell, "Select i/o error: %s\n", strerror(errno));
      return -1;
    }
  } while ( count == -1 );

  if ( count > 0 ) {
#ifdef DEBUG_SELECT
    if ( debug_flag ) { /* For speedup in non-debug mode */
      debug("Select returned something to read (%d)\n", count);
    }
#endif

    /* Process file descriptors that need to be read */
    for (fd = 0; fd < max; fd++) {
      if ( FD_ISSET(fd, &rd) ) {
        if ( fd == input_fd ) {
#ifdef DEBUG_SELECT
          /* Command from input*/
          if ( debug_flag ) { /* For speedup in non-debug mode */
            debug("Something to read from input\n");
          }
#endif
          return 1;
        }
        else {
          /* Event from peripheral */
          if ( run_wait_event_process(shell, fd, NULL) )
            return -1;
        }
      }
    }
  }

  return 0;
}


/*-----------------------------------------------*/
/* Peripheral retrieval                          */
/*-----------------------------------------------*/

static periph_item_t *run_retrieve_peripheral(shell_t *shell, char *id)
{
  periph_item_t *item;

  /* Upper case only */
  strupper(id);

  /* Retrieve peripheral item */
  item = periph_retrieve(run_periph, id);
  if ( item == NULL ) {
    shell_error(shell, "Unknown target peripheral '%s'\n", id);
    return NULL;
  }

  return item;
}


/*-----------------------------------------------*/
/* Shell command: CASE                           */
/*-----------------------------------------------*/

static nibble_t *run_exec_case_buf = NULL;

static int run_exec_case_running(void)
{
  return (run_exec_case_buf != NULL) && (run_exec_case_buf->buf[0] != NUL);
}

static int run_exec_case(shell_t *shell, shell_argv_t *cmd_argv, char *tag)
{
  int argc = cmd_argv->argc;
  char **argv = cmd_argv->argv;
  char *p;
  int i;

  /* Check arguments */
  if ( argc < 2 ) {
    shell_error(shell, "%s: too few arguments\n", argv[0]);
    shell_std_help(shell, argv[0]);
    return -1;
  }

  /* Check CASE/DONE nesting */
  if ( run_exec_case_running() ) {
    shell_error(shell, "%s: Test case already in progress (%s)\n", argv[0], run_exec_case_buf->buf);
    return -1;
  }

  /* Open local result file */
  if ( result_local_get() == NULL ) {
    shell_error(shell, "%s: No local result file. Please use command 'result'\n", argv[0]);
    return -1;
  }
  if ( result_local_open() == -1 ) {
    shell_error(shell, "%s: Cannot open local result file (%s)\n", argv[0], result_local_get());
    return -1;
  }

  /* Put global log Test Case boundary */
  result_global_set_case(argv[1]);

  /* Allocate message buffer, with some room for '[]' */
  run_exec_case_buf = nibble_realloc(run_exec_case_buf, cmd_argv->s_len + 3);
  p = run_exec_case_buf->buf;

  for (i = 1; i < argc; i++) {
    char *s = argv[i];

    if ( i >= 2 )
      *(p++) = ' ';
    if ( i == 2 )
      *(p++) = '[';

    while ( *s != NUL )
      *(p++) = *(s++);

    if ( (i >= 2) && (i == (argc-1)) )
      *(p++) = ']';
  }
  *p = NUL;

  /* Dump message */
  result_dump_engine(tag, run_exec_case_buf->buf);

  return 0;
}


/*-----------------------------------------------*/
/* Shell command: DONE                           */
/*-----------------------------------------------*/

static int run_exec_done(shell_t *shell, shell_argv_t *cmd_argv, char *tag)
{
  int argc = cmd_argv->argc;
  char **argv = cmd_argv->argv;

  /* Check arguments */
  if ( argc > 1 ) {
    shell_error(shell, "%s: too many arguments\n", argv[0]);
    shell_std_help(shell, argv[0]);
    return -1;
  }

  /* Check CASE/DONE nesting */
  if ( ! run_exec_case_running() ) {
    shell_error(shell, "%s: No test case in progress\n", argv[0]);
    return -1;
  }

  /* Dump message */
  result_dump_engine(tag, run_exec_case_buf->buf);

  /* Close local result file */
  result_local_close();

  /* Also dump wait synchronization output */
  if ( run_wait_output != NULL ) {
    fputs(run_exec_case_buf->buf, run_wait_output);
    fputs(_LF, run_wait_output);
  }

  /* Clear CASE name */
  run_exec_case_buf->buf[0] = NUL;

  return 0;
}


/*-----------------------------------------------*/
/* Shell command: VERDICT                        */
/*-----------------------------------------------*/

const char *run_exec_verdict_tab[] = {
  "PASSED", "FAILED", "INCONCLUSIVE", "SKIP", NULL
};

static int run_exec_verdict(shell_t *shell, shell_argv_t *cmd_argv, char *tag)
{
  int argc = cmd_argv->argc;
  char **argv = cmd_argv->argv;
  char *verdict;
  int i;

  /* Check arguments */
  if ( (argc < 2) || (argc > 3) ) {
    shell_error(shell, "%s: too %s arguments\n", argv[0], (argc < 2) ? "few":"many");
    shell_std_help(shell, argv[0]);
    return -1;
  }

  /* Check CASE/DONE nesting */
  if ( run_exec_case_running() ) {
    shell_error(shell, "%s: Test case in progress (%s)\n", argv[0], run_exec_case_buf->buf);
    return -1;
  }

  /* Check result lexical correctness */
  strupper(argv[1]);
  verdict = NULL;
  for (i = 0; (verdict == NULL) && (run_exec_verdict_tab[i] != NULL); i++) {
    verdict = (char *) run_exec_verdict_tab[i];
    if ( strncmp(verdict, argv[1], strlen(argv[1])) != 0 )
      verdict = NULL;
  }

  if ( verdict == NULL ) {
    shell_error(shell, "%s: Illegal verdict result '%s'\n", argv[0], argv[1]);
    return -1;
  }

  /* Dump message */
  if ( argc > 2 )
    result_dump_engine(tag, "%s %s", verdict, argv[2]);
  else
    result_dump_engine(tag, "%s", verdict);

  return 0;
}


/*-----------------------------------------------*/
/* Shell command: TIMEOUT                        */
/*-----------------------------------------------*/

static int run_exec_timeout(shell_t *shell, shell_argv_t *cmd_argv, char *tag)
{
  int argc = cmd_argv->argc;
  char **argv = cmd_argv->argv;

  /* Check arguments */
  if ( argc > 3 ) {
    shell_error(shell, "%s: too many arguments\n", argv[0]);
    shell_std_help(shell, argv[0]);
    return -1;
  }

  /* Setup new timeout value if supplied */
  if ( argc > 1 ) {
    if ( timer_value(shell, argv[1], argv[2], &run_timeout) )
      return -1;
  }

  /* Write info to result file */
  if ( run_timeout > 0 )
    result_dump_engine(tag, "%ld.%03ld s", run_timeout / 1000, run_timeout % 1000);
  else
    result_dump_engine(tag, "No timeout defined");

  return 0;
}


/*-----------------------------------------------*/
/* Shell command: WITH                           */
/*-----------------------------------------------*/

static int run_exec_with_periph(shell_t *shell, char *s)
{
  periph_item_t *item;

  /* Clear previously selected peripheral */
  run_with = NULL;

  /* NULL peripheral ? */
  if ( strcmp(s, "NULL") == 0 ) {
    debug("Default WITH target set to NULL\n");
    return 0;
  }

  /* Retrieve peripheral item */
  if ( (item = periph_retrieve(run_periph, s)) == NULL ) {
    shell_error(shell, "Unknown target peripheral '%s'\n", s);
    return -1;
  }

  /* Set default peripheral item */
  run_with = item;

  return 0;
}


static int run_exec_with(shell_t *shell, shell_argv_t *cmd_argv, char *tag)
{
  int argc = cmd_argv->argc;
  char **argv = cmd_argv->argv;

  /* Check arguments */
  if ( argc > 2 ) {
    shell_error(shell, "%s: too many arguments\n", argv[0]);
    shell_std_help(shell, argv[0]);
    return -1;
  }

  /* Select default peripheral if requested */
  if ( argc == 2 ) {
    if ( run_exec_with_periph(shell, strupper(argv[1])) )
      return -1;
  }

  /* Write info to result file */
  result_dump_engine(tag, (run_with != NULL) ? run_with->id : "NULL");

  return 0;
}


/*-----------------------------------------------*/
/* Shell command: TRIG                           */
/*   trig def, trig undef, trig clear            */
/*-----------------------------------------------*/

static int run_exec_trig(shell_t *shell, shell_argv_t *cmd_argv, char *tag)
{
  int argc = cmd_argv->argc;
  char **argv = cmd_argv->argv;
  int ret = 0;

  /* No arguments: show triggers */
  if ( argc < 2 ) {
    shell_error(shell, "%s: too fee arguments\n", argv[0]);
    shell_std_help(shell, argv[0]);
    return -1;
  }

  /* trig def [<id> [@[<periph>]] [<regex>]] */
  if ( strcmp(argv[1], "def") == 0 ) {
    periph_item_t *periph;
    char *id = argv[2];
    int argc2 = argc - 3;
    char **argv2 = argv + 3;

    /* Retrieve peripheral source */
    periph = run_with;
    if ( (argc2 > 0) && (argv2[0][0] == '@') ) {
      char *args = &(argv2[0][1]);

      if ( args[0] != NUL ) {
        periph = run_retrieve_peripheral(shell, args);
        if ( periph == NULL )
          return -1;
      }
      else {
        periph = NULL;
      }

      argv2++;
      argc2--;

      if ( argc2 <= 0 ) {
        shell_error(shell, "%s: missing <regex> argument\n", argv[0]);
        shell_std_help(shell, argv[0]);
        ret = -1;
      }
    }

    if ( ret == 0 )
      ret = trig_def(shell, id, periph, argv2, argc2);
  }

  /* trig undef [<id> ...] */
  else if ( strcmp(argv[1], "undef") == 0 ) {
    ret = trig_undef(argv+2, argc-2);
  }

  /* trig info <id>*/
  else if ( strcmp(argv[1], "info") == 0 ) {
    if ( argc == 3 ) {
      char *id = strupper(argv[2]);
      char *info;

      if ( trig_info(id, &info) == -1 ) {
        shell_error(shell, "%s: unknown trigger '%s'\n", argv[0], id);
        ret = -1;
      }
      else {
        if ( info == NULL ) {
          info = "";
        }

        /* Dump message */
        result_dump_engine(tag, "%s '%s'", id, info);

        /* Also dump wait synchronization output */
        if ( run_wait_output != NULL ) {
          fputs(info, run_wait_output);
          fputs(_LF, run_wait_output);
        }
      }
    }
    else {
      shell_error(shell, "%s: too %s arguments\n", argv[0], (argc < 3) ? "few" : "many");
      shell_std_help(shell, argv[0]);
      ret = -1;
    }
  }

  /* trig count <id> */
  else if ( strcmp(argv[1], "count") == 0 ) {
    if ( argc == 3 ) {
      char *id = strupper(argv[2]);
      int count = trig_count(id);

      if ( count < 0 ) {
        shell_error(shell, "%s: unknown trigger '%s'\n", argv[0], id);
        ret = -1;
      }
      else {
        result_dump_engine(tag, "%s %d", id, count);

        /* Dump wait synchronization output */
        if ( run_wait_output != NULL )
          fprintf(run_wait_output, "%d\n", count);
      }
    }
    else {
      shell_error(shell, "%s: too %s arguments\n", argv[0], (argc < 3) ? "few" : "many");
      shell_std_help(shell, argv[0]);
      ret = -1;
    }
  }

  /* trig clear [<id> ...] */
  else if ( strcmp(argv[1], "clear") == 0 ) {
    ret = trig_clear(argv+2, argc-2);
  }

  /* Unknown operation */
  else {
    shell_error(shell, "%s: illegal trigger operation (%s)\n", argv[0], argv[1]);
    shell_std_help(shell, argv[0]);
    ret = -1;
  }

  return ret;
}


/*-----------------------------------------------*/
/* Shell command: WAIT                           */
/*-----------------------------------------------*/

static int tv_sub(struct timeval *tv1, struct timeval *tv0)
{
  tv1->tv_sec -= tv0->tv_sec;
  tv1->tv_usec -= tv0->tv_usec;

  if ( tv1->tv_usec < 0 ) {
    tv1->tv_sec -= 1;
    tv1->tv_usec = 1000000 + tv1->tv_usec;
  }

  if ( (tv1->tv_sec < 0) ||
       ((tv1->tv_sec == 0) && (tv1->tv_usec == 0)) )
    return 1;

  return 0;
}


static int run_exec_wait(shell_t *shell, shell_argv_t *cmd_argv, char *tag)
{
  int argc = cmd_argv->argc;
  char **argv = cmd_argv->argv;
  listener_t *listener;
  char *s;
  struct timeval tv0;
  long sec, usec;
  int unblock, timeout, error;

  /* Set and check wait timeout */
  run_wait_timeout = run_timeout;
  if ( (argc > 1) && (argv[1][0] == '-') ) {
    if ( timer_value(shell, &(argv[1][1]), NULL, &run_wait_timeout) )
      return -1;
    argv++;
    argc--;
  }

  if ( run_wait_timeout == -1 ) {
    shell_error(shell, "No timeout value initialized. Please use command 'timeout' before 'wait'\n");
    return -1;
  }

  /* Setup wait conditions */
  argv++;
  argc--;
  if ( (listener = listener_new(argc, argv)) == NULL )
    return -1;

  /* Report what we are waiting for */
  s = listener_str(listener);

  if ( debug_flag ) { /* For speedup in non-debug mode */
    debug("Wait: '%s'\n", s);
  }

  result_puts(result_header_engine(tag));
  result_puts(s);
  result_puts("\n");

  free(s);

  /* Init timeout counter */
  gettimeofday(&tv0, NULL);
  sec = run_wait_timeout / 1000;
  usec = (run_wait_timeout % 1000) * 1000;

  /* Wait until an unblocking condition is met */
  unblock = 0;
  timeout = 0;
  error = 0;
  while ( ! (unblock || timeout || error) ) {
    /* Check unblocking conditions */
    if ( listener_check(listener) ) {
      unblock = 1;
    }
    else {
      struct timeval tv1, tv;

      /* Update timeout counter */
      gettimeofday(&tv1, NULL);
      tv_sub(&tv1, &tv0);
      /*fprintf(stderr, "** %ld.%06ld\n", tv1.tv_sec, tv1.tv_usec);*/

      tv.tv_sec = sec;
      tv.tv_usec = usec;
      if ( tv_sub(&tv, &tv1) ) {
        timeout = 1;
      }
      else {
        /*fprintf(stderr, "   %ld.%06ld\n", tv.tv_sec, tv.tv_usec);*/
        /* Wait for events to arrive, and process incoming data */
        switch ( run_wait_event(shell, tag, &tv) ) {
        case -1:
          error = 1;
          break;
        case 0:
          timeout = 1;
          break;
        default :
          break;
        }
      }
    }
  }

  if ( unblock ) {
    run_wait_event_unblock(shell, tag, listener);
  }
  else if ( timeout ) {
    run_wait_event_timeout(shell, tag);
  }

  /* Clear wait conditions */
  listener_destroy(listener);

  return error ? -1 : 0;
}


/*-----------------------------------------------*/
/* Shell command: OPEN                           */
/*-----------------------------------------------*/

static int run_exec_open(shell_t *shell, shell_argv_t *cmd_argv, char *tag)
{
  int argc = cmd_argv->argc;
  char **argv = cmd_argv->argv;
  int i;

  /* Connect to peripherals if required */
  if ( argc > 1 ) {
    for (i = 1; i < argc; i++) {
      /* Retrieve peripheral item */
      periph_item_t *item = periph_retrieve(run_periph, strupper(argv[i]));
      if ( item == NULL ) {
        shell_error(shell, "Peripheral '%s' not declared. Please use command 'periph'\n", argv[i]);
        return -1;
      }

      /* Open peripheral connection */
      if ( periph_open(run_periph, item) == -1 ) {
        shell_error(shell, "Cannot connect to peripheral '%s'\n", argv[i]);
        return -1;
      }

      /* Report connections status for newly connected peripherals */
      result_dump_engine(tag, periph_item_info(item));
    }
  }
  else {
    /* Report connections status for all connected peripherals */
    for (i = 0; i < run_periph->fd_count; i++) {
      periph_item_t *item = run_periph->fd_tab[i];
      if ( item != NULL )
        result_dump_engine(tag, periph_item_info(item));
    }
  }

  return 0;
}


/*-----------------------------------------------*/
/* Shell command: CLOSE                          */
/*-----------------------------------------------*/

static int run_exec_close_item(periph_item_t *item, char *tag)
{
  char *info;

  if ( item == NULL )
    return 0; /* Be silent if item does not exist */

  /* Close peripheral connection */
  if ( periph_close(run_periph, item) == -1 )
    return 0; /* Be silent if there is nothing to close */

  /* Report connections status for newly connected peripherals */
  info = periph_item_info(item);
  result_dump_engine(tag, info);

  /* Remove peripheral from WITH specification */
  if ( item == run_with ) {
    run_with = NULL;
    result_dump_engine(TAG_WITH, "NULL");
  }

  return 1;
}


static int run_exec_close(shell_t *shell, shell_argv_t *cmd_argv, char *tag)
{
  int argc = cmd_argv->argc;
  char **argv = cmd_argv->argv;
  char *str = NULL;
  int len = 0;
  int i;

  /* Check arguments */
  if ( argc < 2 ) {
    shell_error(shell, "%s: too few arguments\n", argv[0]);
    shell_std_help(shell, argv[0]);
    return -1;
  }

  /* Disconnect from peripherals */
  for (i = 1; i < argc; i++) {
    periph_item_t *item = periph_retrieve(run_periph, strupper(argv[i]));
    if ( run_exec_close_item(item, tag) ) {
      int idx = len;
      len += strlen(item->id) + 1;
      str = realloc(str, len+1);
      sprintf(str+idx, "%s ", item->id);
    }
  }

  /* Dump wait synchronization output */
  if ( run_wait_output != NULL ) {
    if ( str != NULL )
      fputs(str, run_wait_output);
    fputs(_LF, run_wait_output);
  }

  if ( str != NULL )
    free(str);

  return 0;
}


/*-----------------------------------------------*/
/* Shell command: HELP                           */
/*-----------------------------------------------*/

static int run_exec_help(shell_t *shell, shell_argv_t *cmd_argv, char *tag)
{
  int argc = cmd_argv->argc;
  char **argv = cmd_argv->argv;

  /* Check availability */
  if ( ! shell->interactive ) {
    shell_error(shell, "Command '%s' not allowed in non-interactive mode\n", argv[0]);
    return -1;
  }

  /* Check arguments */
  if ( argc == 2 )
    shell_std_help(shell, argv[1]);
  else
    shell_std_help(shell, NULL);

  return 0;
}


/*-----------------------------------------------*/
/* Shell command: PERIPH                         */
/*-----------------------------------------------*/

static int run_exec_periph(shell_t *shell, shell_argv_t *cmd_argv, char *tag)
{
  int argc = cmd_argv->argc;
  char **argv = cmd_argv->argv;
  periph_item_t *item;
  char *info;

  /* User asked to show the whole peripheral list */
  if ( argc < 2 ) {
    periph_info(run_periph, result_header_engine(tag), (periph_info_method_t *) result_puts);
    return 0;
  }

  /* User asked to add or show a peripheral */
  item = periph_retrieve(run_periph, strupper(argv[1]));
  if ( argc < 3 ) {
    if ( item != NULL )
      result_dump_engine(tag, periph_item_info(item));
    return 0;
  }

  /* Check arguments */
  if ( argc > 5 ) {
    shell_error(shell, "%s: Too many arguments\n", argv[0]);
    shell_std_help(shell, argv[0]);
    return -1;
  }

  /* User asked to add a new peripheral: check it is not already declared */
  if ( item != NULL ) {
    debug("Redeclaring peripheral: %s\n", argv[1]);

    /* Peripheral already declared: close it and remove it */
    run_exec_close_item(item, TAG_CLOSE);
    if ( (item = periph_item_init(argv+1, item)) == NULL )
      return -1;
  }
  else {
    debug("New peripheral: %s\n", argv[1]);

    /* Create new peripheral in list */
    if ( (item = periph_item_init(argv+1, NULL)) == NULL )
      return -1;
    periph_item_closed_set(item, (periph_item_closed_t *) run_wait_event_disconnect, (void *) shell);
    periph_new(run_periph, item);
  }

  /* Display new peripheral */
  info = periph_item_info(item);
  result_dump_engine(tag, info);

  /* Also dump wait synchronization output */
  if ( run_wait_output != NULL ) {
    fputs(info, run_wait_output);
    fputs(_LF, run_wait_output);
  }

  return 0;
}


/*-----------------------------------------------*/
/* Shell command: LOCAL                          */
/*-----------------------------------------------*/

static int run_exec_local(shell_t *shell, shell_argv_t *cmd_argv, char *tag)
{
  int argc = cmd_argv->argc;
  char **argv = cmd_argv->argv;
  char *s;

  /* Check arguments */
  if ( argc > 2 ) {
    shell_error(shell, "%s: too many arguments\n", argv[0]);
    shell_std_help(shell, argv[0]);
    return -1;
  }

  /* Set local log file if requested */
  if ( argc == 2 ) {
    if ( result_local_set(argv[1]) == -1 ) {
      shell_error(shell, "%s: cannot open local log file %s\n", argv[0], argv[1]);
      return -1;
    }
  }

  /* Show local log file */
  if ( (s = result_local_get()) != NULL )
    result_dump_engine(tag, s);

  return 0;
}


/*-----------------------------------------------*/
/* Shell command: GLOBAL                         */
/*-----------------------------------------------*/

static int run_exec_global(shell_t *shell, shell_argv_t *cmd_argv, char *tag)
{
  int argc = cmd_argv->argc;
  char **argv = cmd_argv->argv;
  char *s;

  /* Check arguments */
  if ( argc > 2 ) {
    shell_error(shell, "%s: too many arguments\n", argv[0]);
    shell_std_help(shell, argv[0]);
    return -1;
  }

  /* Set global log file if requested */
  if ( argc == 2 ) {
    if ( shell->interactive ) {
      shell_error(shell, "%s: cannot change global log file: interactive shell mode\n", argv[0]);
      return -1;
    }

    if ( result_global_set(argv[1]) == -1 ) {
      shell_error(shell, "%s: cannot open global log file %s\n", argv[0], argv[1]);
      return -1;
    }
  }

  /* Show global log file */
  if ( (s = result_global_get()) != NULL )
    result_dump_engine(tag, s);
  else
    result_dump_engine(tag, "<STDOUT>");

  return 0;
}


/*-----------------------------------------------*/
/* Shell command: VERSION                        */
/*-----------------------------------------------*/

static int run_exec_version(shell_t *shell, shell_argv_t *cmd_argv, char *tag)
{
  /* Check arguments */
  if ( cmd_argv->argc > 1 ) {
    shell_error(shell, "%s: too many arguments\n", cmd_argv->argv[0]);
    shell_std_help(shell, cmd_argv->argv[0]);
    return -1;
  }

  /* Display version */
  result_dump_engine(tag, VERSION " (" __DATE__ ")");

  return 0;
}


/*-----------------------------------------------*/
/* Shell command: DO (peripheral command)        */
/*-----------------------------------------------*/

static int run_exec_command(shell_t *shell, shell_argv_t *cmd_argv, char *tag)
{
  int argc = cmd_argv->argc;
  char **argv = cmd_argv->argv;
  char *args = argv[1];
  periph_item_t *item;
  int len;

  /* Check arguments */
  if ( (argc != 2) && (argc != 3) ) {
    shell_error(shell, "%s: too %s arguments\n", argv[0], (argc > 3) ? "many" : "few");
    shell_std_help(shell, argv[0]);
    return -1;
  }

  /* Retrieve target peripheral */
  if ( args[0] == '@' ) {
    args++;

    /* Get peripheral descriptor */
    item = run_retrieve_peripheral(shell, args);
    if ( item == NULL )
      return -1;

    /* Get command to be sent to the peripheral */
    args = argv[2];
  }
  else {
    /* Check there is a default peripheral */
    if ( run_with == NULL ) {
      result_dump_engine(tag, "No default peripheral selected. Please use command 'with'");
      return 0;
    }

    item = run_with;
  }

  /* Do nothing if the peripheral is not connected */
  if ( ! periph_item_connected(item) ) {
    result_dump_engine(tag, "Target peripheral '%s' not connected. Please use command 'open'", item->id);
    return 0;
  }

  /* Send instruction to peripheral */
  len = strlen(args);
  args[len] = '\n';
  periph_item_write(item, args, len+1);
  args[len] = NUL;

  return 0;
}


/*-----------------------------------------------*/
/* Shell command: ECHO                           */
/*-----------------------------------------------*/

static int run_exec_echo(shell_t *shell, shell_argv_t *cmd_argv, char *tag)
{
  int argc = cmd_argv->argc;
  char **argv = cmd_argv->argv;
  int i;

  /* Get tag modifier if any */
  if ( argv[1][0] == '-' ) {
    char *s = g_strstrip(&(argv[1][1]));
    if ( *s != '\0' )
      tag = s;

    argc--;
    argv++;
  }

  /* Display report header */
  result_puts(result_header_engine(tag));

  /* Display argument list */
  for (i = 1; i < argc; i++) {
    if ( i > 1 )
      result_puts(" ");
    result_puts(argv[i]);
  }
  result_puts("\n");

  return 0;
}


/*-----------------------------------------------*/
/* Shell command: unknown                        */
/*-----------------------------------------------*/

static int run_exec_unknown(shell_t *shell, shell_argv_t *cmd_argv, char *tag)
{
  /* OS-shell command ? */
  if ( cmd_argv->argv[0][0] == '!' ) {
    /* Check availability */
    if ( ! shell->interactive ) {
      shell_error(shell, "Cannot invoke OS-shell command in non-interactive mode\n");
      return -1;
    }

    return shell_std_system(shell, cmd_argv);
  }

  shell_error(shell, "Unknown command '%s'\n", cmd_argv->argv[0]);
  shell_std_help(shell, NULL);

  return -1;
}



/*-----------------------------------------------*/
/* Shell main loop                               */
/*-----------------------------------------------*/

static shell_std_help_t run_help[] = {
  { "case",    "case <testcase> [<comment>]\n"
               "  Mark the beginning of a test case\n" },
  { "done",    "done\n"
               "  Mark the end of a test case\n" },
  { "verdict", "verdict PASSED|FAILED|INCONCLUSIVE|SKIP\n"
               "  Dump the verdict of a test case to result file\n" },
  { "trig",    "trig def [<id>]\n"
               "  Show definition of trigger <id> (or all triggers if ommited) \n"
               "trig def <id> [@[<periph>]] <regex>\n"
               "  Define trigger <id> for detecting an event matching regular expression <regex>.\n"
               "  If argument @ is given before <regex>, event is awaited from any peripheral.\n"
               "  If argument @<periph> is ommitted, event is awaited from the default peripheral.\n"
               "  <regex> conforms to the PERL regular expression format.\n"
               "trig undef [<id>]\n"
               "  Remove trigger <id> (or all triggers if ommited)\n"
               "trig clear [<id>]\n"
               "  Clear trigger <id> (or all triggers if ommited)\n"
               "trig info <id>\n"
               "  Get data upon which trigger was last raised\n"
               "trig count <id>\n"
               "  Get trigger raise counter\n" },
  { "wait",    "wait [-<timeout>] \"<boolean-expression>\"\n"
               "  Wait for one trigger or a boolean combination of triggers.\n"
               "  A trigger id may be suffixed with an occurence counter, e.g. MYTRIG*2.\n" },
  { "timeout", "timeout [<time> [h|min|s|ms]]\n"
               "  Set wait timeout to <time> milliseconds or [hours|minutes|seconds|milliseconds]\n" },
  { "with",    "with [<periph>|NULL]\n"
               "  Select & Show default peripheral\n" },
  { "open",    "open [<peripherals> ...]\n"
               "  Open & Show peripheral connection(s)\n" },
  { "close",   "close <peripherals> ...\n"
               "  Close peripheral connection(s)\n" },
  { "periph",  "periph [<name>]\n"
               "  Show interface(s)\n"
               "periph <name> [PROC://]<cmd-line> [-<flags>] [<comment>]\n"
               "periph <name> TCP://<ip-address>  [-<flags>] [<comment>]\n"
               "periph <name> SERIAL://<device>   [-<flags>] [<comment>]\n"
               "  Declare interface\n" },
  { "local",   "local [<filename>]\n"
               "  Setup/Show local log file\n" },
  { "global",  "global [<filename>]\n"
               "  Setup/Show global log file\n" },
  { "version", "version\n"
               "  Show Test Engine version\n" },
  { "do",      "do [@<periph>] \"<command> [<arguments>]\"\n"
               "  Send command to peripheral <periph>, or to default peripheral\n" },
  { NULL,     NULL }
};


static shell_command_t run_commands[] = {
  { "do",      run_exec_command, "DO     " },
  { "case",    run_exec_case,    "CASE   " },
  { "done",    run_exec_done,    "DONE   " },
  { "verdict", run_exec_verdict, "VERDICT" },
  { "trig",    run_exec_trig,    TAG_TRIG  },
  { "wait",    run_exec_wait,    "WAIT   " },
  { "timeout", run_exec_timeout, "TIMEOUT" },
  { "with",    run_exec_with,    TAG_WITH  },
  { "open",    run_exec_open,    "OPEN   " },
  { "close",   run_exec_close,   TAG_CLOSE },
  { "periph",  run_exec_periph,  "PERIPH " },
  { "result",  run_exec_local,   "RESULT " },  /* Deprecated, replaced with 'local' */
  { "local",   run_exec_local,   "LOCAL  " },
  { "global",  run_exec_global,  "GLOBAL " },
  { "version", run_exec_version, "VERSION" },
  { "help",    run_exec_help,    "HELP   " },
  { NULL,      run_exec_unknown, NULL      }
};


int run_loop(void)
{
  return shell_exec(run_shell);
}


static int run_prologue(shell_t *shell, void *arg)
{
  int input_fd = shell_get_fd(shell);
  int ret;

  /* Display prompt if interactive mode */
  run_prompt(shell);

  /* Wait for input while accepting events */
  while ( (ret = run_wait_input(shell, input_fd)) == 0 );
  if ( ret == -1 )
    return -1;

  return 0;
}


int run_init(periph_t *periph, int argc, char **argv, int interactive, char *wait)
{
  /* Open WAIT synchronization output */
  if ( wait != NULL ) {
    char *err;
    int fd = strtol(wait, &err, 10);

    if ( (err != NULL) && (*err > ' ') )
      fd = -1;

    if ( fd >= 0 ) {
      if ( (run_wait_output = fdopen(fd, "w")) == NULL ) {
        fprintf(stderr, NAME ": Cannot open WAIT synchronization output fd=%d: %s\n", fd, strerror(errno));
        return -1;
      }
    }
    else {
      if ( (run_wait_output = fopen(wait, "w")) == NULL ) {
        fprintf(stderr, NAME ": Cannot open WAIT synchronization output %s: %s\n", wait, strerror(errno));
        return -1;
      }
    }

    setbuf(run_wait_output, NULL);

    /* Child programs should not have access this file descriptor */
    fcntl(fileno(run_wait_output), F_SETFD, FD_CLOEXEC);
  }

  /* Store peripheral descriptor address */
  run_periph = periph;

  /* Setup runtime log gears */
  if ( result_init() )
    return -1;

  /* Create shell engine */
  run_shell = shell_alloc(NAME, argc, argv, NULL);

  /* Set interactive mode */
  shell_set_interactive(run_shell, interactive);

  /* Setup prologue routine */
  shell_set_prologue(run_shell, run_prologue, NULL);

  /* No input echo */
  shell_set_input_echo(run_shell, NULL, NULL);

  /* Setup special commands */
  shell_set_cmd(run_shell, run_commands);

  /* Setup standard commands, standard header routine, help table */
  shell_std_set_header(result_header_engine);
  shell_std_set_echo(run_exec_echo);
  shell_std_set_unknown(run_exec_unknown);
  shell_std_setup(run_shell, run_help);

  return 0;
}


int run_done(void)
{
  /* Free triggers */
  trig_undef(NULL, 0);

  /* Close WAIT synchronization output */
  if ( run_wait_output != NULL ) {
    fclose(run_wait_output);
    run_wait_output = NULL;
  }

  /* Free shell engine */
  if ( run_shell != NULL ) {
    shell_free(run_shell);
    run_shell = NULL;
  }

  /* Free BEGIN message buffer */
  nibble_free(run_exec_case_buf);
  run_exec_case_buf = NULL;

  /* Abort runtime log */
  result_done();

  return 0;
}
