/****************************************************************************/
/* TestFarm                                                                 */
/* Test Suite User Interface: Perl process management                       */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 15-MAY-2000                                                    */
/****************************************************************************/

/*
 * $Revision: 772 $
 * $Date: 2007-10-11 15:58:52 +0200 (jeu., 11 oct. 2007) $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <errno.h>
#include <fcntl.h>
#include <glib.h>
#include <sys/wait.h>
#include <sys/stat.h>

#include "install.h"
#include "useful.h"

#include "child.h"
#include "pipe.h"
#include "filelist.h"
#include "output.h"
#include "status.h"
#include "perl_run.h"


#if ! GLIB_CHECK_VERSION(2,0,0)
#define g_path_get_dirname(path) g_dirname(path)
#define g_path_get_basename(path) strdup(g_basename(path))
#endif


/*===========================================*/
/*        Output file management             */
/*===========================================*/

void perl_run_output_set(perl_run_t *pg, output_file_func_t *func, void *arg)
{
  if ( pg == NULL )
    return;
  pg->output_func = func;
  pg->output_arg = arg;
}


static void perl_run_output_stdin_line(perl_run_t *pg, char *str)
{
  /* Store data to output file */
  output_write_stdio(pg->out, OUTPUT_CHANNEL_STDIN, str);

  /* Invoke output line callback */
  if ( pg->output_func != NULL )
    pg->output_func(pg->output_arg, OUTPUT_CHANNEL_STDIN, str);
}


static void perl_run_output_stdin(perl_run_t *pg, int verdict, char *text)
{
  char *p1;

  if ( text == NULL )
    return;

  /* Ignore empty text */
  p1 = strskip_spaces(text);
  if ( p1[0] != '\0' ) {
    /* Dump input text to output file as STDIN channel */
    while ( p1 != NULL ) {
      char *p2 = strchr(p1, '\n');
      if ( p2 != NULL ) {
	*(p2++) = '\0';
	if ( *p2 == '\0' )
	  p2 = NULL;
      }

      perl_run_output_stdin_line(pg, p1);

      p1 = p2;
    }
  }

  /* Dump verdict if any */
  switch ( verdict ) {
  case VERDICT_PASSED:
    perl_run_output_stdin_line(pg, "*** MANUAL VERDICT: PASSED");
    break;
  case VERDICT_FAILED:
    perl_run_output_stdin_line(pg, "*** MANUAL VERDICT: FAILED");
    break;
  case VERDICT_INCONCLUSIVE:
    perl_run_output_stdin_line(pg, "*** MANUAL VERDICT: INCONCLUSIVE");
    break;
  default:
    break;
  }
}


static void perl_run_output_stdout(perl_run_t *pg, char *str)
{
  int channel = OUTPUT_CHANNEL_STDOUT;

  if ( strncmp(str, "IN_", 3) == 0 ) {
    char *id = str + 3;
    if ( strncmp(id, "TITLE", 5) == 0 )
      channel = OUTPUT_CHANNEL_IN_TITLE;
    else if ( strncmp(id, "HEADER", 6) == 0 )
      channel = OUTPUT_CHANNEL_IN_HEADER;
    else if ( strncmp(id, "VERDICT", 7) == 0 )
      channel = OUTPUT_CHANNEL_IN_VERDICT;
  }

  /* Store data to output file */
  output_write_stdio(pg->out, channel, str);

  /* Invoke output line callback */
  if ( pg->output_func != NULL )
    pg->output_func(pg->output_arg, OUTPUT_CHANNEL_STDOUT, str);
}


static void perl_run_output_stderr(perl_run_t *pg, char *str)
{
  /* Store data to output file */
  output_write_stdio(pg->out, OUTPUT_CHANNEL_STDERR, str);

  /* Invoke output line callback */
  if ( pg->output_func != NULL )
    pg->output_func(pg->output_arg, OUTPUT_CHANNEL_STDERR, str);
}


static void perl_run_output_pipe(pipe_t *pipe, perl_run_t *pg)
{
  /* Dump data */
  if ( pipe->flags == 1 )
    perl_run_output_stderr(pg, pipe->buf);
  else
    perl_run_output_stdout(pg, pipe->buf);
}


