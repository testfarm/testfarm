/****************************************************************************/
/* TestFarm                                                                 */
/* Test Suite code generator : Test Tree building from directory scan       */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 11-MAY-2004                                                    */
/****************************************************************************/

/*
 * $Revision: 761 $
 * $Date: 2007-10-01 14:13:31 +0200 (lun., 01 oct. 2007) $
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <malloc.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <glib.h>

#include "useful.h"
#include "codegen_script.h"
#include "codegen_tree.h"
#include "codegen_tree_error.h"
#include "codegen_tree_precond.h"
#include "codegen_tree_scan.h"


#define debug(_fmt_,_args_...) //fprintf(stderr,_fmt_,_args_)


typedef struct {
  tree_t *tree;
  tree_seq_t *seq;
  tree_err_loc_t loc;
  char *dirname;
} tree_scan_data_t;


static void tree_scan_directive(tree_scan_data_t *p, char *str)
{
  tree_t *tree = p->tree;
  char *arg;

  arg = strskip_chars(str);
  if ( *arg != '\0' )
    *(arg++) = '\0';

  debug("%s:%d: [DIRECTIVE] '%s' arg='%s'\n", p->loc.filename, p->loc.lineno, str, arg);

  if ( strcmp(str, "DESCRIPTION") == 0 ) {
    if ( tree->current->comment != NULL )
      free(tree->current->comment);
    tree->current->comment = strdup(arg);
  }
  else if ( strcmp(str, "REFERENCE") == 0 ) {
    if ( tree->current->reference != NULL )
      free(tree->current->reference);
    tree->current->reference = strdup(arg);
  }
  else if ( strcmp(str, "SYSTEM") == 0 ) {
    if ( tree->current == tree->head ) {
      if ( tree->system != NULL ) {
        tree_error(tree, &(p->loc), "Multiple SYSTEM specification is not allowed");
      }
      else {
        *strskip_chars(arg) = '\0';
        tree->system = strdup(arg);
      }
    }
    else {
      tree_warning(tree, &(p->loc), "Directive #$%s ignored: must be placed in the Tree Root file", str);
    }
  }
  else if ( strcmp(str, "PRECONDITION") == 0 ) {
    if ( tree->current->object->type == TYPE_CASE ) {
      tree_case_t *tcase = tree->current->object->d.Case;

      if ( tree_precond_new(tcase, arg, &(p->loc)) )
        tree_error(tree, &(p->loc), "CASE '%s': Syntax error in precondition", tree->current->name);
    }
    else {
      tree_warning(tree, &(p->loc), "Directive #$%s ignored: must be placed in a Test Script", str);
    }
  }
  else if ( strcmp(str, "CRITICITY") == 0 ) {
    if ( tree->current->object->type == TYPE_CASE ) {
      tree_case_t *tcase = tree->current->object->d.Case;

      if ( arg[0] != NUL ) {
        if ( tcase->criticity == CRITICITY_NONE ) {
          char *s1 = strskip_spaces(arg);
          char *s2 = strskip_chars(s1);

          if ( *s2 != NUL )
            *(s2++) = NUL;

          tcase->criticity = criticity_level(strupper(s1));

          if ( tcase->criticity == CRITICITY_UNKNOWN ) {
            tree_error(tree, &(p->loc), "CASE '%s': Unknown criticity identifier '%s'", tree->current->name, arg);
            tcase->criticity = CRITICITY_NONE;
          }
        }
        else {
          tree_error(tree, &(p->loc), "CASE '%s': Multiple definition of criticity", tree->current->name);
        }
      }
      else {
        tree_error(tree, &(p->loc), "CASE '%s': Missing criticity identifier", tree->current->name);
      }
    }
    else {
      tree_warning(tree, &(p->loc), "Directive #$%s ignored: must be placed in a Test Script", str);
    }
  }
  else if ( strcmp(str, "BREAK_IF_FAILED") == 0 ) {
    if ( tree->current->object->type == TYPE_CASE ) {
      tree->current->object->flags |= FLAG_BREAK_IF_FAILED;
    }
    else {
      tree_warning(tree, &(p->loc), "Directive #$%s ignored: must be placed in a Test Script", str);
    }
  }
  else if ( strcmp(str, "ABORT_IF_FAILED") == 0 ) {
    if ( tree->current->object->type == TYPE_CASE ) {
      tree->current->object->flags |= FLAG_ABORT_IF_FAILED;
    }
    else {
      tree_warning(tree, &(p->loc), "Directive #$%s ignored: must be placed in a Test Script", str);
    }
  }
  else {
    tree_warning(tree, &(p->loc), "Unknown directive #$%s ignored", str);
  }
}


static char *tree_scan_node_name(tree_scan_data_t *p, char *str)
{
  char *dirname = p->dirname;
  char *name;
  int len;
  char *s;
  int i;

  if ( dirname[0] == '.' )
    dirname++;
  if ( dirname[0] == G_DIR_SEPARATOR )
    dirname++;

  /* Compute name length */
  len = strlen(str)+2;

  s = dirname;
  while ( *s != '\0' ) {
    if ( *s == G_DIR_SEPARATOR )
      len++;
    len++;
    s++;
  }

  /* Alloc name buffer */
  name = (char *) malloc(len+1);

  /* Fill name buffer */
  i = 0;

  s = dirname;
  while ( *s != '\0' ) {
    if ( *s == G_DIR_SEPARATOR ) {
      name[i++] = ':';
      name[i++] = ':';
    }
    else if ( ! isalnum(*s) ) {
      name[i++] = '_';
    }
    else {
      name[i++] = *s;
    }
    s++;
  }

  if ( i > 0 ) {
    name[i++] = ':';
    name[i++] = ':';
  }

  strcpy(name+i, str);

  return name;
}


