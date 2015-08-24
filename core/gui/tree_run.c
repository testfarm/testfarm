/****************************************************************************/
/* TestFarm                                                                 */
/* Graphical User Interface : Test Tree execution                           */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 12-APR-2000                                                    */
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
#include <stdlib.h>
#include <string.h>
#include <malloc.h>

#include "child.h"

#include "install.h"
#include "options.h"
#include "support.h"
#include "report_config.h"
#include "perl_run.h"
#include "status.h"
#include "check_gui.h"
#include "tree_gui.h"
#include "tcpsrv.h"
#include "tree_run.h"


/*===========================================*/
/* Test Case output display                  */
/*===========================================*/

static void tree_run_output_feed(tree_run_t *tg, int channel, char *str)
{
  tree_object_t *object;

  /* Retrieve current output descriptor */
  if ( tg->pg == NULL )
    return;
  if ( tg->pg->out == NULL )
    return;

  if ( tg->gui != NULL ) {
    /* Retrieve the current output object */
    object = tree_lookup_name(tg->tree, output_get_current_node_name(tg->pg->out));

    /* Update output display */
    if ( object != NULL ) {
      tree_gui_output(tg->gui, object->key, channel, str);
    }
  }
  else {
    fputs(str, stdout);
    fputs("\n", stdout);
  }
}


static void tree_run_output(tree_run_t *tg, int channel, char *str)
{
  output_write_stdio(tg->pg->out, channel, str);
  tree_run_output_feed(tg, channel, str);
}


/*===========================================*/
/*        Perl debug mode                    */
/*===========================================*/

int tree_run_set_debug(tree_run_t *tg, int state)
{
  if ( state >= 0 )
    tg->debug = state;
  return tg->debug;
}


/*===========================================*/
/*         Breakpoints                       */
/*===========================================*/

int tree_run_set_brkpt(tree_run_t *tg, unsigned long key, int state)
{
  tree_object_t *object = tree_lookup(tg->tree, key);

  if ( object == NULL )
    return -1;
  if ( object->type != TYPE_CASE )
    return -1;

  if ( state >= 0 )
    object->d.Case->breakpoint = state;
  if ( object->d.Case->breakpoint != 0 )
    object->d.Case->breakpoint = 1;

  return object->d.Case->breakpoint;
}


/*===========================================*/
/*         ForceSkip                         */
/*===========================================*/

int tree_run_set_skip(tree_run_t *tg, unsigned long key, int state)
{
  tree_object_t *object = tree_lookup(tg->tree, key);

  if ( object == NULL )
    return -1;

  return tree_object_force_skip(object, state);
}


/*===========================================*/
/*         Execution state                   */
/*===========================================*/

const char *state_info[] = {
  "Reset",
  "Spawn",
  "Break",
  "Run",
  "Input",
  "Finished",
};


static void tree_run_unset(tree_run_t *tg)
{
  /* No current node */
  tree_gui_set_current(tg->gui, 0);
}


static void tree_run_set(tree_run_t *tg, tree_object_t *object)
{
  /* Set new node */
  tg->current_object = object;
  if ( object == NULL )
    object = tg->tree->head->object;

  /* Show current row */
  tree_gui_set_current(tg->gui, object->key);
}


static void tree_run_state(tree_run_t *tg, int state)
{
  char buf[32];

  /* Display state */
  tree_gui_set_state(tg->gui, state);

  /* Inform parent process */
  if ( opt_command ) {
    if ( (state != stRUN) || (tg->state != stRUN) ) {
      report_progress_msg(&(tg->progress), (char *) state_info[state]);
    }
  }

  /* Inform remote service client */
  snprintf(buf, sizeof(buf), "S%d %s", state, (char *) state_info[state]);
  srv_write(&tg->srv, buf);

  /* Update execution state */
  tg->state = state;
}


static void tree_run_finished(tree_run_t *tg)
{
  tg->current_object = NULL;

  /* Set and Show state */
  tree_run_state(tg, stFINISHED);

  /* Report status message */
  status_mesg("Test suite '%s' finished", tg->tree->head->name);

  /* Kill PERL process */
  perl_run_kill(tg->pg);

  /* Automatically exit if option enabled */
  if ( opt_quit ) {
    if ( tg->gui != NULL )
      tree_gui_quit(tg->gui);
    else
      exit(0);
  }
}


