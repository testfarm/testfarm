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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <wait.h>
#include <signal.h>
#include <glib.h>
#include <sys/poll.h>
#include <sys/socket.h>

#include "useful.h"
#include "debug.h"
#include "tcp.h"
#include "periph_item.h"


/*==============================================================*/
/* SIGCHLD handling                                             */
/*==============================================================*/

typedef struct {
  pid_t pid;
  periph_item_t *item;
} periph_item_sigchld_desc_t;


static void periph_item_sigchld_retrieve(periph_item_t *item, periph_item_sigchld_desc_t *desc)
{
  if ( item->dev.proc.pid == desc->pid )
    desc->item = item;
}


static GList *periph_item_sigchld_list = NULL;


static void periph_item_sigchld(int sig)
{
  pid_t pid;
  int status;
  periph_item_sigchld_desc_t desc;

  /* Acknowledge child death */
  while ( (pid = waitpid(-1, &status, WNOHANG)) > 0 ) {
    if ( debug_flag )
      debug("Signal SIGCHLD received from process %d\n", pid);

    /* Retrieve peripheral item from pid */
    desc.pid = pid;
    desc.item = NULL;
    g_list_foreach(periph_item_sigchld_list, (GFunc) periph_item_sigchld_retrieve, &desc);

    if ( desc.item != NULL )
      desc.item->dev.proc.pid = -1;
  }
}


static void periph_item_sigchld_open(periph_item_t *item)
{

  /* At first open, setup SIGCHLD signal handling */
  if ( periph_item_sigchld_list == NULL ) {
    struct sigaction act;

    sigemptyset(&act.sa_mask);
    act.sa_flags = SA_RESTART;
    act.sa_handler = periph_item_sigchld;
    sigaction(SIGCHLD, &act, NULL);
  }

  /* Add proc to SIGCHLD list */
  periph_item_sigchld_list = g_list_append(periph_item_sigchld_list, item);
}


static void periph_item_sigchld_close(periph_item_t *item)
{
  periph_item_sigchld_list = g_list_remove(periph_item_sigchld_list, item);
}


/*==============================================================*/
/* Peripheral flags                                             */
/*==============================================================*/

typedef struct {
  char *id;
  int value;
} periph_flag_t;


static periph_flag_t periph_flag_tab[] = {
  {"DEFAULTS", 0},
  {"NOHEADER", PERIPH_FLAG_NOHEADER},
  {NULL,       0}
};


static int periph_item_init_flags(periph_item_t *item, char *s_flags)
{
  debug_printf(" flags={");

  while ( (s_flags != NULL) && (*s_flags != NUL) ) {
    char *next;
    periph_flag_t *flag = periph_flag_tab;

    /* Split from next flags */
    if ( (next = strchr(s_flags, ',')) != NULL )
      *(next++) = NUL;

    /* Retrieve current flag */
    while ( flag->id != NULL ) {
      if ( strcmp(flag->id, s_flags) == 0 )
        break;
      flag++;
    }

    if ( flag->id == NULL ) {
      debug_printf("}\n");
      fprintf(stderr, NAME ": Illegal flag '%s' for peripheral '%s'\n", s_flags, item->id);
      return -1;
    }

    item->flags |= flag->value;

    debug_printf("%s%s", s_flags, (next == NULL) ? "":",");

    /* Prepare for next flag */
    s_flags = next;
  }

  debug_printf("}\n");

  return 0;
}


/*==============================================================*/
/* Peripheral initialization                                    */
/*==============================================================*/

/* Table link decriptions */
typedef struct {
  char *id;
  void (*init)(periph_item_t *item, char *dev);
} periph_init_desc_t;

static void periph_item_init_proc(periph_item_t *item, char *dev);
static void periph_item_init_tcp(periph_item_t *item, char *dev);
static void periph_item_init_serial(periph_item_t *item, char *dev);
#ifdef USE_PIPE
static void periph_item_init_pipe(periph_item_t *item, char *dev);
#endif
static void periph_item_init_stdio(periph_item_t *item, char *dev);

