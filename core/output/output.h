/****************************************************************************/
/* TestFarm                                                                 */
/* Test Output management                                                   */
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

#ifndef __TESTFARM_OUTPUT_H
#define __TESTFARM_OUTPUT_H

#include <glib.h>

#include "output_stat.h"
#include "output_tree.h"
#include "output_node.h"
#include "output_file.h"

typedef struct {
  output_tree_t tree;
  GList *cases;            /* List of Test Cases */
  GList *sequences;        /* List of Sequences */
  GList *validates;        /* List of validation descriptors */
  output_case_t *cur_case; /* Current Test Cases being executed */
  output_stat_t stat;      /* Verdict statistics */
  output_file_t *file;     /* Output file descriptor */
} output_t;


extern output_t *output_alloc(tree_t *codegen_tree);
extern void output_destroy(output_t *out);

extern char *output_get_current_node_name(output_t *out);
extern void output_set_operator(output_t *out, char *name);
extern void output_set_release(output_t *out, char *name);
extern void output_set_filename(output_t *out, char *filename);

extern void output_read_node_stdio(output_t *out, char *name,
				   output_file_func_t *func, void *arg,
				   char *filter);

extern void output_write_clear(output_t *out);
extern int output_write_open(output_t *out);
extern void output_write_close(output_t *out);
extern void output_write_case(output_t *out, tree_object_t *object);
extern void output_write_verdict(output_t *out, int verdict, int criticity);
extern void output_write_stdio(output_t *out, int channel, char *str);

extern int output_merge(output_t *out, char *log_name);

extern char *output_suffix(char *filename);

#endif /* __TESTFARM_OUTPUT_H */