static void tree_run_clear(tree_run_t *tg)
{
  /* Reset execution state */
  tree_run_state(tg, stRESET);
  tg->current_object = NULL;
  tg->stepping = 0;
  tg->conf = REPORT_CONFIG_DEFAULT;
}


/*===========================================*/
/*         Verdicts                          */
/*===========================================*/

static void tree_run_clear_verdicts_object(tree_object_t *object, tree_run_t *tg)
{
  if ( object->type == TYPE_CASE ) {
    object->d.Case->result.verdict = VERDICT_UNEXECUTED;
    object->d.Case->result.criticity = CRITICITY_NONE;
    tree_gui_verdict(tg->gui, object->key, &(object->d.Case->result));
  }
}


static void tree_run_clear_verdicts(tree_run_t *tg)
{
  tree_foreach(tg->tree, (tree_func_t *) tree_run_clear_verdicts_object, tg);
}


static int tree_run_verdict(tree_run_t *tg, tree_verdict_t verdict, int criticity)
{
  tree_object_t *object;
  tree_object_t *branch;
  char buf[32];

  object = tg->current_object;
  if ( object == NULL )
    return 1;

  /* Set verdict, get next execution branch */
  branch = tree_object_executed(object, verdict, criticity);

  /* Show verdict and execution progress */
  if ( object->type == TYPE_CASE ) {
    tree_gui_verdict(tg->gui, object->key, &(object->d.Case->result));
  }

  /* Report execution progress */
  {
    output_stat_t *stat = &(tg->pg->out->stat);
    report_progress_value(&(tg->progress), ((stat->executed * 100) / stat->ncase));
  }

  /* Send verdict to remote service (if connected) */
  snprintf(buf, sizeof(buf), "V%d %d", verdict, criticity);
  srv_write(&tg->srv, buf);

  /* Go to next node if not in Static Step mode */
  if ( tg->stepping < 2 ) {
    if ( branch != NULL )
      tree_run_set(tg, branch);
    else
      tree_run_unset(tg);
  }
  else {
    branch = object;
  }

  return (branch == NULL);
}


/*===========================================*/
/*         Abort                             */
/*===========================================*/

static void tree_run_abort_guts(tree_run_t *tg)
{
  /* Output INCONCLUSIVE verdict for aborted test case */
  if ( (tg->state == stRUN) || (tg->state == stINPUT) ) {
    output_write_verdict(tg->pg->out, VERDICT_INCONCLUSIVE, -1);
  }

  /* Kill PERL interpreter */
  perl_run_kill(tg->pg);

  switch ( tg->state ) {
  case stSPAWN:
    break;
  case stRUN:
  case stINPUT:
    tree_run_verdict(tg, VERDICT_INCONCLUSIVE, -1);
    tree_run_unset(tg);
    break;
  default:
    break;
  }

  tree_run_finished(tg);

  /* Report status message */
  status_mesg("Test suite '%s' execution aborted", tg->tree->head->name);

  /* Automatically exit if option enabled */
  if ( opt_quit ) {
    tree_gui_quit(tg->gui);
  }
}


void tree_run_abort(tree_run_t *tg)
{
  if ( (tg->state == stRUN) || (tg->state == stINPUT) ) {
    tree_run_output(tg, OUTPUT_CHANNEL_STDERR, "*** USER ABORT ***");
  }

  tree_run_abort_guts(tg);
}


/*===========================================*/
/*         Reset                             */
/*===========================================*/

static void tree_run_reset_case(tree_object_t *object)
{
  if ( object->type == TYPE_CASE ) {
    if ( object->d.Case->breakpoint != 0 )
      object->d.Case->breakpoint = 1;
    object->d.Case->exec_count = 0;
  }
}


