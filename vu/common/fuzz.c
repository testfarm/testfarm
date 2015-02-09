/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* Color fuzzing primitives                                                 */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 22-OCT-2007                                                    */
/****************************************************************************/

/*
 * $Revision: 787 $
 * $Date: 2007-11-06 11:58:20 +0100 (mar., 06 nov. 2007) $
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
