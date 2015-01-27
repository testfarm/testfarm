/****************************************************************************/
/* TestFarm                                                                 */
/* MD5 checksum computation                                                 */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 19-FEB-2001                                                    */
/****************************************************************************/

/*
 * $Revision: 42 $
 * $Date: 2006-06-03 15:30:01 +0200 (sam., 03 juin 2006) $
 */

#ifndef __TESTFARM_MD5_H
#define __TESTFARM_MD5_H

#include <openssl/md5.h>
#define MD5_SIGNATURE_LENGTH (MD5_DIGEST_LENGTH*2+1)

extern int md5_sign(char *filename, unsigned char *signature);

#endif /* __TESTFARM_MD5_H */