static periph_init_desc_t periph_init_tab[] = {
  {"NULL",   NULL},
  {"PROC",   periph_item_init_proc},
  {"TCP",    periph_item_init_tcp},
  {"SERIAL", periph_item_init_serial},
#ifdef USE_PIPE
  {"PIPE",   periph_item_init_pipe},
#endif
  {"STDIO",  periph_item_init_stdio},
  {NULL,     NULL}
};


#define NIBBLE_SIZE 128
#define LF '\n'

#define DEFAULT_TCP_PORT 2000

#define INFO_ID_LEN 20
#define INFO_TYPE_LEN 10
#define INFO_DEV_LEN 80
#define INFO_STATUS_LEN 8
#define INFO_BLABLA_LEN 40


static void periph_item_init_proc(periph_item_t *item, char *dev)
{
  int argc;
  char *s;

  /* Init proc fields */
  item->dev.proc.command = strdup(dev);
  item->dev.proc.argv_buf = strdup(dev);
  item->dev.proc.argv = (char **) malloc(sizeof(char *));
  item->dev.proc.pid = -1;

  /* Build argument list */
  argc = 0;
  s = item->dev.proc.argv_buf;
  do {
    s = strskip_spaces(s);
    if ( *s != NUL ) {
      char *end = strskip_chars(s);

      if ( *end != NUL )
        *(end++) = NUL;

      if ( *s != NUL ) {
        item->dev.proc.argv = (char **) realloc(item->dev.proc.argv, sizeof(char *) * (argc+2));
        item->dev.proc.argv[argc++] = s;
      }

      s = end;
    }
  } while ( *s != NUL );

  item->dev.proc.argv[argc] = NULL;

  debug_printf(" argc=%d", argc);
}


static void periph_item_init_tcp(periph_item_t *item, char *dev)
{
  char *s_port;

  /* Setup TCP port */
  item->dev.tcp.port = DEFAULT_TCP_PORT;
  if ( (s_port = strchr(dev, ':')) != NULL ) {
    *(s_port++) = NUL;
    item->dev.tcp.port = atoi(s_port);
  }

  /* Setup TCP host */
  item->dev.tcp.host = strdup(dev);

  debug_printf(" host=%s port=%d", item->dev.tcp.host, item->dev.tcp.port);
}


static void periph_item_init_serial(periph_item_t *item, char *dev)
{
  item->dev.serial.device = strdup(dev);

  debug_printf(" device=%s", item->dev.serial.device);
}


#ifdef USE_PIPE
static void periph_item_init_pipe(periph_item_t *item, char *dev)
{
  char *s;

  /* Split input/output pipe names */
  if ( (s = strchr(dev, ',')) != NULL )
    *(s++) = NUL;
  else
    s = dev;

  item->dev.pipe.input = strdup(dev);
  item->dev.pipe.output = strdup(s);

  debug_printf(" input=%s output=%s", item->dev.pipe.input, item->dev.pipe.output);
}
#endif


static void periph_item_init_stdio(periph_item_t *item, char *dev)
{
  /* Nothing to do for standard channel */
}


static void periph_item_clear(periph_item_t *item)
{
  switch ( item->type ) {
  case PERIPH_PROC:
    free(item->dev.proc.command);
    free(item->dev.proc.argv_buf);
    free(item->dev.proc.argv);
    break;

  case PERIPH_TCP:
    free(item->dev.tcp.host);
    break;

  case PERIPH_SERIAL:
    free(item->dev.serial.device);
    break;

#ifdef USE_PIPE
  case PERIPH_PIPE:
    free(item->dev.pipe.output);
    free(item->dev.pipe.input);
    break;
#endif

  case PERIPH_STDIO:
    /* Nothing to do for standard i/o channels */
    break;

  default :
    break;
  }

  free(item->id);
  free(item->blabla);
}


