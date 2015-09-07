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
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <malloc.h>
#include <unistd.h>
#include <glib.h>

#include "child.h"
#include "capture_interface.h"
#include "capture_proc.h"


/* Capture process identification */
extern char proc_name[];

#define eprintf(args...) { fprintf(stderr, "%s: ", proc_name); fprintf(stderr, args); }
#define _dprintf(args...) fprintf(stderr, args)
#define dprintf(args...) { _dprintf("[DEBUG INTERFACE %s]: ", proc_name); _dprintf(args); }

#define CAPTURE_PROC(_ptr_) ((capture_proc_t *)(_ptr_))

typedef struct {
  capture_t h;
  capture_opt_t opt;
  child_t *child;
  int stdin_fd;
  int callback_fd;
  int update_fd;
  GIOChannel *update_channel;
  guint update_tag;
  capture_update_fn *update_fn;
  void *data;
} capture_proc_t;


static void proc_terminate(int status, capture_proc_t *capture)
{
  if ( status ) {
    eprintf("process terminated with status %d\n", status);
  }

  if ( capture->update_tag > 0 ) {
    g_source_remove(capture->update_tag);
    capture->update_tag = 0;
  }

  if ( capture->update_channel != NULL ) {
    g_io_channel_unref(capture->update_channel);
    capture->update_channel = NULL;
  }

  if ( capture->stdin_fd > 0 ) {
    close(capture->stdin_fd);
    capture->stdin_fd = -1;
  }

  if ( capture->update_fd > 0 ) {
    close(capture->update_fd);
    capture->update_fd = -1;
  }

  if ( capture->callback_fd > 0 ) {
    close(capture->callback_fd);
    capture->callback_fd = -1;
  }

  if ( capture->child != NULL ) {
    child_handler(capture->child, NULL, NULL);
    child_terminate(capture->child);
    capture->child = NULL;
  }

  capture->update_fn = NULL;
}


static gboolean proc_update(GIOChannel *source, GIOCondition condition, capture_proc_t *capture)
{
  /* Check for remote disconnection */
  if ( condition & G_IO_HUP ) {
    proc_terminate(0, capture);
    return FALSE;
  }

  /* Read from update events pipe */
  if ( condition & G_IO_IN ) {
    frame_geometry_t g = FRAME_GEOMETRY_NULL;
    int ret;

    ret = read(capture->update_fd, &g, sizeof(g));
    if ( ret == sizeof(g) ) {
      if ( capture->opt & CAPTURE_OPT_DEBUG ) {
	dprintf("UPDATE %s\n", frame_geometry_str(&g));
      }

      if ( capture->update_fn )
	capture->update_fn(&g, capture->data);
    }
    else if ( ret < 0 ) {
      eprintf("read(update): %s\n", strerror(errno));
    }
    else {
      eprintf("read(update): truncated message\n");
    }
  }

  return TRUE;
}


static void *proc_callback(capture_proc_t *capture, capture_proc_cb_type_t type)
{
  capture_proc_cb_hdr_t hdr;
  unsigned char *buf;
  int size;
  unsigned char *ptr;
  int ret;

  /* Read message header */
  size = sizeof(hdr);
  ret = read(capture->callback_fd, &hdr, size);

  if ( ret < 0 ) {
    eprintf("read(callback-hdr): %s\n", strerror(errno));
    return NULL;
  }

  if ( ret == 0 ) {
    return NULL;
  }

  if ( ret != size ) {
    eprintf("read(callback-hdr): truncated message (%d/%d)\n", ret, size);
    return NULL;
  }

  if ( capture->opt & CAPTURE_OPT_DEBUG ) {
    const char *type_str[] = {"OPEN", "WINDOW", "PERIOD", "ATTR"};
    dprintf("CALLBACK type=%s(%d) len=%d\n", type_str[hdr.type], hdr.type, hdr.len);
  }

  /* Check message type */
  if ( hdr.type != type ) {
    eprintf("read(callback-hdr): unexpected message type (%d/%d)\n", hdr.type, type);
    return NULL;
  }

  /* Allocate message content buffer */
  buf = malloc(hdr.len);
  ((capture_proc_cb_hdr_t *) buf)->type = hdr.type;
  ((capture_proc_cb_hdr_t *) buf)->len = hdr.len;
  size = hdr.len - sizeof(hdr);
  ptr = buf + sizeof(hdr);

  /* Read message content */
  ret = read(capture->callback_fd, ptr, size);

  if ( ret < 0 ) {
    eprintf("read(callback-buf): %s\n", strerror(errno));
    goto failed;
  }

  if ( ret != size ) {
    eprintf("read(callback-buf): truncated message (%d/%d)\n", ret, size);
    goto failed;
  }

  return buf;

 failed:
  free(buf);
  return NULL;
}