static void tree_run_reset_guts(tree_run_t *tg)
{
  /* Clear execution index */
  tg->stepping = 0;
  tree_run_set(tg, NULL);
  tree_foreach(tg->tree, (tree_func_t *) tree_run_reset_case, NULL);

  /* Set and Show state */
  tree_run_state(tg, stRESET);

  /* Warn if script initialization went wrong */
  if ( tg->pg->script == NULL )
    status_mesg("PERL execution disabled because of initialization failure");

  /* Report status message */
  status_mesg("Test suite '%s' reset", tg->tree->head->name);

  /* Clear verdicts */
  tree_run_clear_verdicts(tg);

  /* Clear operator name */
  output_set_operator(tg->pg->out, NULL);
}


void tree_run_reset(tree_run_t *tg)
{
  /* Kill PERL interpreter */
  perl_run_kill(tg->pg);

  /* Clear output storage & display */
  tree_run_reset_guts(tg);
}


static void tree_run_reset_srv(tree_run_t *tg)
{
  tree_run_unset(tg);
  tree_run_abort_guts(tg);
  tree_run_reset_guts(tg);
}


static void tree_run_wait(tree_run_t *tg)
{
  /* Set and Show state */
  tree_run_state(tg, stWAIT);
}


static void tree_run_go(tree_run_t *tg);


static void tree_run_next(tree_run_t *tg)
{
  if ( tg->stepping ) {
    tg->stepping = 0;
    tree_run_wait(tg);
  }
  else {
    tree_run_go(tg);
  }
}


static void tree_run_continue(tree_run_t *tg, tree_verdict_t verdict, int criticity)
{
  int finished;

  if ( tg->current_object == NULL )
    return;

  finished = tree_run_verdict(tg, verdict, criticity);

  if ( finished ) {
    tree_run_finished(tg);
  }
  else {
    tree_run_next(tg);
  }
}


static void tree_run_simulate(tree_run_t *tg, tree_verdict_t verdict, char *text)
{
  tree_object_t *object;

  object = tg->current_object;
  if ( (object == NULL) || (object->type != TYPE_CASE) )
    return;

  /* Send execution request to PERL for verdict update */
  perl_run_request(tg->pg, object, verdict, text);

  tree_run_continue(tg, verdict, -1);
}


static void tree_run_perl(tree_run_t *tg)
{
  status_mesg("Spawning PERL process");

  /* Set tree in SPAWN state */
  tree_run_state(tg, stSPAWN);

  /* Spawn PERL interpreter */
  if ( perl_run_spawn(tg->pg, tg->debug) ) {
    status_mesg("Failed to spawn PERL process");
    tree_run_reset_guts(tg);
  }
  else {
    status_mesg("Starting Test Suite");
  }
}


static tree_object_t *tree_run_object(tree_run_t *tg)
{
  tree_object_t *object;

  if ( tg->current_object == NULL ) {
    /* Retrieve first test case to be executed */
    tree_item_t *item = tg->tree->head;
    while ( (item != NULL) && (item->object->type == TYPE_SEQ) )
      item = item->object->d.Seq->nodes[0];

    object = (item != NULL) ? item->object : NULL;
  }
  else {
    object = tg->current_object;
  }

  if ( object != NULL )
    tree_run_set(tg, object);

  return object;
}


static void tree_run_go_straight(tree_run_t *tg, tree_object_t *object)
{
  /* Compute preconditions, and emmit a SKIP verdict if they are FALSE */
  if ( tree_object_precond(object) ) {
    /* Request for execution if we are in PERL mode */
    status_mesg("Executing Test Case '%s'", object->parent_item->name);
    tree_run_state(tg, stRUN);

    /* Send execution request to PERL for execution */
    if ( object->d.Case->breakpoint )
      perl_run_request(tg->pg, object, -2, NULL);
    else
      perl_run_request(tg->pg, object, -1, NULL);
  }
  else {
    tree_run_simulate(tg, VERDICT_SKIP, NULL);
  }
}


