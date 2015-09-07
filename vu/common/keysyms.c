/****************************************************************************/
/* TestFarm VNC Interface                                                   */
/* Key symbols management                                                   */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 25-NOV-2003                                                    */
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
#include <string.h>
#include <malloc.h>

#include "keysyms.h"


extern keysyms_t keysyms[];

static keysyms_class_t *keysyms_class_tab = NULL;
static int keysyms_class_size = 0;


void keysyms_init(void)
{
  keysyms_t *tab;
  int i;

  /* Evaluate the number of classes */
  keysyms_class_size = 0;
  tab = keysyms;
  while ( tab->sym != NULL ) {
    if ( tab->v == 0 )
      keysyms_class_size++;
    tab++;
  }

  /* Alloc and clear keysyms class table */
  keysyms_class_tab = (keysyms_class_t *) malloc(sizeof(keysyms_class_t) * keysyms_class_size);

  for (i = 0; i < keysyms_class_size; i++) {
    keysyms_class_tab[i].name = NULL;
    keysyms_class_tab[i].tab = NULL;
    keysyms_class_tab[i].size = 0;
    keysyms_class_tab[i].enable = 0;
  }

  /* Feed keysyms class table */
  i = -1;
  tab = keysyms;
  while ( (tab->sym != NULL) && (i < keysyms_class_size) ) {
    if ( tab->v == 0 ) {
      i++;

      if ( i < keysyms_class_size ) {
        if ( (strcmp(tab->sym, "LATIN1") == 0) ||
             (strcmp(tab->sym, "MISCELLANY") == 0) ) {
          keysyms_class_tab[i].enable = 1;
        }

        keysyms_class_tab[i].name = tab->sym;
        keysyms_class_tab[i].tab = (++tab);
      }
    }
    else if ( i >= 0 ) {
      keysyms_class_tab[i].size++;
    }

    tab->class = &(keysyms_class_tab[i]);
    tab++;
  }
}


void keysyms_destroy(void)
{
  keysyms_t *tab = keysyms;

  /* Unlink class descriptors */
  while ( tab->sym != NULL ) {
    tab->class = NULL;
    tab++;
  }

  /* Kill key symbols */
  if ( keysyms_class_tab != NULL ) {
    free(keysyms_class_tab);
    keysyms_class_tab = NULL;
    keysyms_class_size = 0;
  }
}


void keysyms_show(char *hdr, char *class_name, int show_content)
{
  int i;

  /* Show class table */
  for (i = 0; i < keysyms_class_size; i++) {
    keysyms_class_t *class = &(keysyms_class_tab[i]);
    int j;

    if ( (class_name == NULL) || (strcmp(class_name, class->name) == 0) ) {
      printf("%s", hdr);
      if ( class->enable == 0 )
        printf("(");
      printf("%s", class->name);
      if ( class->enable == 0 )
        printf(")");
      printf("\n");

      if ( show_content ) {
        for (j = 0; j < class->size; j++) {
          keysyms_t *tab = &(class->tab[j]);
          printf("%s  0x%04lX %s\n", hdr, tab->v, tab->sym);
        }
      }
    }
  }
}


keysyms_class_t *keysyms_class(char *class_name)
{
  keysyms_class_t *class = NULL;
  int i;

  for (i = 0; (i < keysyms_class_size) && (class == NULL); i++) {
    class = &(keysyms_class_tab[i]);

    if ( strcmp(class->name, class_name) != 0 )
      class = NULL;
  }

  return class;
}


keysyms_t *keysyms_retrieve(char *key)
{
  keysyms_t *keysym = NULL;
  int i;

  for (i = 0; (i < keysyms_class_size) && (keysym == NULL); i++) {
    keysyms_class_t *class = &(keysyms_class_tab[i]);

    if ( class->enable ) {
      int j;

      for (j = 0; (j < class->size) && (keysym == NULL); j++) {
        keysym = &(class->tab[j]);

        if ( strcmp(keysym->sym, key) != 0 )
          keysym = NULL;
      }
    }
  }

  return keysym;
}


keysyms_t *keysyms_retrieve_by_code(unsigned long v)
{
  keysyms_t *tab = keysyms;

  while ( tab->sym != NULL ) {
    if ( tab->v == v )
      return tab;
    tab++;
  }

  return NULL;
}
