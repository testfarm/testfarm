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
