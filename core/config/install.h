/****************************************************************************/
/* TestFarm                                                                 */
/* Standard Installation Environment Settings                               */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 08-AUG-2003                                                    */
/****************************************************************************/

/* $Revision: 1084 $ */
/* $Date: 2009-10-29 18:56:37 +0100 (jeu., 29 oct. 2009) $ */

#ifndef __TESTFARM_INSTALL_H__
#define __TESTFARM_INSTALL_H__

#define INSTALL_HOME   "/opt/testfarm"
#define INSTALL_CONFIG "/var/testfarm"
#define INSTALL_DEFAULT_BROWSER "firefox"
#define INSTALL_DEFAULT_PERLDB "ptkdb"
#define INSTALL_LOGVIEW "testfarm-logview"

extern char *get_home(void);
extern char *get_config(void);
extern char *get_font_fixed(void);
extern char *get_browser(void);
extern char *get_perldb(void);

extern char *get_user_config(char *fmt, ...);

extern char *get_lib(char *fname);

#endif /* __TESTFARM_INSTALL_H__ */