static void proc_request(capture_proc_t *capture, char *fmt, ...)
{
  char str[1024];
  int len;
  va_list ap;

  va_start(ap, fmt);
  len = vsnprintf(str, sizeof(str), fmt, ap);
  va_end(ap);

  if ( capture->stdin_fd > 0 )
    write(capture->stdin_fd, str, len);
}


static capture_t * capture_proc_open(char *device, capture_opt_t opt, capture_update_fn *update, void *data)
{
  capture_proc_t *capture;
  int stdin_p[2];
  int callback_p[2];
  int update_p[2];
  char opt_callback[10];
  char opt_update[10];
  char *argv[] = {
    proc_name,
    device,
    "-callback", opt_callback,
    "-update", opt_update,
    NULL, NULL, NULL,
    NULL
  };
  int argc = 6;
  capture_proc_cb_open_t *cb;

  /* Create stdin pipe */
  if ( pipe(stdin_p) == -1 ) {
    eprintf("Cannot create stdin pipe: %s\n", strerror(errno));
    return NULL;
  }

  fcntl(stdin_p[1], F_SETFD, FD_CLOEXEC);

  /* Create callback pipe */
  if ( pipe(callback_p) == -1 ) {
    eprintf("Cannot create callback pipe: %s\n", strerror(errno));
    return NULL;
  }

  fcntl(callback_p[0], F_SETFD, FD_CLOEXEC);

  /* Create update pipe */
  if ( pipe(update_p) == -1 ) {
    eprintf("Cannot create update pipe: %s\n", strerror(errno));
    return NULL;
  }

  fcntl(update_p[0], F_SETFD, FD_CLOEXEC);
  fcntl(update_p[0], F_SETFL, O_NONBLOCK);

  /* Setup capture subprocess arguments */
  snprintf(opt_callback, sizeof(opt_callback), "%d", callback_p[1]);
  snprintf(opt_update, sizeof(opt_update), "%d", update_p[1]);
  if ( opt & CAPTURE_OPT_SHARED )
    argv[argc++] = "-shared";
  if ( opt & CAPTURE_OPT_DEBUG )
    argv[argc++] = "-debug";
  if ( opt & CAPTURE_OPT_ROTATE ) {
    if ( opt & CAPTURE_OPT_ROTATE_CCW )
      argv[argc++] = "-rotate-ccw";
    else
      argv[argc++] = "-rotate-cw";
  }

  if ( opt & CAPTURE_OPT_DEBUG ) {
    char **argp = argv;

    dprintf("OPEN argc=%d, argv[]={ ", argc);
    while ( *argp ) {
      if ( argp != argv )
	_dprintf(", ");
      _dprintf("\"%s\"", *argp);
      argp++;
    }
    _dprintf(" }\n");
  }

  /* Allocate capture interface descriptor */
  capture = malloc(sizeof(capture_proc_t));
  capture->opt = opt;

  /* Spawn capture subprocess */
  capture->child = child_spawn(argv, stdin_p[0], -1, -1,
			     (child_handler_t *) proc_terminate, capture);

  /* Close useless file descriptors (only used by agent subprocess) */
  close(stdin_p[0]);
  close(callback_p[1]);
  close(update_p[1]);

  if ( capture->child == NULL )
    goto failed;

  /* Setup io endpoints */
  capture->stdin_fd = stdin_p[1];
  capture->callback_fd = callback_p[0];
  capture->update_fd = update_p[0];
  capture->update_channel = g_io_channel_unix_new(capture->update_fd);
  capture->update_tag = g_io_add_watch(capture->update_channel, G_IO_IN | G_IO_HUP,
				       (GIOFunc) proc_update, capture);

  if ( opt & CAPTURE_OPT_DEBUG ) {
    dprintf("OPEN stdin(w)=%d callback(r)=%d update(r)=%d\n",
	    capture->stdin_fd, capture->callback_fd, capture->update_fd);
  }

  /* Set callback handler */
  capture->update_fn = update;
  capture->data = data;

  /* Wait for open callback */
  cb = proc_callback(capture, CAPTURE_PROC_CB_OPEN);
  if ( cb == NULL )
    goto failed;

  capture->h.shmid = cb->shmid;
  capture->h.cap = cb->cap;
  capture->h.name = strdup(cb->name);

  free(cb);

  if ( opt & CAPTURE_OPT_DEBUG ) {
    dprintf("OPEN shmid=%d cap=%d name=\"%s\"\n",
	    capture->h.shmid, capture->h.cap, capture->h.name);
  }

  return (capture_t *) capture;

 failed:
  close(stdin_p[1]);
  close(callback_p[0]);
  close(update_p[0]);
  free(capture);
  return NULL;
}


