/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* Scroll dir definitions                                                   */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 21-JUN-2007                                                    */
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