static void tree_run_go(tree_run_t *tg)
{
  /* Find out first available test case */
  tree_object_t *object = tree_run_object(tg);

  /* If no subsequent test case, we are finished */
  if ( object == NULL ) {
    tree_run_finished(tg);
    return;
  }

  /* If we encounter a breakpoint, process it */
  if ( object->d.Case->breakpoint == 1 ) {
    status_mesg("Breakpoint encountered on test case '%s'", object->parent_item->name);
    object->d.Case->breakpoint = 2;

    /* If we are in PERL debug mode, let the debugger process the breakpoint by itself */
    if ( tg->debug )
      tree_run_go_straight(tg, object);
    else
      tree_run_wait(tg);
  }

  /* If no breakpoint, go straight */
  else {
    tree_run_go_straight(tg, object);
  }
}


void tree_run_input(tree_run_t *tg, int verdict, char *text)
{
  switch ( tg->state ) {
  case stINPUT:
    tree_run_state(tg, stRUN);
    perl_run_request_input(tg->pg, verdict, text);
    break;
  case stWAIT:
    tg->stepping = 1;
    tree_run_simulate(tg, verdict, text);
    break;
  default:
    fprintf(stderr, "*PANIC* Input Reply occured in illegal state (%s)\n", state_info[tg->state]);
    break;
  }
}


static void tree_run_input_request(tree_run_t *tg, char *text)
{
  tree_object_t *object;

  if ( tg->state != stRUN ) {
    fprintf(stderr, "*PANIC* Input Request in illegal state (%s)\n", state_info[tg->state]);
    return;
  }

  object = tg->current_object;
  if ( (object != NULL) && (object->type == TYPE_CASE) ) {
    tree_run_state(tg, stINPUT);

    /* Dump prompt text to STDOUT */
    if ( text != NULL ) {
      tree_run_output(tg, OUTPUT_CHANNEL_STDOUT, text);
    }
  }
  else {
    fprintf(stderr, "*PANIC* Input Request from unknown Test Case\n");
    perl_run_request_input(tg->pg, -1, NULL);
  }
}


static void tree_run_reply(tree_run_t *tg, int verdict, int criticity, char *tail)
{
  /* Test case execution result */
  if ( verdict >= 0 ) {
    /* Test case verdict */
    if ( verdict < VERDICT_MAX ) {
      if ( (tg->current_object != NULL) && (tg->current_object->type == TYPE_CASE) )
	status_mesg("Test Case '%s' done", tg->current_object->parent_item->name);

      tree_run_continue(tg, verdict, criticity);
    }
    /* Test case input request */
    else {
      tree_run_input_request(tg, tail);
    }
  }

  /* Spawn operation failed */
  else if ( verdict == PERL_RUN_REPLY_SPAWN_FAILED ) {
    status_mesg("PERL execution process aborted itself");
    tree_run_abort_guts(tg);
  }

  /* Spawn operation succeeded (no system definition, deprecated) */
  else if ( verdict == PERL_RUN_REPLY_NOSYSTEM ) {
    tree_run_object(tg);
    tree_run_go(tg); /* No START() procedure present => go directly to first test case */
  }

  /* Spawn operation succeeded (with system definition) */
  else if ( verdict == PERL_RUN_REPLY_SYSTEM ) {
    status_mesg("Test Suite initialization done");
    tree_run_object(tg);
    tree_run_next(tg);
  }
}


static void tree_run_start_0(tree_run_t *tg)
{
  /* Setup properties if in RESET state*/
  if ( tg->state == stRESET ) {
    char *operator = tg->prop->operator;

    if ( operator == NULL ) {
      operator = "(UNKNOWN)";
    }

    output_set_operator(tg->pg->out, operator);
    output_set_release(tg->pg->out, tg->prop->release);

    perl_run_set_name(tg->pg, tg->tree->head->name, tg->prop->release);
    tree_gui_set_report(tg->gui, tg->pg->log_name, tg->pg->report_directory);

    /* Backup output file if needed */
    perl_run_backup(tg->pg);

    /* Spawn PERL process */
    /* tree_run_go() will be called after the PERL process is ready */
    tree_run_perl(tg);
  }

  /* Go straight if execution in progress */
  else {
    tree_run_go(tg);
  }
}


