/****************************************************************************/
/* TestFarm                                                                 */
/* Output file, XML format                                                  */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 29-JAN-2004                                                    */
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

#ifndef __TESTFARM_OUTPUT_XML_H
#define __TESTFARM_OUTPUT_XML_H

#include "codegen.h"
#include "validate.h"
#include "output_file.h"
#include "output_tree.h"
#include "output_node.h"

#define OUTPUT_XML(_file_) ((output_xml_t *) (_file_))

typedef struct {
  output_file_t file;
  tree_object_t *cur_obj;
  int indent;
  int channel;
} output_xml_t;

extern output_xml_t *output_xml_alloc(void);
extern void output_xml_free(output_xml_t *ox);

extern void output_xml_filename(output_xml_t *ox, char *filename);
extern int output_xml_open(output_xml_t *ox, output_tree_t *tree);
extern void output_xml_close(output_xml_t *ox);

extern int output_xml_case(output_xml_t *ox, tree_object_t *object);
extern int output_xml_validated(output_xml_t *ox, validate_t *v);
extern int output_xml_verdict(output_xml_t *ox, int verdict, int criticity);
extern int output_xml_stdio(output_xml_t *ox, int channel, char *str);

extern int output_xml_read(output_xml_t *ox, output_case_t *ocase,
                           output_file_func_t *func, void *arg,
                           char *filter);

extern int output_xml_merge(output_xml_t *ox, char *log_name);

#endif /* __TESTFARM_OUTPUT_XML_H */
