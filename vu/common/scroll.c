/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* Scroll dir definitions                                                   */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 21-JUN-2007                                                    */
/****************************************************************************/

/*
 * $Revision: 586 $
 * $Date: 2007-06-21 14:11:50 +0200 (jeu., 21 juin 2007) $
 */

#include <stdio.h>
#include "scroll.h"


char *scroll_str(unsigned char direction)
{
  char *sdir = NULL;

  switch ( direction ) {
  case SCROLL_UP:    sdir = "up";    break;
  case SCROLL_DOWN:  sdir = "down";  break;
  case SCROLL_LEFT:  sdir = "left";  break;
  case SCROLL_RIGHT: sdir = "right"; break;
  }

  return sdir;
}
