/****************************************************************************/
/* TestFarm                                                                 */
/* Test Suite User Interface: command options                               */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 25-SEP-2002                                                    */
/****************************************************************************/

/*
 * $Revision: 770 $
 * $Date: 2007-10-09 14:34:18 +0200 (mar., 09 oct. 2007) $
 */

#ifndef __ATSGUI_OPTIONS_H__
#define __ATSGUI_OPTIONS_H__

extern int opt_noflags;
extern int opt_go;
extern int opt_quit;
extern char *opt_target;
extern unsigned long opt_depth;
extern char *opt_operator;
extern char *opt_release;
extern int opt_command;
extern int opt_service;
extern int opt_nogui;

extern int opt_get(int argc, char *argv[]);
extern void opt_usage(int summary);

#endif /* __ATSGUI_OPTIONS_H__ */
