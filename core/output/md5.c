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

#include <stdio.h>

#include "md5.h"

int md5_sign(char *filename, unsigned char *signature)
{
  FILE *f;
  MD5_CTX ctx;
  unsigned char buf[BUFSIZ];
  unsigned char digest[MD5_DIGEST_LENGTH];
  int i;

  signature[0] = '\0';

  if ( (f = fopen(filename, "r")) == NULL )
    return -1;

  MD5_Init(&ctx);

  clearerr(f);

  while ( ! (feof(f) || ferror(f)) ) {
    size_t size = fread(buf, sizeof(unsigned char), BUFSIZ, f);
    if ( size > 0 )
      MD5_Update(&ctx, buf, size);
  }

  fclose(f);

  MD5_Final(digest, &ctx);
  for (i = 0; i < MD5_DIGEST_LENGTH; i++)
    sprintf((char *) (signature+(i*2)), "%02x", digest[i]);

  return 0;
}
