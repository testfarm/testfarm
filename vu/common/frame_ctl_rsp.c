/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* Display control (client->display messages)                               */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 24-APR-2007                                                    */
/****************************************************************************/

/*
 * $Revision: 1183 $
 * $Date: 2010-07-28 22:09:53 +0200 (mer., 28 juil. 2010) $
 */

#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <unistd.h>
#include <fcntl.h>
#include <limits.h>

#include "frame_ctl.h"
#include "frame_ctl_msg.h"


int frame_ctl_init(int sock, int shmid, capture_cap_t cap)
{
  char cwd[PATH_MAX];
  int len = sizeof(frame_ctl_rsp_init);
  frame_ctl_rsp_init *init;
  int ret;

  if ( getcwd(cwd, sizeof(cwd)) != NULL ) {
    len += strlen(cwd);
  }
  else {
    cwd[0] = '\0';
  }

  init = (frame_ctl_rsp_init *) malloc(len);
  memset(init, 0, len);

  init->hdr.ctl = FRAME_CTL_INIT;
  init->hdr.len = len;
  init->shmid = shmid;
  init->cap = cap;
  strcpy(init->cwd, cwd);

  ret = write(sock, init, len);

  free(init);

  return ret;
}


int frame_ctl_window(int sock, frame_geometry_t *g)
{
  frame_ctl_rsp_window window = {
    hdr: {
      ctl: FRAME_CTL_WINDOW,
      len: sizeof(frame_ctl_rsp_window)
    },
    g: *g
  };

  return write(sock, &window, window.hdr.len);
}


int frame_ctl_refresh(int sock, int shmid, frame_geometry_t *g)
{
  frame_ctl_rsp_refresh refresh = {
    hdr: {
      ctl: FRAME_CTL_REFRESH,
      len: sizeof(frame_ctl_rsp_refresh)
    },
    shmid: shmid,
    g: *g
  };

  return write(sock, &refresh, sizeof(frame_ctl_rsp_refresh));
}


int frame_ctl_pattern(int sock, pattern_t *pattern)
{
  int idlen = strlen(pattern->id);
  int len;
  frame_ctl_rsp_pattern *msg;
  int ret;

  /* Compute message length */
  len = sizeof(frame_ctl_rsp_pattern) + idlen + 2;
  if ( pattern->source != NULL )
    len += strlen(pattern->source);

  /* Alloc message buffer */
  msg = (frame_ctl_rsp_pattern *) malloc(len);
  memset(msg, 0, len);

  msg->hdr.ctl = FRAME_CTL_PATTERN;
  msg->hdr.len = len;

  msg->shmid = pattern->frame->shmid;
  msg->g = pattern->window;
  msg->mode = pattern->mode;
  msg->type = pattern->type;

  strcpy(msg->id_source, pattern->id);
  if ( pattern->source != NULL )
    strcpy(msg->id_source + idlen + 1, pattern->source);

  switch (pattern->type) {
  case PATTERN_TYPE_IMAGE:
	  msg->fuzz = pattern->d.image.fuzz;
	  msg->loss[0] = pattern->d.image.badpixels_rate;
	  msg->loss[1] = pattern->d.image.potential_rate;
	  break;
  case PATTERN_TYPE_TEXT:
	  break;
  default:
	  break;
  }

  ret = write(sock, msg, len);

  free(msg);

  return ret;
}


int frame_ctl_pattern_remove(int sock, char *id)
{
  int idlen = strlen(id);
  int len;
  frame_ctl_rsp_pattern *msg;
  int ret;

  /* Compute message length */
  len = sizeof(frame_ctl_rsp_pattern) + idlen + 2;

  /* Alloc message buffer */
  msg = (frame_ctl_rsp_pattern *) malloc(len);
  memset(msg, 0, len);

  msg->hdr.ctl = FRAME_CTL_PATTERN;
  msg->hdr.len = len;

  msg->shmid = -1;
  strcpy(msg->id_source, id);

  ret = write(sock, msg, len);

  free(msg);

  return ret;
}