static void capture_proc_close(capture_t *capture)
{
  proc_terminate(0, CAPTURE_PROC(capture));
  free(capture);
}


static int capture_proc_set_window(capture_t *capture, frame_geometry_t *g)
{
  proc_request(CAPTURE_PROC(capture), "window %s\n", frame_geometry_str(g));
  return 0;
}


static int capture_proc_get_window(capture_t *capture, frame_geometry_t *g)
{
  capture_proc_cb_window_t *cb;

  proc_request(CAPTURE_PROC(capture), "window\n");

  cb = proc_callback(CAPTURE_PROC(capture), CAPTURE_PROC_CB_WINDOW);
  if ( cb == NULL )
    return -1;

  *g = cb->g;

  free(cb);

  if ( CAPTURE_PROC(capture)->opt & CAPTURE_OPT_DEBUG ) {
    dprintf("WINDOW g=%s\n", frame_geometry_str(g));
  }

  return 0;
}


static long capture_proc_set_period(capture_t *capture, long delay)
{
  capture_proc_cb_period_t *cb;

  proc_request(CAPTURE_PROC(capture), "period %ld\n", delay);

  cb = proc_callback(CAPTURE_PROC(capture), CAPTURE_PROC_CB_PERIOD);
  if ( cb == NULL )
    return -1;

  delay = cb->delay;

  free(cb);

  if ( CAPTURE_PROC(capture)->opt & CAPTURE_OPT_DEBUG ) {
    dprintf("PERIOD(set) delay=%ld\n", delay);
  }

  return delay;
}


static long capture_proc_get_period(capture_t *capture)
{
  capture_proc_cb_period_t *cb;
  long delay;

  proc_request(CAPTURE_PROC(capture), "period\n");

  cb = proc_callback(CAPTURE_PROC(capture), CAPTURE_PROC_CB_PERIOD);
  if ( cb == NULL )
    return -1;

  delay = cb->delay;

  free(cb);

  if ( CAPTURE_PROC(capture)->opt & CAPTURE_OPT_DEBUG ) {
    dprintf("PERIOD(get) delay=%ld\n", delay);
  }

  return delay;
}


static int capture_proc_refresh(capture_t *capture)
{
  proc_request(CAPTURE_PROC(capture), "refresh\n");
  return 0;
}


