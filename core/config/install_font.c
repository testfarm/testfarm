/****************************************************************************/
/* TestFarm                                                                 */
/* Standard Fonts                                                           */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 08-AUG-2003                                                    */
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
#include <stdlib.h>
#include <gtk/gtk.h>

#include "install.h"


#define DEFAULT_FONT_FIXED "MiscFixed 10"

char *get_font_fixed(void)
{
  char *env = getenv("TESTFARM_FONT_FIXED");
  return (env != NULL) ? env : DEFAULT_FONT_FIXED;
}
