/****************************************************************************/
/* TestFarm                                                                 */
/* Default settings storage                                                 */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 29-MAR-2005                                                    */
/****************************************************************************/

/* $Revision: 42 $ */
/* $Date: 2006-06-03 15:30:01 +0200 (sam., 03 juin 2006) $ */

#ifndef __CONFIG_DEFAULTS_H__
#define __CONFIG_DEFAULTS_H__

extern char *defaults_get_string(char *name);
extern char *defaults_set_string(char *name, char *content);

#endif /* __CONFIG_DEFAULTS_H__ */
