/****************************************************************************/
/* TestFarm                                                                 */
/* Test Engine -Serial line access link management                          */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 17-AUG-1999                                                    */
/****************************************************************************/

/*
 * $Revision: 29 $
 * $Date: 2006-06-03 10:39:29 +0200 (sam., 03 juin 2006) $
 */

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>

#include "serial.h"

extern int debug;

static struct termios serial_termio_save;
static int serial_fd = -1;

int serial_open(char *dev)
{
  int fd;
  struct termios termio;

  /* Check line is not already open */
  if ( serial_fd != -1 ) {
    fprintf(stderr, "tengine: Serial line %s already open", dev);
    if ( debug )
      fprintf(stderr, " (fd=%d)", serial_fd);
    fprintf(stderr, "\n");
    return -1;
  }

  if ( (fd = open(dev, O_RDWR)) == -1 ) {
    fprintf(stderr, "tengine: Cannot open serial line %s\n", dev);
    return -1;
  }

  if ( !isatty(fd) ) {
    close(fd);
    fprintf(stderr, "tengine: %s is not a serial line\n", dev);
    return -1;
  }

  if ( tcgetattr(fd, &serial_termio_save) ) {
    close(fd);
    fprintf(stderr, "tengine: Cannot get attributes for serial device %s\n", dev);
    return -1;
  }

  /* Ecriture des nouveaux parametres: bps, 8N1 */
  termio = serial_termio_save;
  /*termio.c_cflag = "Inherited from previous configuration using stty" */
  termio.c_oflag = 0;                /* Mode raw */
  termio.c_iflag = 0;
  termio.c_lflag = 0;                /* Mode non canonique */
  termio.c_line = 0;                 /* Discipline 0 */
  termio.c_cc[VMIN] = 1;
  termio.c_cc[VTIME] = 0;

  if ( tcsetattr(fd, TCSANOW, &termio) ) {
    fprintf(stderr, "tengine: Cannot get attributes for serial device %s\n", dev);
    return -1;
  }

  /* Purger les buffers */
  tcflush(fd, TCIOFLUSH);

  if ( debug )
    fprintf(stderr, "Serial line %s open (fd=%d)\n", dev, fd);

  serial_fd = fd;
  return fd;
}


void serial_close(void)
{
  if ( serial_fd == -1 )
    return;

  /* Restore previous tty configuration */
  tcsetattr(serial_fd, TCSANOW, &serial_termio_save);

  /* Close device */
  close(serial_fd);
  serial_fd = -1;
}
