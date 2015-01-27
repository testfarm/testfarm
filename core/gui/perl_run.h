/****************************************************************************/
/* TestFarm                                                                 */
/* Test Suite User Interface: Perl process management                       */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 15-MAY-2000                                                    */
/****************************************************************************/

/*
 * $Revision: 398 $
 * $Date: 2007-03-07 12:21:06 +0100 (mer., 07 mars 2007) $
 */

#ifndef __PERL_RUN_H
#define __PERL_RUN_H

#include "child.h"
#include "pipe.h"
#include "output.h"


typedef void perl_run_reply_t(void *arg, int verdict, int criticity, char *tail);

#define PERL_RUN_REPLY_SPAWN_FAILED -1
#define PERL_RUN_REPLY_NOSYSTEM     -2
#define PERL_RUN_REPLY_SYSTEM       -3

typedef struct {
  char *script;
  int script_built;
  char *target_directory;
  char *report_directory;
  char *log_name;

  int ctl_request;
  int ctl_reply;
  int ctl_tag;
  child_t *child;
  int tengine;
  int request_case;
  perl_run_reply_t *reply;
  void *reply_arg;

  output_t *out;
  output_file_func_t *output_func;
  void *output_arg;
  pipe_t *output_stdout;
  pipe_t *output_stderr;
  pipe_t *terminated;
} perl_run_t;

extern perl_run_t *perl_run_init(perl_run_reply_t *reply, void *arg);
extern void perl_run_destroy(perl_run_t *pg);

extern char *perl_run_script(perl_run_t *pg, tree_t *tree, char *target_directory);
extern void perl_run_set_name(perl_run_t *pg, char *treename, char *release);
extern void perl_run_clear(perl_run_t *pg);
extern void perl_run_backup(perl_run_t *pg);
extern int perl_run_spawn(perl_run_t *pg, int debug);
extern void perl_run_kill(perl_run_t *pg);
extern void perl_run_request(perl_run_t *pg, tree_object_t *object, int verdict, char *text);
extern void perl_run_request_input(perl_run_t *pg, int verdict, char *text);
extern void perl_run_request_action(perl_run_t *pg, tree_object_t *object, char *action);

extern void perl_run_output_set(perl_run_t *pg, output_file_func_t *func, void *arg);

#endif /* __PERL_RUN_H */