periph_item_t *periph_item_init(char *argv[], periph_item_t *item)
{
  periph_item_type_t type;
  char *s_id, *s_type, *s_dev, *s_flags, *s_blabla;
  periph_init_desc_t *desc;
  char *s;
  int i;

  /* Clear item content if exists */
  if ( item != NULL )
    periph_item_clear(item);

  /* Set default arguments */
  s_type = "PROC";
  s_dev = NULL;
  s_flags = NULL;
  s_blabla = NULL;

  /* Parse arguments */
  if ( argv != NULL ) {
    /* Get interface name */
    s_id = strupper(argv[0]);

    /* Get interface address */
    s_dev = argv[1];
    s = strstr(s_dev, "://");
    if ( s != NULL ) {
      *s = '\0';
      s_type = strupper(s_dev);
      s_dev = s+3;
    }

    /* Get other interface fields */
    for (i = 2; argv[i] != NULL; i++) {
      s = argv[i];

      if ( s[0] == '-' ) {
	s_flags = strupper(&(s[1]));
      }
      else {
	if ( s_blabla == NULL )
	  s_blabla = s;
      }
    }

    /* Setup link type */
    type = PERIPH_NULL;
    desc = periph_init_tab;
    for (i = 0; (i < PERIPH_N) && (type == PERIPH_NULL); i++) {
      desc = &(periph_init_tab[i]);
      if ( strcmp(s_type, desc->id) == 0 ) {
        type = i;
      }
    }

    /* Check link type */
    if ( (type >= PERIPH_N) || (type == PERIPH_NULL) ) {
      fprintf(stderr, NAME ": Illegal link type '%s' for interface '%s'\n", s_type, s_id);
      return NULL;
    }

    /* Check link address */
    if ( s_dev == NULL ) {
      fprintf(stderr, NAME ": Missing link adress for interface '%s'\n", s_id);
      return NULL;
    }
  }
  else {
    s_id = "STDIO";
    s_dev = "<stdio>";
    s_blabla = "Standard input/output";
    type = PERIPH_STDIO;
    desc = &(periph_init_tab[PERIPH_STDIO]);
  }

  /* Allocate new item with null settings */
  if ( item == NULL ) {
    item = (periph_item_t *) malloc(sizeof(periph_item_t));
    item->buf = (char *) malloc(NIBBLE_SIZE);
    item->size = NIBBLE_SIZE;
    item->closed = NULL;
    item->closed_arg = NULL;
    item->next = NULL;
  }

  item->id = strupper(strdup(s_id));
  item->type = type;
  item->flags = 0;
  item->blabla = (s_blabla != NULL) ? strdup(s_blabla) : NULL;
  if ( type == PERIPH_STDIO ) {
    item->fd_in = STDIN_FILENO;
    item->fd_out = STDOUT_FILENO;
  }
  else {
    item->fd_in = item->fd_out = -1;
  }
  item->index = 0;

  /* Init link descriptor */
  debug("Init peripheral: id=%s type=%s dev=\"%s\"", item->id, desc->id, s_dev);
  desc->init(item, s_dev);

  /* Setup peripheral flags */
  if ( periph_item_init_flags(item, s_flags)) {
    periph_item_done(item);
    return NULL;
  }

  return item;
}


/*==============================================================*/
/*==============================================================*/

void periph_item_closed_set(periph_item_t *item, periph_item_closed_t *closed, void *closed_arg)
{
  item->closed = closed;
  item->closed_arg = closed_arg;
}


void periph_item_closed_raise(periph_item_t *item)
{
  if ( item == NULL )
    return;

  if ( item->closed != NULL )
    item->closed(item, item->closed_arg);
}


