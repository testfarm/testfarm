/****************************************************************************/
/* TestFarm                                                                 */
/* Output file abstract descriptor                                          */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 02-FEB-2004                                                    */
/****************************************************************************/

/*
 * $Revision: 754 $
 * $Date: 2007-09-29 17:54:20 +0200 (sam., 29 sept. 2007) $
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