void perl_run_output_flush(perl_run_t *pg)
{
  /* Flush output pipes */
  if ( pg->output_stdout != NULL )
    pipe_input(pg->output_stdout);
  if ( pg->output_stderr != NULL )
    pipe_input(pg->output_stderr);
}


static int perl_run_output_case(perl_run_t *pg, tree_object_t *object)
{
  int num;

  /* Check object is a Test Case */
  if ( object->type != TYPE_CASE )
    return -1;
  num = object->d.Case->num;

  /* Dump test case header to output file */
  if ( num != pg->request_case ) {
    pg->request_case = num;
    object->d.Case->exec_count++;
    output_write_case(pg->out, object);
  }

  return num;
}


/*===========================================*/
/*        PERL process management            */
/*===========================================*/

static void perl_run_sigchld(int status, perl_run_t *pg)
{
  /*fprintf(stderr, "*DEBUG* PERL process SIGCHLD\n");*/

  /* Continue only if child terminated by itself */
  if ( pg->child == NULL )
    return;
  pg->child = NULL;

  /* Send PERL process termination event */
  if ( pg->terminated != NULL ) {
    char str[80];

    if ( WIFEXITED(status) ) {
      sprintf(str, "*** PERL process exited with code %d\n", WEXITSTATUS(status));
    }
    else if ( WIFSIGNALED(status) ) {
      sprintf(str, "*** PERL process killed with signal %d\n", WTERMSIG(status));
    }
    else if ( WIFSTOPPED(status) ) {
      sprintf(str, "*** PERL process stopped with signal %d\n", WSTOPSIG(status));
    }

    write(pg->terminated->fds[1], str, strlen(str));
  }
}


static void perl_run_terminated(pipe_t *pipe, perl_run_t *pg)
{
  /*fprintf(stderr, "*DEBUG* %s\n", pipe->buf);*/

  /* Dump the reason why PERL process terminated */
  perl_run_output_stderr(pg, "");
  perl_run_output_stderr(pg, pipe->buf);

  /* Shut PERL gears down */
  perl_run_kill(pg);

  /* Reply to client */
  if ( pg->reply != NULL )
    pg->reply(pg->reply_arg, -1, -1, NULL);
}


void perl_run_request(perl_run_t *pg, tree_object_t *object, int verdict, char *text)
{
  int num;
  char buf[20];
  int size;

  if ( pg == NULL )
    return;

  /* Setup Test Case entry */
  pg->request_case = -1;
  num = perl_run_output_case(pg, object);
  if ( num < 0 )
    return;

  /* Dump input text if any */
  perl_run_output_stdin(pg, verdict, text);

  /* Dump verdict if it was simulated */
  if ( verdict >= 0 ) {
    output_write_verdict(pg->out, verdict, -1);
  }

  /* Verify we can communicate with PERL */
  if ( pg->ctl_request < 0 )
    return;

  /* Construct request message */
  size = snprintf(buf, sizeof(buf), "=%d %d\n", num, verdict);

  /* Send request message */
  if ( write(pg->ctl_request, buf, size) < 0 ) {
    perror("write(request)");
    return;
  }
}


void perl_run_request_input(perl_run_t *pg, int verdict, char *text)
{
  char buf[1024];
  int size;

  /* Dump input text if any */
  perl_run_output_stdin(pg, verdict, text);

  /* Verify we can communicate with PERL */
  if ( pg->ctl_request < 0 )
    return;

  /* Construct request message */
  size = snprintf(buf, sizeof(buf), "=INPUT %d %s\n", verdict, (text != NULL) ? text:"");

  /* Send request message (which acts as an input reply) */
  if ( write(pg->ctl_request, buf, size) < 0 ) {
    perror("write(input reply)");
    return;
  }
}


void perl_run_request_action(perl_run_t *pg, tree_object_t *object, char *action)
{
  char lf = '\n';

  //fprintf(stderr, "ACTION: '%s'\n", action);

  /* Setup Test Case entry */
  perl_run_output_case(pg, object);

  /* Verify we can communicate with PERL */
  if ( pg->ctl_request < 0 )
    return;

  if ( write(pg->ctl_request, action, strlen(action)) < 0 ) {
    perror("write(manual action)");
    return;
  }

  write(pg->ctl_request, &lf, 1);
}


