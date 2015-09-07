/****************************************************************************/
/* TestFarm                                                                 */
/* Interface logical link management - FIFO/PORT data source                */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 06-APR-2007                                                    */
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
#include <stdlib.h>
#include <string.h>

#include "useful.h"
#include "link.h"
#include "link_bin.h"


static int link_bin_parse_source(char *str)
{
  int mode = LINK_MODE_BITS;
  char *s1 = strupper(str);

  /* Parse link source qualifiers */
  while ( *s1 != '\0' ) {
    char *s2 = strchr(s1, ',');

    if ( s2 != NULL )
      *(s2++) = '\0';
    else
      s2 = s1 + strlen(s1);

    if ( strncmp("PORT", s1, 4) == 0 ) {
      mode |= LINK_MODE_PORT;
      s1 += 4;

      if ( *s1 == ':' )
	s1++;

      if ( *s1 != '\0' ) {
	unsigned int bit = atoi(s1);
	mode &= ~LINK_MODE_BITS;
	mode |= bit & LINK_MODE_BITS;
      }
    }
    else if ( strcmp("FIFO", s1) == 0 )
      mode |= LINK_MODE_FIFO;
    else
      return -1;

    s1 = s2;
  }

  return mode;
}


static int link_bin_parse(link_src_bin_t *bin, int argc, char **argv)
{
  int argx;
  int source = -1;
  int mode = 0;

  if ( argc < 1 )
    return -1;

  /* Get options */
  for (argx = 0; argx < argc; argx++) {
    if ( strcmp(argv[argx], "-inv") == 0 ) {
      mode |= LINK_MODE_INV;
    }
    else if ( strcmp(argv[argx], "-hex") == 0 ) {
      mode |= LINK_MODE_HEX;
    }
    else {
      break;
    }
  }

  /* Get link source */
  if ( argx < argc )
    source = link_bin_parse_source(argv[argx++]);
  if ( source == -1 )
    return -1;

  /* Setup link properties */
  bin->mode = mode | source;

  if ( argx < argc )
    bin->mask = strtoul(argv[argx++], (char **) NULL, 0);
  if ( argx < argc )
    bin->equal = strtoul(argv[argx++], (char **) NULL, 0);

  return 0;
}


static void link_bin_clear(link_src_bin_t *bin)
{
  bin->mode = 0;
  bin->mask = 0;
  bin->equal = 0;
  bin->state = 0xFFFFFFFF;
}


static void link_bin_show(link_src_bin_t *bin)
{
  int len = 0;

  if ( bin->mode & LINK_MODE_INV )
    printf("-inv ");
  if ( bin->mode & LINK_MODE_HEX )
    printf("-hex ");

  /* Link mode */
  printf("source=");
  if ( bin->mode & LINK_MODE_PORT ) {
    unsigned int bit = bin->mode & LINK_MODE_BITS;
    len += printf("PORT");
    if ( bit != LINK_MODE_BITS )
      printf(":%u", bit);
  }
  if ( bin->mode & LINK_MODE_FIFO )
    len += printf("%sFIFO", len ? ",":"");

  /* Link mask */
  printf(" mask=0x%lX equal=0x%lX", bin->mask, bin->equal);
}


static int link_bin_send_data(link_src_bin_t *bin, char **argv, void **pbuf)
{
  void *buf = NULL;
  int size = 0;
  unsigned long v;

  /* Retrieve data word to send */
  v = strtol(argv[2], (char **) NULL, 0) & 0xFFFF;

  if ( bin->mode & LINK_MODE_HEX ) {
    char str[8];

    size = snprintf(str, sizeof(str), "!%04lX\n", v);
    buf = strdup(str);
  }
  else {
    size = sizeof(v);
    buf = malloc(size);
    *((unsigned long *) buf) = 0x40000000 | v;
  }

  if ( pbuf != NULL )
    *pbuf = buf;  

  return size;
}


int link_bin_init(void)
{
  link_src_desc.help_str0 =
    "[-inv] [-hex] FIFO,PORT[:<bit>] [[<mask>] <equal>]";
  link_src_desc.help_str1 =
    "  Option -inv forces inverting input bits.\n"
    "  Option -hex enables hexadecimal dump mode.\n"
    "  Data flow source may be FIFO, PORT or both\n"
    "  Bitwise PORT monitoring may be activated with the :<bit> qualifier.\n"
    "  Inputs are AND-ed with <mask> and ignored if result not equal to <equal>.\n";
  link_src_desc.parse = (void *) link_bin_parse;
  link_src_desc.clear = (void *) link_bin_clear;
  link_src_desc.show = (void *) link_bin_show;
  link_src_desc.send_data = (void *) link_bin_send_data;

  return 0;
}
