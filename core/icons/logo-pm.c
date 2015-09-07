/****************************************************************************/
/* TestFarm                                                                 */
/* XPM to Perl Pixmap conversion tool                                       */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 10-JUN-2004                                                    */
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