static void perl_run_reply(perl_run_t *pg, int fd, GdkInputCondition condition)
{
  char buf[256];
  int size;
  char *sv, *sc, *tail;
  int verdict, criticity;

  /* Get reply from PERL process */
  size = read(fd, buf, sizeof(buf)-1);
  if ( size < 0 ) {
    perror("read(reply)");
    return;
  }
  buf[size] = '\0';

  /* Get verdict and criticity values from reply */
  sv = strskip_spaces(buf);
  sc = strskip_chars(sv);
  if ( *sc != '\0' )
    *(sc++) = '\0';
  sc = strskip_spaces(sc);
  tail = strskip_chars(sc);
  if ( *tail != '\0' )
    *(tail++) = '\0';
  tail = strskip_spaces(tail);
  if ( *tail != '\0' ) {
    int len = strlen(tail) - 1;
    while ( (len >= 0) && (tail[len] < ' ') )
      len--;
    tail[len+1] = '\0';
  }
  if ( *tail == '\0' ) {
    tail = NULL;
  }

  verdict = (*sv != '\0') ? atoi(sv) : -1;
  criticity = (*sc != '\0') ? atoi(sc) : -1;

  /* Flush PERL process stdout/stderr pipes */
  perl_run_output_flush(pg);

  /* If verdict value is positive, this is a Test Case verdict,
     so we dump it to output file */
  if ( verdict >= 0 ) {
    if ( verdict < VERDICT_MAX )
      output_write_verdict(pg->out, verdict, criticity);
  }
  /* If verdict value is negative, this means that the Test Suite just started,
     and the criticity field contains the pid of the Test Engine process */
  else {
    pg->tengine = criticity;
  }

  /* Reply to client */
  if ( pg->reply != NULL )
    pg->reply(pg->reply_arg, verdict, criticity, tail);
}


static int perl_run_build(perl_run_t *pg)
{
  int ret = 0;
  FILE *f;

  /* Build PERL script */
  if ( (f = fopen(pg->script, "w")) != NULL ) {
    tree_t *tree = pg->out->tree.tree;
    code_build(f, tree);
    fclose(f);
  }
  else {
    fprintf(stderr, "Cannot create PERL program file %s: %s\n", pg->script, strerror(errno));
    ret = -1;
  }

  return ret;
}


