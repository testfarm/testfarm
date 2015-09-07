/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* Display Tool - Miscellaneous utilities                                   */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 14-JAN-2004                                                    */
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

#ifndef __TVU_DISPLAY_UTILS_H
#define __TVU_DISPLAY_UTILS_H

#include <gtk/gtk.h>

/* Debug flags */
#define DEBUG_CONNECT   1   /* Connection events */
#define DEBUG_PATTERN   2   /* PATTERN / PAD events */
#define DEBUG_CTL       4   /* CTL messages received from TVU Engine */
#define DEBUG_FRAME     8   /* Frame selection */
#define DEBUG_RFU1     16
#define DEBUG_RFU2     32
#define DEBUG_REFRESH  64   /* Display refresh event */
#define DEBUG_GTK     128   /* GTK expose/input events */

extern unsigned int opt_debug;

#define debug(level, args...) if ( opt_debug & level ) fprintf(stderr, "TVU Display: DEBUG: " args)
#define error(args...) fprintf(stderr, "TVU Display: " args)

extern char *lprintf(GtkLabel *label, char *fmt, ...);

extern GtkLabel *eprintf_label;
extern void eprintf(char *fmt, ...);
extern void eprintf2(char *fmt, ...);

extern char *strfilename(char *filename);

#endif /* __TVU_DISPLAY_UTILS_H */
