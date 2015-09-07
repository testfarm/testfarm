/****************************************************************************/
/* TestFarm                                                                 */
/* MD5 checksum computation                                                 */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 19-FEB-2001                                                    */
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

#ifndef __TESTFARM_MD5_H
#define __TESTFARM_MD5_H

#include <openssl/md5.h>
#define MD5_SIGNATURE_LENGTH (MD5_DIGEST_LENGTH*2+1)

extern int md5_sign(char *filename, unsigned char *signature);

#endif /* __TESTFARM_MD5_H */