int perl_run_spawn(perl_run_t *pg, int debug)
{
  char *argv[] = {"perl", "-w", NULL, NULL, NULL, NULL, NULL, NULL, NULL};
  int argc = 2;
  char *perldb = NULL;
  int ctl_request[2], ctl_reply[2];
  char str_request[10], str_reply[10];
  int ret;

  if ( pg->script == NULL )
    return -1;
  if ( pg->child != NULL )
    return -1;

  /* Setup the request control pipe */
  if ( pipe(ctl_request) ) {
    perror("pipe(request)");
    return -1;
  }

  fcntl(ctl_request[0], F_SETFD, 0);
  fcntl(ctl_request[1], F_SETFD, FD_CLOEXEC);
  snprintf(str_request, sizeof(str_request), "%d", ctl_request[0]);

  /* Setup the reply control pipe */
  if ( pipe(ctl_reply) ) {
    perror("pipe(reply)");
    close(ctl_request[0]);
    close(ctl_request[1]);
    return -1;
  }

  fcntl(ctl_reply[0], F_SETFD, FD_CLOEXEC);
  fcntl(ctl_reply[1], F_SETFD, 0);
  snprintf(str_reply, sizeof(str_reply), "%d", ctl_reply[1]);

  /* Create target directory */
  ret = mkdir(pg->target_directory, 0755);
  if ( ret ) {
    if ( errno == EEXIST )
      ret = 0;
    else
      fprintf(stderr, "Cannot create target directory %s: %s\n", pg->target_directory, strerror(errno));
  }

  /* Create report directory */
  if ( pg->report_directory != NULL ) {
    ret = mkdir(pg->report_directory, 0755);
    if ( ret ) {
      if ( errno == EEXIST )
	ret = 0;
      else
	fprintf(stderr, "Cannot create report directory %s: %s\n", pg->report_directory, strerror(errno));
    }
  }
  else {
    fprintf(stderr, "*PANIC* Unknown report directory name\n");
    ret = -1;
  }

  /* Build PERL program (if not already done) */
  if ( (ret == 0) && (pg->script_built == 0) ) {
    ret = perl_run_build(pg);
  }

  if ( ret ) {
    close(ctl_request[0]);
    close(ctl_request[1]);
    close(ctl_reply[0]);
    close(ctl_reply[1]);
    return -1;
  }

  pg->script_built = 1;

  /* Start new output file */
  output_write_open(pg->out);

  /* Open stdout pipe */
  if ( pg->output_stdout != NULL )
    pipe_done(pg->output_stdout);
  pg->output_stdout = pipe_init(0);
  pipe_notify(pg->output_stdout, (pipe_notify_t *) perl_run_output_pipe, pg);

  /* Open stderr pipe */
  if ( pg->output_stderr != NULL )
    pipe_done(pg->output_stderr);
  pg->output_stderr = pipe_init(1);
  pipe_notify(pg->output_stderr, (pipe_notify_t *) perl_run_output_pipe, pg);

  /* Setup debug mode if enabled */
  if ( debug ) {
    char *module = get_perldb();
    perldb = (char *) malloc(strlen(module)+4);
    strcpy(perldb, "-d");

    if ( module[0] != '\0' ) {
      perldb[2] = ':';
      strcpy(&(perldb[3]), module);
    }

    argv[argc++] = perldb;
  }

  /* Setup PERL arguments */
  argv[argc++] = pg->script;
  argv[argc++] = pg->report_directory;
  argv[argc++] = pg->log_name;
  argv[argc++] = str_request;
  argv[argc++] = str_reply;
  argv[argc] = NULL;

  /* Spawn PERL process */
  pg->child = child_spawn(argv, -1, pipe_fd(pg->output_stdout), pipe_fd(pg->output_stderr),
                          (child_handler_t *) perl_run_sigchld, pg);

  /* Set useful control pipe endpoints. Close unused control pipe endpoints. */
  close(ctl_request[0]);
  pg->ctl_request = ctl_request[1];
  close(ctl_reply[1]);
  pg->ctl_reply = ctl_reply[0];

  /* Abort if PERL process spawn failed */
  if ( pg->child == NULL ) {
    perl_run_kill(pg);
    return -1;
  }

  /* Setup reply control pipe handling */
  pg->ctl_tag = gdk_input_add(ctl_reply[0], GDK_INPUT_READ,
                              (GdkInputFunction) perl_run_reply, pg);

  /* Close unused stdio pipe endpoints */
  close(pg->output_stdout->fds[1]);
  pg->output_stdout->fds[1] = -1;

  close(pg->output_stderr->fds[1]);
  pg->output_stderr->fds[1] = -1;

  /* Free PERLDB option if any */
  if ( perldb != NULL )
    free(perldb);

  return 0;
}


static int perl_run_wait_tengine(perl_run_t *pg, int timeout)
{
  struct sigaction act;
  struct sigaction oldact;
  pid_t pid;
  int ret = 0;

  if ( pg->tengine <= 0 )
    return 0;

  /* Setup SIGALRM signal handling */
  sigemptyset(&act.sa_mask);
  act.sa_flags = 0;
  act.sa_handler = SIG_IGN;
  sigaction(SIGALRM, &act, &oldact);
  alarm(timeout);

  /* Wait for tengine termination */
  pid = waitpid(pg->tengine, NULL, 0);
  if ( (pid == -1) && (errno == EINTR) )
    ret = -1;

  /* Restore SIGALRM signal handling */
  alarm(0);
  sigaction(SIGALRM, &oldact, NULL);

  return ret;
}


