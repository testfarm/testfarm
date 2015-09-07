/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* Display control messages                                                 */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 25-APR-2007                                                    */
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

#ifndef __TVU_FRAME_CTL_MSG_H__
#define __TVU_FRAME_CTL_MSG_H__

#include "scroll.h"
#include "capture_cap.h"
#include "frame_geometry.h"
#include "frame_hdr.h"
#include "pattern.h"

#define FRAME_CTL_NONE        0x00
#define FRAME_CTL_INIT        0x80
#define FRAME_CTL_WINDOW      0x81
#define FRAME_CTL_REFRESH     0x82
#define FRAME_CTL_PATTERN     0xC3
#define FRAME_CTL_MATCH       0x84
#define FRAME_CTL_REAP        0x45
#define FRAME_CTL_KEY         0x46
#define FRAME_CTL_POINTER     0x47
#define FRAME_CTL_SCROLL      0x48
#define FRAME_CTL_COMMAND     0x49
#define FRAME_CTL_PERIOD      0xCA
#define FRAME_CTL_SOURCE      0xCB
#define FRAME_CTL_PAD         0x8C
#define FRAME_CTL_FRAME       0x8D
#define FRAME_CTL_HISTORY     0x8E

#define FRAME_CTL_MAXSIZE (sizeof(frame_ctl_rsp) + 1024)


typedef struct {
  unsigned char ctl;
  unsigned int len;
} frame_ctl_hdr;

typedef struct {
  frame_ctl_hdr hdr;            /* FRAME_CTL_INIT */
  int shmid;                    /* Root frame SHM id */
  capture_cap_t cap;            /* Capture device capabilities */
  char cwd[1];                  /* Client's Current Working Directory */
} frame_ctl_rsp_init;

typedef struct {
  frame_ctl_hdr hdr;            /* FRAME_CTL_WINDOW */
  frame_geometry_t g;           /* Active frame window */
} frame_ctl_rsp_window;

typedef struct {
  frame_ctl_hdr hdr;            /* FRAME_CTL_REFRESH */
  int shmid;                    /* Frame SHM id */
  frame_geometry_t g;           /* Refresh area geometry */
} frame_ctl_rsp_refresh;

typedef struct {
  frame_ctl_hdr hdr;            /* FRAME_CTL_PATTERN */
  int shmid;                    /* Frame SHM id */
  frame_geometry_t g;           /* Pattern geometry */
  unsigned int mode;            /* Pattern detection mode */
  int type;                     /* Pattern type */
  fuzz_t fuzz;                  /* Image pattern color-fuzz value */
  unsigned char loss[2];        /* Image pattern pixel-loss */
  char id_source[1];            /* Pattern id, followed by pattern source */
} frame_ctl_rsp_pattern;

typedef struct {
  frame_ctl_hdr hdr;            /* FRAME_CTL_MATCH */
  int state;                    /* Match state */
  char id[1];                   /* Pattern id */
} frame_ctl_rsp_match;

typedef struct {
  frame_ctl_hdr hdr;            /* FRAME_CTL_PERIOD */
  unsigned int period;          /* Refresh period in milliseconds */
} frame_ctl_rsp_period;

typedef struct {
  frame_ctl_hdr hdr;            /* FRAME_CTL_SOURCE */
  char args[1];                 /* Argument string */
} frame_ctl_rsp_source;

typedef struct {
  frame_ctl_hdr hdr;            /* FRAME_CTL_PAD */
  int nmemb;                    /* Number of elements in the pad list */
  frame_geometry_t gtab[1];     /* Pad area geometry list */
} frame_ctl_rsp_pad;

typedef struct {
  frame_ctl_hdr hdr;            /* FRAME_CTL_FRAME */
  int parent_shmid;             /* Parent frame SHM id */
  frame_geometry_t g0;          /* Frame geometry in root frame */
  int shmid;                    /* Frame SHM id */
  char id[1];                   /* Frame id */
} frame_ctl_rsp_frame;

typedef struct {
  frame_ctl_hdr hdr;            /* FRAME_CTL_HISTORY */
  int id;                       /* Command id in history list */
  char cmd[1];                  /* Command string */
} frame_ctl_rsp_history;

typedef union {
  frame_ctl_hdr hdr;
  frame_ctl_rsp_init init;
  frame_ctl_rsp_window window;
  frame_ctl_rsp_refresh refresh;
  frame_ctl_rsp_pattern pattern;
  frame_ctl_rsp_match match;
  frame_ctl_rsp_period period;
  frame_ctl_rsp_source source;
  frame_ctl_rsp_pad pad;
  frame_ctl_rsp_frame frame;
  frame_ctl_rsp_history history;
} frame_ctl_rsp;

typedef frame_ctl_rsp_pattern frame_ctl_cmd_pattern;
typedef frame_ctl_rsp_period frame_ctl_cmd_period;
typedef frame_ctl_rsp_source frame_ctl_cmd_source;

typedef struct {
  frame_ctl_hdr hdr;            /* FRAME_CTL_REAP */
} frame_ctl_cmd_reap;

typedef struct {
  frame_ctl_hdr hdr;            /* FRAME_CTL_KEY */
  unsigned char down;
  unsigned long key;
} frame_ctl_cmd_key;

typedef struct {
  frame_ctl_hdr hdr;            /* FRAME_CTL_POINTER */
  unsigned char buttons;
  unsigned int x;
  unsigned int y;
} frame_ctl_cmd_pointer;

typedef struct {
  frame_ctl_hdr hdr;            /* FRAME_CTL_SCROLL */
  unsigned char direction;
} frame_ctl_cmd_scroll;

typedef struct {
  frame_ctl_hdr hdr;            /* FRAME_CTL_COMMAND */
  char str[1];                  /* Command string */
} frame_ctl_cmd_command;

typedef union {
  frame_ctl_hdr hdr;
  frame_ctl_cmd_pattern pattern;
  frame_ctl_cmd_reap reap;
  frame_ctl_cmd_key key;
  frame_ctl_cmd_pointer pointer;
  frame_ctl_cmd_scroll scroll;
  frame_ctl_cmd_command command;
  frame_ctl_cmd_period period;
  frame_ctl_cmd_source source;
} frame_ctl_cmd;


extern int frame_ctl_init(int sock, int shmid, capture_cap_t cap);
extern int frame_ctl_window(int sock, frame_geometry_t *g);
extern int frame_ctl_refresh(int sock, int shmid, frame_geometry_t *g);
extern int frame_ctl_pattern(int sock, pattern_t *pattern);
extern int frame_ctl_pattern_remove(int sock, char *id);
extern int frame_ctl_match(int sock, char *id, int state);
extern int frame_ctl_reap(int sock);
extern int frame_ctl_key(int sock, unsigned char down, unsigned long key);
extern int frame_ctl_pointer(int sock, unsigned char buttons, unsigned int x, unsigned int y);
extern int frame_ctl_scroll(int sock, unsigned char direction);
extern int frame_ctl_command(int sock, char *str);
extern int frame_ctl_period(int sock, unsigned int period);
extern int frame_ctl_source(int sock, char *args);
extern int frame_ctl_pad(int sock, frame_geometry_t *gtab, int nmemb);
extern int frame_ctl_frame(int sock, int parent_shmid, frame_hdr_t *frame);
extern int frame_ctl_history(int sock, int id, char *cmd);

#endif /* __TVU_FRAME_CTL_MSG_H__ */