static void periph_item_open_proc(periph_item_t *item)
{
  int p_in[2], p_out[2];
  pid_t pid;

  /* Create communication pipes */
  item->fd_in = item->fd_out = -1;
  if ( pipe(p_in) == -1 ) {
    return;
  }
  if ( pipe(p_out) == -1 ) {
    close(p_in[0]);
    close(p_in[1]);
    return;
  }

  /* Enable close-on-exec mode on local pipe endpoints */
  fcntl(p_in[1], F_SETFD, FD_CLOEXEC);
  fcntl(p_out[0], F_SETFD, FD_CLOEXEC);

  /* Spawn child process */
  switch ( (pid = fork()) ) {
  case 0: /* Child */
    /* Redirect inputs */
    dup2(p_in[0], STDIN_FILENO);
    close(p_in[0]);

    /* Redirect outputs */
    dup2(p_out[1], STDOUT_FILENO);
    close(p_out[1]);

    /* Perform exec (returnless call if success) */
    execvp(item->dev.proc.argv[0], item->dev.proc.argv);

    /* Return from exec: something went wrong */
    fprintf(stderr, NAME ": execvp '%s': %s\n", item->dev.proc.argv[0], strerror(errno));
    break;

  case -1: /* Error */
    perror(NAME ": fork");

    close(p_in[0]);
    close(p_in[1]);
    close(p_out[0]);
    close(p_out[1]);
    break;

  default : /* Parent */
    item->dev.proc.pid = pid;
    debug_printf(" PID=%d", pid);

    close(p_out[1]);
    item->fd_in = p_out[0];
    debug_printf(" INPUT=%d", item->fd_in);

    close(p_in[0]);
    item->fd_out = p_in[1];
    debug_printf(" OUTPUT=%d", item->fd_out);

    periph_item_sigchld_open(item);
    break;
  }
}


static int periph_item_open_serial(periph_item_t *item)
{
  char *dev = item->dev.serial.device;
  int fd;
  struct termios termio;

  /* Open serial link device */
  if ( (fd = open(dev, O_RDWR)) == -1 ) {
    fprintf(stderr, NAME ": Cannot open serial link %s\n", dev);
    return -1;
  }

  /* Check it is actually a serial link device */
  if ( ! isatty(fd) ) {
    fprintf(stderr, NAME ": %s is not a serial link\n", dev);
    close(fd);
    return -1;
  }

  /* Get previous device settings */
  if ( tcgetattr(fd, &termio) ) {
    fprintf(stderr, NAME ": Cannot get attributes for serial device %s\n", dev);
    close(fd);
    return -1;
  }
  item->dev.serial.termio_save = termio;

  /* Ecriture des nouveaux parametres: bps, 8N1 */
  /*termio.c_cflag = "Inherited from previous configuration using stty" */
  termio.c_oflag = 0;                /* Mode raw */
  termio.c_iflag = 0;
  termio.c_lflag = 0;                /* Mode non canonique */
  termio.c_line = 0;                 /* Discipline 0 */
  termio.c_cc[VMIN] = 1;
  termio.c_cc[VTIME] = 0;

  if ( tcsetattr(fd, TCSANOW, &termio) ) {
    fprintf(stderr, NAME ": Cannot get attributes for serial device %s\n", dev);
    close(fd);
    return -1;
  }

  /* Purger les buffers */
  tcflush(fd, TCIOFLUSH);

  return fd;
}


int periph_item_open(periph_item_t *item)
{
  if ( item->fd_in != -1 )
    return item->fd_in;

  switch ( item->type ) {
  case PERIPH_PROC:
    debug("Spawning '%s' to PROC \"%s\":", item->id, item->dev.proc.command);

    periph_item_open_proc(item);

    debug_printf("\n");
    break;

  case PERIPH_TCP:
    debug("Connecting '%s' to TCP/IP %s:%d\n", item->id, item->dev.tcp.host, item->dev.tcp.port);

    item->fd_in = item->fd_out = tcpconnect(item->dev.tcp.host, item->dev.tcp.port, debug_flag);
    break;

  case PERIPH_SERIAL:
    debug("Openning '%s' serial link %s\n", item->id, item->dev.serial.device);

    item->fd_in = item->fd_out = periph_item_open_serial(item);
    break;

#ifdef USE_PIPE
  case PERIPH_PIPE:
    debug("Openning '%s' pipes: in=%s out=%s\n", item->id, item->dev.pipe.input, item->dev.pipe.output);

    if ( (item->fd_in = open(item->dev.pipe.input, O_RDONLY)) == -1 ) {
      fprintf(stderr, NAME ": Cannot open input pipe %s\n", item->dev.pipe.input);
    }

    debug("Pipe: in=%s open\n", item->dev.pipe.input);

    if ( (item->fd_out = open(item->dev.pipe.output, O_WRONLY)) == -1 ) {
      fprintf(stderr, NAME ": Cannot open output pipe %s\n", item->dev.pipe.output);
      close(item->fd_in);
      item->fd_in = -1;
      break;
    }

    debug("Pipe: out=%s open\n", item->dev.pipe.output);
    break;
#endif

  case PERIPH_STDIO:
    /* Nothing to do for standard i/o channels */
    break;

  default :
    debug("Cannot open illegal link type (%d)\n", item->type);
    break;
  }

  /* Error ? */
  if ( item->fd_in == -1 )
    return -1;

  /* Enable non-blocking input operations */
  fcntl(item->fd_in, F_SETFL, O_NONBLOCK | fcntl(item->fd_in, F_GETFL, 0));

  /* Clear read buffering index */
  item->index = 0;

  debug("Link to peripheral %s is open\n", item->id);

  return item->fd_in;
}