static int tree_scan_file(tree_t *tree, char *name, char *filename);

static int tree_scan_branch(tree_scan_data_t *p, char *str)
{
  char *filename;
  int len;
  struct stat stat1;
  int ret = -1;

  /* Find out the Tree Branch file name */
  filename = (char *) malloc(strlen(p->dirname)+strlen(str)+strlen(p->tree->head->name)+10);
  len = sprintf(filename, "%s" G_DIR_SEPARATOR_S "%s", p->dirname, str);

  if ( (stat(filename, &stat1) == 0) && (S_ISDIR(stat1.st_mode)) ) {
    sprintf(filename+len, G_DIR_SEPARATOR_S "%s.tree", p->tree->head->name);
    if ( access(filename, R_OK) ) {
      sprintf(filename+len, G_DIR_SEPARATOR_S ".tree");
      if ( access(filename, R_OK) ) {
	filename[len] = '\0';
	tree_warning(p->tree, &(p->loc), "No Tree Branch file found in directory \"%s\"", filename);
	len = 0;
      }
    }

    /* Setup node name */
    if ( len > 0 ) {
      char *name = tree_scan_node_name(p, str);
      tree_item_t *item;

      debug("%s:%d: [DIRECTORY] '%s' file='%s' name='%s'\n", p->loc.filename, p->loc.lineno, str, filename, name);

      /* Build Tree Branch node */
      if ( tree_scan_file(p->tree, name, filename) == 0 ) {
	/* Add new SEQ reference to current sequence */
	item = tree_item_new(name, NULL, p->tree, NULL, &(p->loc));
	tree_seq_feed(p->seq, item);
      }

      free(name);
    }

    ret = 0;
  }

  free(filename);
  return ret;
}


static int tree_scan_script(tree_scan_data_t *p, char *str)
{
  tree_t *tree = p->tree;
  char filename[strlen(p->dirname)+strlen(str)+10];
  char *name;
  script_t *script;
  tree_item_t *item;
  tree_item_t *save_current;
  tree_object_t *obj;
  FILE *f;

  /* Setup PERL script name */
  snprintf(filename, sizeof(filename), "%s" G_DIR_SEPARATOR_S "%s" SCRIPT_PERL_SUFFIX, p->dirname, str);
  if ( access(filename, R_OK) ) {
    tree_error(tree, &(p->loc), "Couldn't access script file '%s': %s", filename, strerror(errno));
    return -1;
  }

  /* Save current node pointer */
  save_current = tree->current;

  /* Compute Test Case identification */
  name = tree_scan_node_name(p, str);
  script = script_new(filename);

  debug("%s:%d: [SCRIPT   ] '%s' script='%s'\n", p->loc.filename, p->loc.lineno, name, script->name);

  /* Add new CASE reference to current sequence */
  item = tree_item_new(name, NULL, tree, NULL, &(p->loc));
  tree_seq_feed(p->seq, item);

  /* Create new CASE node */
  obj = tree_object_new(TYPE_CASE);
  obj->d.Case->script = script;
  tree->current = tree_item_new(name, NULL, tree, obj, &(p->loc));
  tree_add(tree, tree->current);

  /* Check a package definition is present */
  if ( script->package == NULL )
    tree_error(tree, &(p->loc), "CASE '%s': Missing package definition in script file '%s'", tree->current->name, script->name);

  free(name);

  /* Parse Test Script directives */
  name = (script->wizname != NULL) ? script->wizname : script->name;
  if ( (f = fopen(name, "r")) != NULL ) {
    char *buf = NULL;
    int size = 0;
    tree_scan_data_t d;

    d.tree = tree;
    d.seq = p->seq;
    d.loc.filename = name;
    d.loc.lineno = 0;
    d.dirname = p->dirname;

    while ( !feof(f) ) {
      int len = fgets2(f, &buf, &size);

      d.loc.lineno++;

      if ( len > 0 ) {
        char *s = g_strstrip(buf);
        if ( (s[0] == '#') && (s[1] == '$') ) {
          tree_scan_directive(&d, s+2);
        }
      }
    }

    if ( buf != NULL )
      free(buf);

    fclose(f);
  }

  /* Restore current node pointer */
  tree->current = save_current;

  return 0;
}


