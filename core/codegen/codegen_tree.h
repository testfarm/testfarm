/****************************************************************************/
/* TestFarm                                                                 */
/* Test Suite code generator : test tree                                    */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 12-APR-2000                                                    */
/****************************************************************************/

/*
 * $Revision: 763 $
 * $Date: 2007-10-01 17:45:56 +0200 (lun., 01 oct. 2007) $
 */

#ifndef __CODEGEN_TREE_H
#define __CODEGEN_TREE_H

#include <time.h>
#include <glib.h>


#define TREE_DEF_SUFFIX ".tree"


/*******************************************************/
/* Tree items                                           */
/*******************************************************/

typedef struct tree_s tree_t;
typedef struct tree_item_s tree_item_t;
typedef struct tree_object_s tree_object_t;


/*================= ERROR MESSAGES =========================*/

typedef enum {
  TREE_ERR_INFO=0,
  TREE_ERR_WARNING,
  TREE_ERR_ERROR,
  TREE_ERR_PANIC,
  TREE_ERR_MAX
} tree_err_level_t;

typedef struct {
  char *filename;
  int lineno;
} tree_err_loc_t;

typedef struct {
  tree_err_loc_t loc;      /* The file the error comes from */
  tree_err_level_t level;  /* The error level */
  tree_object_t *object;   /* The object the error is related to */
  char *msg;               /* The error message */
} tree_err_t;


/*================= TEST CASE =========================*/

#include "codegen_script.h"
#include "codegen_criticity.h"

typedef enum {
  VERDICT_UNEXECUTED=-1,
  VERDICT_PASSED=0,
  VERDICT_FAILED,
  VERDICT_INCONCLUSIVE,
  VERDICT_SKIP,
  VERDICT_EXECUTED,
  VERDICT_MAX
} tree_verdict_t;


typedef struct {
  tree_verdict_t verdict;
  criticity_t criticity;
} tree_result_t;


typedef struct tree_case_s tree_case_t;

struct tree_case_s {
  int num;
  GList *preconds;
  int loop;
  criticity_t criticity;
  script_t *script;

  int breakpoint;
  tree_verdict_t simu;
  tree_result_t result;
  unsigned long exec_count;
};


/*================= TEST SEQUENCE =========================*/

typedef struct {
  int nnodes;
  tree_item_t **nodes;
} tree_seq_t;

extern void tree_seq_feed(tree_seq_t *tseq, tree_item_t *item);


/*================= TREE OBJECT =========================*/

typedef enum {
  TYPE_NULL,
  TYPE_CASE,
  TYPE_SEQ,
} tree_object_type_t;

#define FLAG_BREAK_IF_FAILED 1
#define FLAG_ABORT_IF_FAILED 2
#define FLAG_FORCE_SKIP      4

struct tree_object_s {
  tree_object_type_t type;
  union {
    tree_case_t *Case;
    tree_seq_t *Seq;
    void *ptr;
  } d;
  unsigned long key;
  tree_item_t *parent_item;
  tree_object_t *parent_seq;
  tree_object_t *end_jump;
  tree_object_t *break_jump;
  int enter;
  GHashTable *data_hash;
  unsigned int flags;
};

typedef void tree_func_t(tree_object_t *tobject, void *arg);

extern tree_object_t *tree_object_new(tree_object_type_t type);
extern void tree_object_destroy(tree_object_t *tobject);

extern tree_object_t *tree_object_find(tree_object_t *tobject);
extern int tree_object_has_seq(tree_object_t *tobject);
extern int tree_object_force_skip(tree_object_t *tobject, int state);
extern void tree_object_foreach(tree_object_t *tobject, tree_func_t *func, void *arg);

extern void tree_object_set_data(tree_object_t *tobject, char *key, void *ptr, void (*unref)(void *));
extern void tree_object_unset_data(tree_object_t *tobject, char *key);
extern void *tree_object_get_data(tree_object_t *tobject, char *key);

extern tree_object_t *tree_object_executed(tree_object_t *object, tree_verdict_t verdict, int criticity);
extern int tree_object_precond(tree_object_t *object);


/*================= TREE ITEM =========================*/

struct tree_item_s {
  char *name;
  char *comment;
  char *reference;
  tree_object_t *object;
  tree_t *parent_tree;
  int links;
  tree_err_loc_t loc;
};

extern tree_item_t *tree_item_new(char *name, char *comment, tree_t *tree, tree_object_t *object,
                                  tree_err_loc_t *loc);
extern void tree_item_destroy(tree_item_t *item);


/*================= TREE =========================*/

struct tree_s {
  tree_err_loc_t loc;
  int errcount;
  int warncount;
  GList *errlist;

  char *system;

  int keycount;
  int casecount;
  int nmemb;
  time_t t0;
  tree_item_t **items;
  tree_item_t *head;
  tree_item_t *current;

  tree_object_t **stack;
  int stack_size;
  int stack_index;
};

extern void tree_foreach(tree_t *tree, tree_func_t *func, void *arg);

extern void tree_count(tree_t *tree, int *_ncase, int *_nscenario);

extern tree_item_t *tree_add(tree_t *tree, tree_item_t *item);
extern void tree_remove(tree_t *tree, tree_item_t *item);
extern tree_item_t *tree_retrieve(tree_t *tree, char *name);
extern tree_t *tree_new(void);
extern void tree_destroy(tree_t *tree, int save_cfg);
extern tree_t *tree_load(tree_t *tree, char *filename);
extern void tree_simu(tree_t *tree);

extern tree_object_t *tree_lookup(tree_t *tree, unsigned long key);
extern tree_object_t *tree_lookup_name(tree_t *tree, char *name);

#endif /* __CODEGEN_TREE_H */
