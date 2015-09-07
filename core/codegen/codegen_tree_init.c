/****************************************************************************/
/* TestFarm                                                                 */
/* Test Suite code generator : Test Tree SKIP/BREAKPOINT Preload            */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 27-DEC-2000                                                    */
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
#include <unistd.h>

#include "useful.h"

#include "codegen_tree.h"
#include "codegen_tree_error.h"
#include "codegen_tree_init.h"


#define TREE_INIT_CLEAR      0
#define TREE_INIT_BREAKPOINT 1
#define TREE_INIT_FORCE_SKIP 2
#define TREE_INIT_ENABLE     4
#define TREE_INIT_RECURSIVE  8
#define TREE_INIT_QUAL       3


static char *tree_init_filename(tree_t *tree)
{
  int len = strlen(tree->loc.filename);
  char *filename = strcpy(malloc(len+7), tree->loc.filename);
  char *suffix = strrchr(filename, '.');
  if ( (suffix == NULL) || (strcmp(suffix, TREE_DEF_SUFFIX) != 0) )
    suffix = filename + len;
  strcpy(suffix, ".flags");
  return filename;
}


static void tree_init_load_item(tree_item_t *item, int qual);


static void tree_init_load_case(tree_object_t *object, int qual)
{
  tree_case_t *tcase = object->d.Case;

  switch ( qual & TREE_INIT_QUAL ) {
  case TREE_INIT_CLEAR:
    tcase->breakpoint = 0;
    object->flags &= ~FLAG_FORCE_SKIP;
    /*fprintf(stderr, "CASE %s: CLEAR\n", object->parent_item->name);*/
    break;
  case TREE_INIT_BREAKPOINT:
    tcase->breakpoint = (qual & TREE_INIT_ENABLE) ? 1:0;
    /*fprintf(stderr, "CASE %s: BREAKPOINT %d\n", object->parent_item->name, tcase->breakpoint);*/
    break;
  case TREE_INIT_FORCE_SKIP:
    if ( qual & TREE_INIT_ENABLE )
      object->flags |= FLAG_FORCE_SKIP;
    else
      object->flags &= ~FLAG_FORCE_SKIP;
    /*fprintf(stderr, "CASE %s: FORCE_SKIP %d\n", object->parent_item->name, (tobject->flags & FLAG_FORCE_SKIP) ? 1:0);*/
    break;
  default:
    break;
  }
}


static void tree_init_load_seq(tree_object_t *object, int qual)
{
  tree_seq_t *tseq = object->d.Seq;
  int i;

  switch ( qual & TREE_INIT_QUAL ) {
  case TREE_INIT_CLEAR:
    object->flags &= ~FLAG_FORCE_SKIP;
    /*fprintf(stderr, "SEQ %s: CLEAR %d\n", object->parent_item->name);*/
    break;
  case TREE_INIT_BREAKPOINT:
    break;
  case TREE_INIT_FORCE_SKIP:
    if ( qual & TREE_INIT_ENABLE )
      object->flags |= FLAG_FORCE_SKIP;
    else
      object->flags &= ~FLAG_FORCE_SKIP;
    /*fprintf(stderr, "SEQ %s: FORCE_SKIP %d\n", object->parent_item->name, (tobject->flags & FLAG_FORCE_SKIP) ? 1:0);*/
    break;
  default:
    break;
  }

  if ( qual & TREE_INIT_RECURSIVE ) {
    for (i = 0; i < tseq->nnodes; i++) {
      tree_init_load_item(tseq->nodes[i], qual);
    }
  }
}


static void tree_init_load_object(tree_object_t *object, int qual)
{
  switch ( object->type ) {
  case TYPE_CASE:
    tree_init_load_case(object, qual);
    break;
  case TYPE_SEQ:
    tree_init_load_seq(object, qual);
    break;
  default :
    /* Nothing */
    break;
  }
}


static void tree_init_load_item(tree_item_t *item, int qual)
{
  if ( item != NULL )
    tree_init_load_object(item->object, qual);
}


