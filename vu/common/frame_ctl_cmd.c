/****************************************************************************/
/* TestFarm VNC Interface                                                   */
/* Display control (display->client commands only)                          */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 02-APR-2007                                                    */
/****************************************************************************/

/*
 * $Revision: 487 $
 * $Date: 2007-04-25 15:20:30 +0200 (mer., 25 avril 2007) $
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "frame_ctl.h"
#include "frame_ctl_msg.h"


int frame_ctl_reap(int sock)
{
  int len = sizeof(frame_ctl_cmd_reap);
  char buf[len];
  frame_ctl_cmd_reap *msg = (frame_ctl_cmd_reap *) buf;

  memset(buf, 0, len);

  msg->hdr.ctl = FRAME_CTL_REAP;
  msg->hdr.len = len;

  return write(sock, buf, len);
}


int frame_ctl_key(int sock, unsigned char down, unsigned long key)
{
  int len = sizeof(frame_ctl_cmd_key);
  char buf[len];
  frame_ctl_cmd_key *msg = (frame_ctl_cmd_key *) buf;

  memset(buf, 0, len);

  msg->hdr.ctl = FRAME_CTL_KEY;
  msg->hdr.len = len;
  msg->down = down;
  msg->key = key;

  return write(sock, buf, len);
}


int frame_ctl_pointer(int sock, unsigned char buttons, unsigned int x, unsigned int y)
{
  int len = sizeof(frame_ctl_cmd_pointer);
  char buf[len];
  frame_ctl_cmd_pointer *msg = (frame_ctl_cmd_pointer *) buf;

  memset(buf, 0, len);

  msg->hdr.ctl = FRAME_CTL_POINTER;
  msg->hdr.len = len;
  msg->buttons = buttons;
  msg->x = x;
  msg->y = y;

  return write(sock, buf, len);
}


int frame_ctl_scroll(int sock, unsigned char direction)
{
  int len = sizeof(frame_ctl_cmd_scroll);
  char buf[len];
  frame_ctl_cmd_scroll *msg = (frame_ctl_cmd_scroll *) buf;

  memset(buf, 0, len);

  msg->hdr.ctl = FRAME_CTL_SCROLL;
  msg->hdr.len = len;
  msg->direction = direction;

  return write(sock, buf, len);
}


int frame_ctl_command(int sock, char *str)
{
  int len = sizeof(frame_ctl_cmd_command) + strlen(str) + 1;
  char buf[len];
  frame_ctl_cmd_command *msg = (frame_ctl_cmd_command *) buf;

  memset(buf, 0, len);

  msg->hdr.ctl = FRAME_CTL_COMMAND;
  msg->hdr.len = len;
  strcpy(msg->str, str);

  return write(sock, buf, len);
}
