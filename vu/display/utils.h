/****************************************************************************/
/* TestFarm Virtual User                                                    */
/* Display Tool - Miscellaneous utilities                                   */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 14-JAN-2004                                                    */
/****************************************************************************/

/*
 * $Revision: 813 $
 * $Date: 2007-11-22 17:54:18 +0100 (jeu., 22 nov. 2007) $
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