int frame_ctl_match(int sock, char *id, int state)
{
  int len = sizeof(frame_ctl_rsp_match) + strlen(id) + 1;
  char buf[len];
  frame_ctl_rsp_match *msg = (frame_ctl_rsp_match *) buf;

  memset(buf, 0, len);

  msg->hdr.ctl = FRAME_CTL_MATCH;
  msg->hdr.len = len;
  msg->state = state;
  strcpy(msg->id, id);

  return write(sock, buf, len);
}


/* Both CMD and RSP message */
int frame_ctl_period(int sock, unsigned int period)
{
  int len = sizeof(frame_ctl_cmd_period);
  char buf[len];
  frame_ctl_cmd_period *msg = (frame_ctl_cmd_period *) buf;

  memset(buf, 0, len);

  msg->hdr.ctl = FRAME_CTL_PERIOD;
  msg->hdr.len = len;
  msg->period = period;

  return write(sock, buf, len);
}


/* Both CMD and RSP message */
int frame_ctl_source(int sock, char *args)
{
  int len = sizeof(frame_ctl_cmd_source) + strlen(args) + 1;
  char buf[len];
  frame_ctl_cmd_source *msg = (frame_ctl_cmd_source *) buf;

  memset(buf, 0, len);

  msg->hdr.ctl = FRAME_CTL_SOURCE;
  msg->hdr.len = len;
  strcpy(msg->args, args);

  return write(sock, buf, len);
}


#define FRAME_CTL_PAD_MAX 10

int frame_ctl_pad(int sock, frame_geometry_t *gtab, int nmemb)
{
  char *buf = NULL;
  int ret = 0;

  while ( (nmemb > 0) && (ret >= 0) ) {
    int len;
    frame_ctl_rsp_pad *msg;
    int i;

    int nmemb2 = nmemb;
    if ( nmemb2 > FRAME_CTL_PAD_MAX )
      nmemb2 = FRAME_CTL_PAD_MAX;
    nmemb -= nmemb2;

    len = sizeof(frame_ctl_rsp_pad) + ((nmemb2-1) * sizeof(frame_geometry_t));

    if ( buf == NULL )
      buf = malloc(len);
    memset(buf, 0, len);

    msg = (frame_ctl_rsp_pad *) buf;
    msg->hdr.ctl = FRAME_CTL_PAD;
    msg->hdr.len = len;
    msg->nmemb = nmemb2;

    for (i = 0; i < nmemb2; i++)
      msg->gtab[i] = *(gtab++);

    ret = write(sock, buf, len);
  }

  if ( buf != NULL )
    free(buf);

  return ret;
}


int frame_ctl_frame(int sock, int parent_shmid, frame_hdr_t *frame)
{
  int len = sizeof(frame_ctl_rsp_frame) + strlen(frame->id);
  frame_ctl_rsp_frame *msg;
  int ret;

  msg = (frame_ctl_rsp_frame *) malloc(len);
  memset(msg, 0, len);

  msg->hdr.ctl = FRAME_CTL_FRAME;
  msg->hdr.len = len;

  msg->parent_shmid = parent_shmid;
  msg->g0 = frame->g0;
  msg->shmid = frame->shmid;
  strcpy(msg->id, frame->id);

  ret = write(sock, msg, len);

  free(msg);

  return ret;
}


int frame_ctl_history(int sock, int id, char *cmd)
{
	int len = sizeof(frame_ctl_rsp_history);
	frame_ctl_rsp_history *msg;
	int ret;

	if (cmd)
		len += strlen(cmd);

	msg = (frame_ctl_rsp_history *) malloc(len);
	memset(msg, 0, len);

	msg->hdr.ctl = FRAME_CTL_HISTORY;
	msg->hdr.len = len;

	msg->id = id;
	if (cmd)
		strcpy(msg->cmd, cmd);

	ret = write(sock, msg, len);

	free(msg);

	return ret;
}