static void capture_proc_action_key(capture_t *capture, int down, unsigned long key)
{
  proc_request(CAPTURE_PROC(capture), "action_key %d %lu\n", down, key);
}


static void capture_proc_action_pointer(capture_t *capture, unsigned char buttons, unsigned int x, unsigned int y)
{
  proc_request(CAPTURE_PROC(capture), "action_pointer %u %u %u\n", buttons, x, y);
}


static void capture_proc_action_scroll(capture_t *capture, unsigned char direction)
{
  proc_request(CAPTURE_PROC(capture), "action_scroll %u\n", direction);
}


static int capture_proc_attr_set(capture_t *capture, char *key, char *value)
{
  proc_request(CAPTURE_PROC(capture), "attr %s %s\n", key, value);
  return 0;
}


static capture_attr_t * capture_proc_attr_get(capture_t *capture, char *key, int *nmemb)
{
  static capture_attr_t *attrs = NULL;
  static int attrs_n = 0;
  capture_proc_cb_attr_t *cb;
  int len;
  char *str;
  int count;

  if ( key )
    proc_request(CAPTURE_PROC(capture), "attr %s\n", key);
  else
    proc_request(CAPTURE_PROC(capture), "attr\n");

  cb = proc_callback(CAPTURE_PROC(capture), CAPTURE_PROC_CB_ATTR);
  if ( cb == NULL )
    return NULL;

  /* Cleanup attributes buffer */
  for ( count = 0; count < attrs_n; count++) {
    capture_attr_t *attr = &attrs[count];
    if ( attr->key != NULL ) {
      free(attr->key);
      attr->key = NULL;
    }
    if ( attr->value != NULL ) {
      free(attr->value);
      attr->value = NULL;
    }
  }

  /* Parse message content and feed attributes table */
  len = cb->hdr.len - sizeof(capture_proc_cb_hdr_t);
  str = cb->key_value;
  count = 0;

  while ( (*str != '\0') && (len > 0) ) {
    char *value = str;

    /* Split key from value strings */
    while ( (*value != '\0') && (*value != '=') )
      value++;
    if ( *value == '=' )
      *(value++) = '\0';

    /* Realloc attributes buffer if needed */
    if ( attrs_n <= count ) {
      attrs_n = count + 1;
      attrs = realloc(attrs, sizeof(capture_attr_t) * attrs_n);
    }

    /* Add key and value to attributes buffer */
    attrs[count].key = strdup(str);
    attrs[count].value = strdup(value);
    count++;

    /* Move string pointer to next item */
    while ( *value != '\0' )
      value++;
    value++;

    len -= (value - str);
    str = value;
  }

  free(cb);

  if ( nmemb != NULL )
    *nmemb = count;

  if ( CAPTURE_PROC(capture)->opt & CAPTURE_OPT_DEBUG ) {
    int i;

    dprintf("ATTR(get) nmemb=%d", count);
    for (i = 0; i < count; i++) {
      capture_attr_t *attr = &attrs[i];
      _dprintf(" %s=\"%s\"", attr->key, attr->value);
    }
    _dprintf("\n");
  }

  return attrs;
}


static void capture_proc_show_status(capture_t *capture, char *hdr)
{
  proc_request(CAPTURE_PROC(capture), "show_status %s\n", hdr);
}


capture_interface_t _capture_interface = {
  open :           capture_proc_open,
  close :          capture_proc_close,
  set_window :     capture_proc_set_window,
  get_window :     capture_proc_get_window,
  set_period :     capture_proc_set_period,
  get_period :     capture_proc_get_period,

  refresh :        capture_proc_refresh,

  action_key :     capture_proc_action_key,
  action_pointer : capture_proc_action_pointer,
  action_scroll :  capture_proc_action_scroll,

  attr_set:        capture_proc_attr_set,
  attr_get:        capture_proc_attr_get,

  show_status :    capture_proc_show_status,
};


#if 0
int capture_proc_init(char *method)
{
}
#endif
