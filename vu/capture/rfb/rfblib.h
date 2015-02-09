/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* Remote Frame Buffer device primitives                                    */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 25-APR-2007                                                    */
/****************************************************************************/

/*
 * $Revision: 621 $
 * $Date: 2007-07-09 14:48:13 +0200 (lun., 09 juil. 2007) $
 */

#ifndef __RFBLIB_H__
#define __RFBLIB_H__

#include <netinet/in.h>

#define RFB_PORT 5900


typedef struct {
  char *host;
  int port;
  struct sockaddr_in iremote;
  int sock;
  int major, minor;

  unsigned short width, height;
  char *name;

  unsigned int bits_per_pixel;
  unsigned int depth;
  int big_endian;
  int true_color;
  unsigned short red_max, green_max, blue_max;
  unsigned char red_shift, green_shift, blue_shift;

  unsigned int pointer_x;
  unsigned int pointer_y;
  unsigned char pointer_buttons;
} rfb_t;


extern int rfb_Debug(int state);
extern rfb_t *rfb_New(char *host, int port, char *passwd, int shared);
extern void rfb_Destroy(rfb_t *rfb);
extern int rfb_Report(rfb_t *rfb, FILE *f, char *hdr);
extern void rfb_Shutdown(rfb_t *rfb);

extern int rfb_KeyEvent(rfb_t *rfb, int down, unsigned long key);

#define RFB_BUTTON_1 0x01
#define RFB_BUTTON_2 0x02
#define RFB_BUTTON_3 0x04
#define RFB_BUTTON_UP    0x08
#define RFB_BUTTON_DOWN  0x10
#define RFB_BUTTON_LEFT  0x20
#define RFB_BUTTON_RIGHT 0x40

extern int rfb_PointerEvent(rfb_t *rfb, unsigned char buttons, unsigned int x, unsigned int y);
extern int rfb_PointerPosition(rfb_t *rfb, unsigned int x, unsigned int y);
extern int rfb_PointerButtons(rfb_t *rfb, unsigned char buttons);
extern int rfb_PointerScroll(rfb_t *rfb, unsigned char direction);

extern int rfb_SetEncodings(rfb_t *rfb, int tabc, const unsigned long tabv[]);

extern int rfb_SetPixelFormat(rfb_t *rfb);
extern int rfb_SetRGBPixelFormat(rfb_t *rfb, int bits_per_color);

extern int rfb_UpdateRequest(rfb_t *rfb, unsigned char incremental,
                             unsigned int x, unsigned int y, unsigned int w, unsigned int h);

#endif /* __RFBLIB_H__ */