static void tree_run_start_1(tree_run_t *tg)
{
  report_config_t *rc;

  /* Fetch default Report Configuration */
  rc = report_config_alloc();
  report_config_load(rc, NULL);
  tg->conf = rc->conf;
  report_config_destroy(rc);

  /* Get operator name if required and not already done */
  if ( (tg->gui != NULL) && (! opt_go) &&
       (tg->state == stRESET) &&
       (tg->conf & REPORT_CONFIG_OPERATOR) ) {
    properties_check(tg->prop, "Operator Name is required\nfor launching the Test Suite",
		     (void *) tree_run_start_0, tg);
  }
  else {
    tree_run_start_0(tg);
  }
}


void tree_run_start(tree_run_t *tg, int step)
{
  int start = 1;

  /* Set stepping mode */
  tg->stepping = step;

  /* Burst a dialog window if check list is not empty in interactive mode */
  if ( (tg->gui != NULL) && (!opt_go) &&
       (tg->state == stRESET) ) {
    GList *list = tree_check(tg->tree);

    if ( list != NULL ) {
      check_gui_files(list,
		      (check_gui_spot_t *) tree_gui_spot, tg->gui,
		      (check_gui_done_t *) tree_run_start_1, tg);
      g_list_free(list);
      start = 0;
    }
  }

  if ( start )
    tree_run_start_1(tg);
}


void tree_run_manual_action(tree_run_t *tg, char *action)
{
  perl_run_request_action(tg->pg, tg->current_object, action);
}


/*===========================================*/
/* Main Test Suite GUI                       */
/*===========================================*/

static void tree_run_unload(tree_run_t *tg)
{
  if ( tg == NULL )
    return;

  /* Kill running PERL process (if any) */
  perl_run_kill(tg->pg);

  /* Remove PERL script */
  perl_run_clear(tg->pg);

  /* Clear Tree View */
  tree_gui_unload(tg->gui);

  /* Destroy tree */
  if ( tg->tree != NULL ) {
    tree_destroy(tg->tree, 1);
    tg->tree = NULL;
  }

  /* Clear execution pointers */
  tree_run_clear(tg);
}


tree_t *tree_run_load(tree_run_t *tg, char *filename)
{
  tree_t *tree;

  if ( tg == NULL )
    return NULL;

  /* Ensure the tree is empty */
  tree_run_unload(tg);

  /* Disable System Management buttons */
  properties_set(tg->prop, NULL, NULL);

  tree = tree_new();
  if ( tree_load(tree, filename) == NULL ) {
    tree_destroy(tree, 0);
    return NULL;
  }
  tg->tree = tree;

  if ( tree->errcount == 0 ) {
    char *target_directory;

    /* Build PERL script */
    if ( (opt_target == NULL) || (access(opt_target, W_OK) != 0) ) {
      char *cwd = g_get_current_dir();
      target_directory = (char *) malloc(strlen(cwd)+8);
      sprintf(target_directory, "%s/report", cwd);
      free(cwd);
    }
    else {
      target_directory = strdup(opt_target);
    }

    perl_run_script(tg->pg, tree, target_directory);

    free(target_directory);

    /* Set report name, so that the next reset operation
       performs a report backup if needed */
    perl_run_set_name(tg->pg, tg->tree->head->name, tg->prop->release);

    /* Build Tree View */
    tree_gui_load(tg->gui, tree, tg->pg->out);

    /* Reset execution state */
    tree_run_reset_guts(tg);

    /* Init report stuffs */
    tree_gui_set_report(tg->gui, tg->pg->log_name, tg->pg->report_directory);

    /* Activate Service start/stop buttons */
    if ( tree->system != NULL ) {
      properties_set(tg->prop, tree->system, tree->head->name);
    }

    /* Report operation to status banner */
    if ( tree->warncount == 0 )
      status_mesg("Test Tree \"%s\" loaded", filename);
    else
      status_mesg("Test Tree \"%s\" loaded (%d warning%s)", filename,
		  tree->warncount, (tree->warncount > 1) ? "s":"");

    /* Launch execution if --execute option is enabled */
    if ( opt_go )
      tree_run_start_1(tg);
  }
  else {
    status_mesg("%d error%s found in Test Tree file \"%s\"",
		tree->errcount, (tree->errcount > 1) ? "s":"", filename);
  }

  return tree;
}