void periph_item_hangup(periph_item_t *item)
{
  if ( item->type == PERIPH_PROC ) {
    if ( item->fd_out >= 0 )
      close(item->fd_out);
    item->fd_out = -1;
  }
}


void periph_item_close(periph_item_t *item)
{
  /* Already closed => nothing to do */
  if ( item->fd_in == -1 )
    return;

  debug("Peripheral '%s': ", item->id);

  switch ( item->type ) {
  case PERIPH_PROC:
    /* Cancel process from SIGCHLD list */
    periph_item_sigchld_close(item);

    /* Close output pipe (child stdin) */
    periph_item_hangup(item);

    /* Flush & Close input pipe (child stdout) */
    close(item->fd_in);
    item->fd_in = -1;

    /* Kill process (if it did not by itself) */
    if ( item->dev.proc.pid > 0 ) {
      pid_t pid = item->dev.proc.pid;
      item->dev.proc.pid = -1;
      kill(pid, SIGTERM);
    }
    break;

  case PERIPH_TCP:
    /* Shut TCP connection down */
    shutdown(item->fd_in, 2);
    close(item->fd_in);
    item->fd_in = item->fd_out = -1;

    debug_printf("TCP connection to %s:%d shut down\n", item->dev.tcp.host, item->dev.tcp.port);
    break;

  case PERIPH_SERIAL:
    /* Restore original device settings */
    tcsetattr(item->fd_in, TCSANOW, &(item->dev.serial.termio_save));

    /* Close serial link */
    close(item->fd_in);
    item->fd_in = item->fd_out = -1;

    debug_printf("serial line %s closed\n", item->dev.serial.device);
    break;

#ifdef USE_PIPE
  case PERIPH_PIPE:
    /* Close input & output pipes */
    close(item->fd_in);
    close(item->fd_out);
    item->fd_in = item->fd_out = -1;

    debug_printf("pipes in=%s out=%s closed\n", item->dev.pipe.input, item->dev.pipe.output);
    break;
#endif

  case PERIPH_STDIO:
    /* Nothing to do for standard i/o channels */
    break;

  default :
    debug_printf("cannot close illegal link type (%d)\n", item->type);
    break;
  }
}


void periph_item_done(periph_item_t *item)
{
  if ( item == NULL )
    return;

  periph_item_close(item);
  periph_item_clear(item);

  free(item->buf);

  free(item);
}


int periph_item_connected(periph_item_t *item)
{
  return ( (item->fd_in >= 0) && (item->fd_out >= 0) );
}


int periph_item_read(periph_item_t *item)
{
  int count = 0;
  int ret;
  char c;

  while ( (ret = read(item->fd_in, &c, 1)) > 0 ) {
    /* Grow i/o buffer by one nibble if necessary */
    if ( item->index >= (item->size-1) ) {
      item->size += NIBBLE_SIZE;
      item->buf = realloc(item->buf, item->size);

      if ( debug_flag ) { /* For speedup in non-debug mode */
        debug("%s input buffer grown to %d chars\n", item->id, item->size);
      }
    }

    /* Convert non-displayable data into space */
    if ( (c != LF) && (c < ' ') )
      c = ' ';

    /* Put data into i/o buffer */
    item->buf[(item->index)++] = c;
    count++;

    /* If end of line encountered, transfer data to target buffer... */
    if ( c == LF ) {
      int total_size = item->index;

      /* Terminate buffer with a NUL character */
      item->buf[total_size++] = NUL;

      /* Clear i/o buffer indexing */
      item->index = 0;

      if ( debug_flag ) { /* For speedup in non-debug mode */
        debug("RECEIVED FROM '%s': %s", item->id, item->buf);
      }
      return total_size;
    }
  }

  /* Check error report */
  if ( (ret == -1) && (errno != EAGAIN) ) {
    debug_errno("read() error on peripheral '%s'", item->id);
    return -1;
  }

  /* If an empty read occured, that means the peripheral we were
     talking with has closed the connection */
  if ( count == 0 ) {
    debug("Peripheral '%s' disconnected by peer (fd=%d)\n", item->id, item->fd_in);
    periph_item_closed_raise(item);
  }

  return 0;
}


