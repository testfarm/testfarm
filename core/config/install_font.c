/****************************************************************************/
/* TestFarm                                                                 */
/* Standard Fonts                                                           */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 08-AUG-2003                                                    */
/****************************************************************************/

/*
 * $Revision: 42 $
 * $Date: 2006-06-03 15:30:01 +0200 (sam., 03 juin 2006) $
 */

#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>

#include "install.h"


#define DEFAULT_FONT_FIXED "MiscFixed 10"

char *get_font_fixed(void)
{
  char *env = getenv("TESTFARM_FONT_FIXED");
  return (env != NULL) ? env : DEFAULT_FONT_FIXED;
}