int tree_run_loaded(tree_run_t *tg)
{
  if ( tg == NULL )
    return 0;
  if ( tg->tree == NULL )
    return 0;

  return (tg->tree->errcount == 0);
}


int tree_run_jump(tree_run_t *tg, unsigned long key)
{
  tree_object_t *object;
  int ret = -1;

  object = tree_lookup(tg->tree, key);
  if ( (object != NULL) && (object->type == TYPE_CASE) ) {
    tree_run_set(tg, object);
    ret = 0;
  }

  return ret;
}


static int tree_run_jump_name(tree_run_t *tg, char *nodename)
{
  tree_object_t *object;
  int ret = -1;

  object = tree_lookup_name(tg->tree, nodename);
  if ( (object != NULL) && (object->type == TYPE_CASE) ) {
    tree_run_set(tg, object);
    ret = 0;
  }

  return ret;
}


static void tree_run_srv_handler(tree_run_t *tg, srv_io_t io, char *str)
{
  if ( io == SRV_IO_DATA ) {
    int ret;
    char s_ret[16];

    switch ( str[0] ) {
    case 'R':  /* RESET */
      tree_run_reset_srv(tg);
      ret = 0;
      break;
    case 'J':  /* JUMP */
      ret = tree_run_jump_name(tg, str+1);
      break;
    case 'G':  /* GO */
      tg->stepping = 0;
      tree_run_start_0(tg);
      ret = 0;
      break;
    case 'S':  /* STEP */
      tg->stepping = 1;
      tree_run_start_0(tg);
      ret = 0;
      break;
    case 'X':  /* EXEC */
      ret = tree_run_jump_name(tg, str+1);
      if ( ret == 0 ) {
	tg->stepping = 2;
	tree_run_start_0(tg);
      }
      break;
    default: /* Ignore other commands */
      ret = 2;
      break;
    }

    snprintf(s_ret, sizeof(s_ret), "R%d", ret);
    srv_write(&tg->srv, s_ret);
  }
}


tree_run_t *tree_run_init(GtkWidget *window)
{
  tree_run_t *tg = (tree_run_t *) malloc(sizeof(tree_run_t));

  /* Clear Test Tree descriptor */
  tg->tree = NULL;

  /* Clear remote service descriptor */
  srv_init(&tg->srv);

  /* Setup PERL execution */
  tg->debug = 0;
  tg->pg = perl_run_init((perl_run_reply_t *) tree_run_reply, tg);

  /* Setup session properties */
  tg->prop = properties_init(window);

  /* Setup GUI and action handlers */
  tg->gui = tree_gui_init(window, tg->prop, tg);

  /* Setup PERL stdio handling */
  perl_run_output_set(tg->pg, (output_file_func_t *) tree_run_output_feed, tg);

  /* Setup periodic progress reporting */
  report_progress_init(&(tg->progress));

  /* Clear execution pointers */
  tree_run_clear(tg);

  /* Setup remote service management */
  if ( opt_service > 0 ) {
    srv_listen(&tg->srv, opt_service, SRV_PROTO_UDP,
	       (srv_func_t *) tree_run_srv_handler, tg);
  }

  return tg;
}


void tree_run_destroy(tree_run_t *tg)
{
  /* Stop remote progress reporting */
  report_progress_clear(&(tg->progress));

  /* Close remote service connection */
  srv_shutdown(&tg->srv);

  /* Clear selections */
  tg->current_object = NULL;

  /* Destroy PERL gears */
  perl_run_destroy(tg->pg);
  tg->pg = NULL;

  /* Destroy GUI */
  tree_gui_destroy(tg->gui);
  tg->gui = NULL;

  /* Destroy properties */
  properties_destroy(tg->prop);
  tg->prop = NULL;

  /* Destroy tree */
  if ( tg->tree != NULL ) {
    tree_destroy(tg->tree, 1);
    tg->tree = NULL;
  }

  /* Free all remaining data */
  free(tg);
}