static void tree_init_load_line(tree_t *tree, char *str, tree_err_loc_t *loc)
{
  char *s1, *s2;
  int qual = -1;

  /* Remove comments */
  if ( (s1 = strchr(str, '#')) != NULL )
    *s1 = NUL;

  /* Parse init qualifier */
  s1 = strskip_spaces(str);
  s2 = strskip_chars(s1);
  if ( *s2 != NUL )
    *(s2++) = NUL;
  s2 = strskip_spaces(s2);

  if ( *s1 == NUL )
    return;

  strupper(s1);

  if ( strcmp(s1, "BREAKPOINT") == 0 ) {
    qual = TREE_INIT_BREAKPOINT | TREE_INIT_ENABLE;
  }
  else if ( strcmp(s1, "/BREAKPOINT") == 0 ) {
    qual = TREE_INIT_BREAKPOINT;
  }
  else if ( strcmp(s1, "FORCE_SKIP") == 0 ) {
    qual = TREE_INIT_FORCE_SKIP | TREE_INIT_ENABLE;
  }
  else if ( strcmp(s1, "/FORCE_SKIP") == 0 ) {
    qual = TREE_INIT_FORCE_SKIP;
  }
  else if ( strcmp(s1, "CLEAR") == 0 ) {
    qual = TREE_INIT_CLEAR;
  }
  else {
    tree_warning(tree, loc, "Illegal init qualifier");
  }

  if ( qual >= 0 ) {
    /* Parse node names */
    while ( *s2 != NUL ) {
      /* Extract node name */
      s1 = s2;
      s2 = strskip_chars(s1);
      if ( *s2 != NUL )
        *(s2++) = NUL;
      s2 = strskip_spaces(s2);

      /* Wildcard for all nodes */
      if ( strcmp(s1, "*") == 0 ) {
        tree_foreach(tree, (tree_func_t *) tree_init_load_object, (void *) qual);
      }
      else {
        tree_item_t *item = tree_retrieve(tree, s1);

        if ( item != NULL ) {
          if ( item->object->type == TYPE_SEQ )
            qual |= TREE_INIT_RECURSIVE;
          tree_init_load_item(item, qual);
        }
        else {
          tree_warning(tree, loc, "Unknown node name '%s'", s1);
        }
      }
    }
  }
}


void tree_init_load(tree_t *tree)
{
  char *filename;
  FILE *f;

  /* Build init file name from tree file name */
  filename = tree_init_filename(tree);

  if ( (f = fopen(filename, "r")) != NULL ) {
    char *buf = NULL;
    int size = 0;
    int len = 0;
    int finished = 0;
    tree_err_loc_t loc;

    loc.filename = filename;
    loc.lineno = 0;

    while ( ! finished ) {
      char c;

      /* Read character from file */
      if ( read(fileno(f), &c, 1) != 1 ) {
        c = '\n';
        finished = 1;
      }

      /* Grow line buffer if needed */
      if ( len >= (size-1) ) {
        size += 256;
        buf = (char *) realloc(buf, size);
      }

      /* End of the line */
      if ( c == '\n' ) {
        loc.lineno++;
        buf[len] = NUL;

        tree_init_load_line(tree, buf, &loc);

        len = 0;
      }
      /* Within the line: store character */
      else {
        if ( c < ' ' )
          c = ' ';

        buf[len++] = c;
      }
    }

    if ( buf != NULL )
      free(buf);

    fclose(f);
  }

  free(filename);
}


static void tree_init_save_object(tree_object_t *object, FILE *f)
{
  char *name = object->parent_item->name;
  int skip = -1;

  switch ( object->type ) {
  case TYPE_CASE:
    skip = object->flags & FLAG_FORCE_SKIP;
    if ( object->d.Case->breakpoint )
      fprintf(f, "BREAKPOINT %s\n", name);
    break;
  case TYPE_SEQ:
    skip = object->flags & FLAG_FORCE_SKIP;
    break;
  default :
    /* Nothing */
    break;
  }

  if ( skip >= 0 ) {
    int parent_skip = (object->parent_seq != NULL) && (object->parent_seq->flags & FLAG_FORCE_SKIP);

    if ( (skip) && (!parent_skip) )
      fprintf(f, "FORCE_SKIP %s\n", name);
    if ( (!skip) && (parent_skip) )
      fprintf(f, "/FORCE_SKIP %s\n", name);
  }
}


void tree_init_save(tree_t *tree)
{
  char *filename;
  FILE *f;

  /* Build init file name from tree file name */
  filename = tree_init_filename(tree);

  if ( (f = fopen(filename, "w")) != NULL ) {
    long size;

    tree_foreach(tree, (tree_func_t *) tree_init_save_object, f);
    size = ftell(f);
    fclose(f);

    if ( size <= 0 )
      remove(filename);
  }

  free(filename);
}