int periph_item_write(periph_item_t *item, char *buf, int size)
{
  if ( item->fd_out == -1 ) {
    fprintf(stderr, "Peripheral '%s' not open for write\n", item->id);
    return -1;
  }

  if ( debug_flag ) { /* For speedup in non-debug mode */
    char *msg = (char *) malloc(size + 1);
    memcpy(msg, buf, size);
    msg[size] = NUL;
    debug("SENDING TO '%s': %s", item->id, msg);
    free(msg);
  }

  return write(item->fd_out, buf, size);
}


int periph_item_cleanup(periph_item_t *item)
{
  if ( item == NULL )
    return 0;

  if ( (item->type == PERIPH_PROC) && (item->dev.proc.pid == -1) ) {
    struct pollfd ufds = {item->fd_in, POLLIN};

    if ( poll(&ufds, 1, 0) == 0 ) {
      periph_item_closed_raise(item);
      return 1;
    }
  }

  return 0;
}


char *periph_item_info(periph_item_t *item)
{
  static char buf[INFO_ID_LEN+INFO_TYPE_LEN+INFO_DEV_LEN+INFO_STATUS_LEN+INFO_BLABLA_LEN+2];
  int len = 0;

  /* Peripheral id */
  strncpy(buf+len, item->id, INFO_ID_LEN-1);
  len += strlen(buf+len);
  buf[len++] = ' ';

  /* Link type */
  strncpy(buf+len, periph_init_tab[item->type].id, INFO_TYPE_LEN-1);
  len += strlen(periph_init_tab[item->type].id);
  buf[len++] = ' ';  

  /* Link device */
  switch ( item->type ) {
  case PERIPH_PROC:
    len += snprintf(buf+len, INFO_DEV_LEN, "\"%s\"", item->dev.proc.command);
    break;

  case PERIPH_TCP:
    strncpy(buf+len, item->dev.tcp.host, INFO_DEV_LEN-10);
    len += strlen(item->dev.tcp.host);
    len += sprintf(buf+len, ":%d", item->dev.tcp.port);
    break;

  case PERIPH_SERIAL:
    strncpy(buf+len, item->dev.serial.device, INFO_DEV_LEN);
    len += strlen(item->dev.serial.device);
    break;

#ifdef USE_PIPE
  case PERIPH_PIPE:
    strncpy(buf+len, item->dev.pipe.input, INFO_DEV_LEN-2);
    len += strlen(item->dev.pipe.input);
    len += sprintf(buf+len, "< ");

    strncpy(buf+len, item->dev.pipe.output, INFO_DEV_LEN-1);
    len += strlen(item->dev.pipe.output);
    len += sprintf(buf+len, ">");
    break;
#endif

  case PERIPH_STDIO:
    strncpy(buf+len, "<stdio>", INFO_DEV_LEN);
    len += 7;
    break;

  default :
    strcpy(buf+len, "?");
    len++;
    break;
  }

  /* Connect status */
  if ( item->fd_in != -1 )
    len += sprintf(buf+len, " open   ");
  else
    len += sprintf(buf+len, " closed ");

  /* Link blabla description */
  if ( item->blabla != NULL ) {
    buf[len++] = '"';
    strncpy(buf+len, item->blabla, INFO_BLABLA_LEN-2);
    len += strlen(item->blabla);
    buf[len++] = '"';
  }

  return buf;
}
