/****************************************************************************/
/* TestFarm                                                                 */
/* Output file abstract descriptor                                          */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 02-FEB-2004                                                    */
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

#ifndef __TESTFARM_OUTPUT_FILE_H
#define __TESTFARM_OUTPUT_FILE_H

#define OUTPUT_FILE_UNKNOWN -1
#define OUTPUT_FILE_XML  0

#define OUTPUT_CHANNEL_NONE       -1
#define OUTPUT_CHANNEL_STDIN      0
#define OUTPUT_CHANNEL_STDOUT     1
#define OUTPUT_CHANNEL_STDERR     2
#define OUTPUT_CHANNEL_IN_TITLE   3
#define OUTPUT_CHANNEL_IN_HEADER  4
#define OUTPUT_CHANNEL_IN_VERDICT 5
#define OUTPUT_CHANNEL_N          6

typedef void output_file_func_t(void *arg, int channel, char *str);

typedef struct {
  int type;
  char *name;         /* Output file name */
  FILE *f;            /* Output file descriptor */
} output_file_t;

extern void output_file_name(output_file_t *of, char *filename);
extern int output_file_type(char *filename);

#endif /* __TESTFARM_OUTPUT_FILE_H */