static void perl_run_kill_tengine(perl_run_t *pg)
{
  if ( pg->tengine <= 0 )
    return;

  if ( perl_run_wait_tengine(pg, 2) ) {
    fprintf(stderr, "Test Engine process %d termination is late: sending SIGTERM signal.\n", pg->tengine);

    /* Terminate tengine */
    kill(pg->tengine, SIGTERM);

    /* Wait for tengine termination */
    if ( perl_run_wait_tengine(pg, 2) ) {
      fprintf(stderr, "Test Engine process %d termination failed: killing it.\n", pg->tengine);
      kill(pg->tengine, SIGKILL);
    }
  }

  pg->tengine = -1;
}


void perl_run_kill(perl_run_t *pg)
{
  child_t *child = pg->child;
  pg->child = NULL;

  /* Disable reply input checker */
  if ( pg->ctl_tag != -1 )
    gdk_input_remove(pg->ctl_tag);
  pg->ctl_tag = -1;

  /* Close request/reply control pipes */
  if ( pg->ctl_request >= -1 ) {
    close(pg->ctl_request);
    usleep(10000);
  }
  pg->ctl_request = -1;

  if ( pg->ctl_reply >= -1 )
    close(pg->ctl_reply);
  pg->ctl_reply = -1;

  /* Terminate PERL process */
  child_terminate(child);

  /* Close stdout/stderr pipes */
  if ( pg->output_stdout != NULL ) {
    pipe_input(pg->output_stdout);
    pipe_done(pg->output_stdout);
    pg->output_stdout = NULL;
  }

  if ( pg->output_stderr != NULL ) {
    pipe_input(pg->output_stderr);
    pipe_done(pg->output_stderr);
    pg->output_stderr = NULL;
  }

  /* Terminate output file */
  output_write_close(pg->out);
  pg->request_case = -1;

  /* Terminate Test Engine process */
  perl_run_kill_tengine(pg);

  /* Merge log file to output file */
  output_merge(pg->out, pg->log_name);
}


static void perl_run_remove(char **pname)
{
  if ( *pname == NULL )
    return;

  remove(*pname);
  free(*pname);
  *pname = NULL;
}


static void perl_run_clear_name(perl_run_t *pg)
{
  /* Free log file name */
  if ( pg->log_name != NULL ) {
    free(pg->log_name);
    pg->log_name = NULL;
  }
}


void perl_run_set_name(perl_run_t *pg, char *treename, char *release)
{
  int size;
  char *p;

  if ( pg == NULL )
    return;

  /* Clear old name settings */
  perl_run_clear_name(pg);

  if ( treename == NULL )
    return;

  /* Compute max size of dir/file name buffers */
  size = strlen(pg->target_directory) + strlen(treename) + 16;
  if ( release != NULL )
    size += (1 + strlen(release));

  /* Construct report directory name */
  pg->report_directory = p = (char *) malloc(size);
  p += snprintf(pg->report_directory, size, "%s" G_DIR_SEPARATOR_S "%s",
		pg->target_directory, treename);
  if ( release != NULL )
    p += sprintf(p, ".%s", release);

  /* Set global log file name */
  pg->log_name = (char *) malloc(size);
  snprintf(pg->log_name, size, "%s" G_DIR_SEPARATOR_S "global.log", pg->report_directory);

  /* Set output file name */
  p = (char *) malloc(size);
  snprintf(p, size, "%s" G_DIR_SEPARATOR_S "output.xml", pg->report_directory);
  output_set_filename(pg->out, p);
  free(p);
}


void perl_run_clear(perl_run_t *pg)
{
  if ( pg == NULL )
    return;

  /* Remove PERL script */
  perl_run_remove(&(pg->script));
  pg->script_built = 0;

  /* Clear name settings */
  perl_run_clear_name(pg);

  /* Free output file manager */
  if ( pg->out != NULL ) {
    output_destroy(pg->out);
    pg->out = NULL;
  }

  /* Clear target directory */
  if ( pg->target_directory != NULL ) {
    free(pg->target_directory);
    pg->target_directory = NULL;
  }

  /* Clear report directory */
  if ( pg->report_directory != NULL ) {
    free(pg->report_directory);
    pg->report_directory = NULL;
  }
}


