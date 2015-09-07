/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* Color fuzzing primitives                                                 */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 22-OCT-2007                                                    */
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
#include <string.h>

#include "color.h"
#include "fuzz.h"


fuzz_t fuzz_null = FUZZ_NULL;


int fuzz_is_null(fuzz_t *fuzz)
{
  return ((fuzz->color[0] | fuzz->color[1] | fuzz->color[2]) == 0);
}


int fuzz_parse(fuzz_t *fuzz, char *str)
{
  unsigned long color_value = 0;
  int ret;

  /* Extract RGB range value */
  ret = color_parse(str, &color_value);
  if ( ret == 0 )
    color_set_rgb(color_value, fuzz->color);

  return ret;
}


char *fuzz_str(fuzz_t *fuzz)
{
  static char str[16];
  
  snprintf(str, sizeof(str), "#%02X%02X%02X", fuzz->color[0], fuzz->color[1], fuzz->color[2]);

  return str;
}
