/****************************************************************************/
/* TestFarm                                                                 */
/* Output Test Case/Scenario information                                    */
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

#ifndef __TESTFARM_OUTPUT_INFO_H
#define __TESTFARM_OUTPUT_INFO_H

typedef struct {
  char *name;          /* Node name */
  char *description;   /* Textual Description */
  char *reference;     /* reference to Test Specification */
  int verdict;         /* Verdict result */
  long elapsed;        /* Duration */
  int validated;       /* Validation check */
  int criticity;       /* Criticity level */
} output_info_t;

extern void output_info_init(output_info_t *info, char *name);
extern void output_info_description(output_info_t *info, char *description);
extern void output_info_reference(output_info_t *info, char *reference);
extern void output_info_clear(output_info_t *info);

#endif /* __TESTFARM_OUTPUT_INFO_H */