char *perl_run_script(perl_run_t *pg, tree_t *tree, char *target_directory)
{
  char *treename;

  if ( pg == NULL )
    return NULL;

  /* Destroy old settings */
  perl_run_clear(pg);

  /* Set test tree descriptor */
  if ( tree == NULL )
    return NULL;
  treename = tree->head->name;

  /* Set target directory */
  pg->target_directory = strdup(target_directory);

  /* Make PERL script name */
  pg->script = (char *) malloc(strlen(pg->target_directory) + strlen(treename) + 5);
  sprintf(pg->script, "%s/%s.pl", pg->target_directory, treename);
  pg->script_built = 0;

  /* Init output file manager */
  pg->out = output_alloc(tree);

  /* Set output file names */
  perl_run_set_name(pg, treename, NULL);

  return pg->script;
}


static void perl_run_backup_highest(char *fname, int *pnum)
{
  char *p;
  int num;

  p = strrchr(fname, '.');
  if ( p == NULL )
    return;
  *(p++) = '\0';

  num = atoi(p);
  if ( num > *pnum )
    *pnum = num;
}


void perl_run_backup(perl_run_t *pg)
{
  int size;
  char *out_backup;
  char *out_suffix;
  GList *list;
  int num;

  if ( pg == NULL )
    return;

  if ( pg->report_directory == NULL )
    return;

  /* Check if a backup is needed */
  if ( access(pg->report_directory, F_OK) )
    return;

  /* Alloc output backup name buffer */
  size = strlen(pg->report_directory) + 20;
  out_backup = (char *) malloc(size);
  out_suffix = out_backup + snprintf(out_backup, size, "%s", pg->report_directory);

  /* Build the list of existing backups */
  strcpy(out_suffix, "\\.[0123456789]+$");
  list = filelist(out_backup);

  /* Get the greatest backup number */
  num = 0;
  g_list_foreach(list, (GFunc) perl_run_backup_highest, &num);

  /* Free the list of existing backups */
  g_list_foreach(list, (GFunc) free, NULL);
  g_list_free(list);

  /* Set the new report backup name */
  sprintf(out_suffix, ".%d", num+1);

  /* Rename report directory, and annouce it on the status bar */
  if ( rename(pg->report_directory, out_backup) == 0 ) {
    char *old_name = g_path_get_basename(pg->report_directory);
    char *new_name = g_path_get_basename(out_backup);

    status_mesg("Test Report backup: %s -> %s", old_name, new_name);

    free(old_name);
    free(new_name);
  }

  /* Clear output file indexing */
  output_write_clear(pg->out);

  /* Free log backup name buffer */
  free(out_backup);

  /* Remove the Log File */
  if ( pg->log_name != NULL )
    remove(pg->log_name);
}


perl_run_t *perl_run_init(perl_run_reply_t *reply, void *arg)
{
  perl_run_t *pg = (perl_run_t *) malloc(sizeof(perl_run_t));

  pg->script = NULL;
  pg->script_built = 0;
  pg->log_name = NULL;
  pg->target_directory = NULL;
  pg->report_directory = NULL;

  pg->ctl_request = -1;
  pg->ctl_reply = -1;
  pg->ctl_tag = -1;
  pg->child = NULL;
  pg->tengine = -1;
  pg->request_case = -1;
  pg->reply = reply;
  pg->reply_arg = arg;

  pg->out = NULL;
  pg->output_func = NULL;
  pg->output_arg = NULL;
  pg->output_stdout = NULL;
  pg->output_stderr = NULL;

  /* Open termination event pipe */
  pg->terminated = pipe_init(-1);
  fcntl(pg->terminated->fds[0], F_SETFD, FD_CLOEXEC);
  fcntl(pg->terminated->fds[1], F_SETFD, FD_CLOEXEC);
  pipe_notify(pg->terminated, (pipe_notify_t *) perl_run_terminated, pg);

  return pg;
}


void perl_run_destroy(perl_run_t *pg)
{
  if ( pg == NULL )
    return;

  /* Close termination event pipe */
  if ( pg->terminated != NULL ) {
    pipe_done(pg->terminated);
    pg->terminated = pg->terminated;
  }

  /* Kill running process */
  perl_run_output_set(pg, NULL, NULL);
  perl_run_kill(pg);

  /* Free PERL script name */
  perl_run_clear(pg);

  free(pg);
}
