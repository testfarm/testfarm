/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* Display control                                                          */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 05-JAN-2004                                                    */
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
#include <string.h>
#include <malloc.h>
#include <errno.h>
#include <unistd.h>
#include <glib.h>
#include <sys/stat.h>

#include "user_dir.h"
#include "frame_ctl.h"


#define FRAME_CTL_SUBDIR  "display"


char *frame_ctl_sockdir(void)
{
	return user_dir(FRAME_CTL_SUBDIR, NULL);
}


char *frame_ctl_sockname(char *display_name)
{
	return user_dir(FRAME_CTL_SUBDIR, display_name);
}
