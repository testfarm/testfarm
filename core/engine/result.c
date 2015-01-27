/****************************************************************************/
/* TestFarm                                                                 */
/* Test Engine - Result output management                                   */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 17-AUG-1999                                                    */
/****************************************************************************/

/*
 * $Revision: 174 $
 * $Date: 2006-07-26 17:43:56 +0200 (mer., 26 juil. 2006) $
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <sys/time.h>

#include "periph.h"


/*-------------------------------------------------*/
/* Time Stamp generator                            */
/*-------------------------------------------------*/

static int timestamp(char *buf)
{
  static long t0 = -1;
  time_t t;
  struct timeval tv;
  char *p = buf;

  /* Get time information */
  time(&t);
  gettimeofday(&tv, NULL);

  /* Set T0 date if not already done */
  if ( t0 < 0 )
    t0 = tv.tv_sec;

  /* Build human-readable time stamp */
  p += strftime(p, 32, "%d-%b-%Y %H:%M:%S ", localtime(&t));

  /* Build machine-processable timestamp */
  p += sprintf(p, "%ld%06ld ", tv.tv_sec - t0, tv.tv_usec);

  return (p - buf);
}


/*-------------------------------------------------*/
/* Global log management                           */
/*-------------------------------------------------*/

static char *result_global_path = NULL;
static FILE *result_global_fd = NULL;
static int result_global_case = 0;


void result_global_set_case(char *id)
{
  if ( result_global_case ) {
    fprintf(result_global_fd, "</LOG_CASE>\n");
    result_global_case = 0;
  }

  if ( id != NULL ) {
    fprintf(result_global_fd, "<LOG_CASE id=\"%s\">\n", id);
    result_global_case = 1;
  }
}


static void result_global_clear(void)
{
  /* Close global log file */
  if ( result_global_fd != NULL ) {
    result_global_set_case(NULL);
    fprintf(result_global_fd, "</LOG>\n");
    fclose(result_global_fd);
    result_global_fd = NULL;
  }

  /* Clear global log configuration */
  if ( result_global_path != NULL ) {
    free(result_global_path);
    result_global_path = NULL;
  }
}


int result_global_set(char *fname)
{
  /* Clear previous global log configuration */
  result_global_clear();

  if ( fname == NULL )
    return 0;

  /* File name "-" stands for stdout */
  if ( strcmp(fname, "-") == 0 )
    return 0;

  result_global_fd = fopen(fname, "w");
  if ( result_global_fd == NULL )
    return -1;
  setlinebuf(result_global_fd);

  /* Child programs should not have access this file descriptor */
  fcntl(fileno(result_global_fd), F_SETFD, FD_CLOEXEC);

  result_global_path = strdup(fname);

  fprintf(result_global_fd, "<LOG>\n");

  return 0;
}


char *result_global_get(void)
{
  return result_global_path;
}


/*-------------------------------------------------*/
/* Local log management                            */
/*-------------------------------------------------*/

static char *result_local_path = NULL;
static FILE *result_local_fd = NULL;

int result_local_open(void)
{
  if ( result_local_path == NULL )
    return 0;

  if ( (result_local_fd = fopen(result_local_path, "w")) == NULL )
    return -1;
  setlinebuf(result_local_fd);

  /* Child programs should not have access this file descriptor */
  fcntl(fileno(result_local_fd), F_SETFD, FD_CLOEXEC);

  return 0;
}


void result_local_close(void)
{
  if ( result_local_fd == NULL )
    return;

  fclose(result_local_fd);
  result_local_fd = NULL;

  if ( result_global_fd != NULL )
    fflush(result_global_fd);
  else
    fflush(stdout);
}


static void result_local_clear(void)
{
  /* Clear local result file configuration */
  result_local_close();
  if ( result_local_path != NULL ) {
    free(result_local_path);
    result_local_path = NULL;
  }
}


int result_local_set(char *fname)
{
  FILE *f;

  /* Clear previous configuration */
  result_local_clear();

  if ( fname == NULL )
    return 0;

  if ( (f = fopen(fname, "w")) == NULL )
    return -1;
  fclose(f);

  result_local_path = strdup(fname);

  return 0;
}


char *result_local_get(void)
{
  return result_local_path;
}


/*-------------------------------------------------*/
/* Generic dump services                           */
/*-------------------------------------------------*/

int result_init(void)
{
  return 0;
}


void result_done(void)
{
  /* Free log file configurations */
  result_local_clear();
  result_global_clear();
}


int result_puts(char *msg)
{
  /* Dump global result log */
  if ( result_global_fd != NULL ) {
    char *s = msg;

    while ( *s != '\0' ) {
      switch ( *s ) {
      case '<':
        fputs("&lt;", result_global_fd);
        break;
      case '&':
        fputs("&amp;", result_global_fd);
        break;
      case '>':
        fputs("&gt;", result_global_fd);
        break;
      default:
        fputc(*s, result_global_fd);
        break;
      }

      s++;
    }
  }
  else {
    fputs(msg, stdout);
  }

  /* Dump local result log */
  if ( result_local_fd != NULL )
    fputs(msg, result_local_fd);

  return 0;
}


/*-------------------------------------------------*/
/* Peripherals dump services                       */
/*-------------------------------------------------*/

char *result_header_periph(periph_item_t *item)
{
  static char buf[80];
  int ts_len;
  char *id_str;
  int id_max;
  int id_len;
  int offset;

  /* Just return the last header if periph argument is NULL */
  if ( item == NULL )
    return buf;

  /* Build global time stamp */
  ts_len = timestamp(buf);

  /* Compute other lengthes and offsets */
  id_max = sizeof(buf) - ts_len - 6;
  id_str = item->id;
  id_len = strlen(id_str);
  if ( id_len > id_max )
    id_len = id_max;
  offset = ts_len + id_len;

  /* Append peripheral id to time stamp */
  memcpy(buf + ts_len, id_str, id_len);
  buf[offset++] = ' ';

  /* Append a pseudo local header if peripheral does not supply one */
  if ( item->flags & PERIPH_FLAG_NOHEADER ) {
    strcpy(buf + offset, "* * ");
    offset += 4;
  }

  buf[offset] = '\0';

  return buf;
}


int result_dump_periph(periph_item_t *item, char **buf)
{
  int ret;

  if ( item == NULL )
    return 0;

  if ( buf != NULL )
    *buf = NULL;

  /* Read data from peripheral */
  if ( (ret = periph_item_read(item)) > 0 ) {
    /* Dump message header */
    result_puts(result_header_periph(item));

    /* Add message to test result log */
    result_puts(item->buf);

    /* Return result buffer address */
    if ( buf != NULL )
      *buf = item->buf;
  }

  return 0;
}


/*-------------------------------------------------*/
/* Test engine dump services                       */
/*-------------------------------------------------*/

char *result_header_engine(char *tag)
{
  static char buf[80];
  int ts_len;

  /* Build global time stamp */
  ts_len = timestamp(buf);

  sprintf(buf + ts_len, "ENGINE * %s ", tag);

  return buf;
}


int result_dump_engine(char *tag, char *fmt, ...)
{
  char msg[1024];
  va_list ap;

  /* Construct message buffer */
  va_start(ap, fmt);
  vsnprintf (msg, sizeof(msg), fmt, ap);
  va_end(ap);

  /* Dump message header */
  result_puts(result_header_engine(tag));

  /* Write event into result file */
  result_puts(msg);
  result_puts("\n");

  return 0;
}