static void tree_scan_line(tree_scan_data_t *p, char *str)
{
  if ( str[0] == '#' ) {
    if ( str[1] == '$' ) {
      tree_scan_directive(p, str+2);
    }
  }
  else {
    char *tail;

    /* Split line after first space character (if any) */
    tail = strskip_chars(str);
    if ( *tail != NUL ) {
      *(tail++) = NUL;
      tail = strskip_spaces(tail);
    }

    /* Check for sub-directory or script. Report error if none found */
    if ( tree_scan_branch(p, str) )
      if ( tree_scan_script(p, str) )
	tree_error(p->tree, &(p->loc), "Cannot find a valid object \"%s\" in directory %s", str, p->dirname);
  }
}


static int tree_scan_file(tree_t *tree, char *name, char *filename)
{
  FILE *f;
  char buf[BUFSIZ];
  char *buf_dirname;
  tree_item_t *save_current;
  tree_object_t *obj;
  tree_scan_data_t d;
  int ret = 0;

  /* Retrieve cannonical file name */
  if ( (filename[0] == '.') && (filename[1] == G_DIR_SEPARATOR) )
    filename += 2;

  /* Open Tree definition file */
  debug("SCAN TREE FILE %s\n", filename);
  if ( (f = fopen(filename, "r")) == NULL )
    return -1;

  /* Save current node pointer */
  save_current = tree->current;

  /* Setup scan iterator */
  d.tree = tree;
  d.seq = NULL;
  d.loc.filename = filename;
  d.loc.lineno = 0;
  d.dirname = buf_dirname = g_path_get_dirname(filename);
  debug(">>>>> OPEN TREE FILE %s (dirname=%s)\n", d.loc.filename, d.dirname);

  /* Create new SEQuence object */
  obj = tree_object_new(TYPE_SEQ);
  tree->current = tree_item_new(name, NULL, tree, obj, &(d.loc));
  tree_add(tree, tree->current);
  d.seq = obj->d.Seq;

  /* If no sequence was defined before, assume it is the tree Tree Root  */
  if ( tree->head == NULL )
    tree->head = tree->current;

  /* Parse tree file */
  while ( ! feof(f) ) {
    if ( fgets(buf, BUFSIZ, f) != NULL ) {
      char *str = g_strstrip(buf);

      d.loc.lineno++;

      if ( *str != '\0' )
        tree_scan_line(&d, str);
    }
  }

  free(buf_dirname);
  fclose(f);
  debug("<<<<< CLOSE TREE FILE %s\n", d.loc.filename);

  /* Check branch emptyness */
  if ( d.seq->nodes == NULL ) {
    tree_warning(d.tree, &(d.loc), "SEQ '%s' ignored: branch file is empty", name);
    tree_remove(tree, tree->current);
    tree_item_destroy(tree->current);
    tree_object_destroy(obj);
    ret = -1;
  }

  /* Restore current node pointer */
  tree->current = save_current;

  return ret;
}


int tree_scan_root(tree_t *tree, char *filename)
{
  char *base_name;
  char *dir_name;
  char *name;
  char *s;
  int ret;

  debug("SCAN ROOT %s\n", filename);

  /* Set working directory to Tree root directory */
  dir_name = g_path_get_dirname(filename);
  chdir(dir_name);

  /* Get Test Tree head */
  base_name = g_path_get_basename(filename);
  name = strdup(base_name);
  if ( (s = strchr(name, '.')) != NULL )
    *s = '\0';

  /* Build Tree Root node */
  ret = tree_scan_file(tree, name, base_name);

  free(name);
  free(base_name);
  free(dir_name);

  return ret;
}
