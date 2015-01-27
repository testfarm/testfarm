/****************************************************************************/
/* TestFarm                                                                 */
/* XPM to Perl Pixmap conversion tool                                       */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 10-JUN-2004                                                    */
/****************************************************************************/

/* $Revision: 42 $ */
/* $Date: 2006-06-03 15:30:01 +0200 (sam., 03 juin 2006) $ */

#include <stdio.h>

#include "logo.xpm"

int main (int argc, char *argv[])
{
  int rows, colors;
  int n;
  int i;

  sscanf(xpm_logo_data[0], "%*d %d %d", &rows, &colors);

  printf("my @logo_xpm = (\n");

  n = 1 + rows + colors;
  for (i = 0; i < n; i++) {
    char *s = xpm_logo_data[i];
    printf("'");
    while ( *s != '\0' ) {
      if ( *s == '\'' )
	printf("'.\"%c\".'", *s);
      else
	printf("%c", *s);
      s++;
    }
    printf("',\n");
  }

  printf(");\n");

  return 0;
}
