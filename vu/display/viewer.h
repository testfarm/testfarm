/****************************************************************************/
/* TestFarm RFB Interface                                                   */
/* Remote Frame Buffer display - PPM viewer                                 */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 14-JAN-2004                                                    */
/****************************************************************************/

/*
 * $Revision: 430 $
 * $Date: 2007-03-27 16:46:06 +0200 (mar., 27 mars 2007) $
 */

#ifndef __RFB_DISPLAY_VIEWER_H
#define __RFB_DISPLAY_VIEWER_H

extern int viewer_init(GtkWindow *window);
extern void viewer_done(void);
extern int viewer_show(char *fname);

#endif /* __RFB_DISPLAY_VIEWER_H */
