/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* Display control                                                          */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 05-JAN-2004                                                    */
/****************************************************************************/

/*
 * $Revision: 1153 $
 * $Date: 2010-06-06 11:36:58 +0200 (dim., 06 juin 2010) $
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
