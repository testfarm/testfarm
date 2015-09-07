/****************************************************************************/
/* TestFarm VNC Interface                                                   */
/* Key symbols management                                                   */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 12-NOV-2001                                                    */
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

#ifndef __TVU_KEYSYMS_H__
#define __TVU_KEYSYMS_H__

typedef struct keysyms_class_s keysyms_class_t;

typedef struct {
  char *sym;
  unsigned long v;
  keysyms_class_t *class;
} keysyms_t;

struct keysyms_class_s {
  char *name;
  keysyms_t *tab;
  int size;
  int enable;
};

extern void keysyms_init(void);
extern void keysyms_destroy(void);

extern void keysyms_show(char *hdr, char *class_name, int show_content);
extern keysyms_class_t *keysyms_class(char *class_name);
extern keysyms_t *keysyms_retrieve(char *key);
extern keysyms_t *keysyms_retrieve_by_code(unsigned long v);

#endif /* __TVU_KEYSYMS_H__ */
