/****************************************************************************/
/* TestFarm                                                                 */
/* Shell Script Interpreter Library - Standard Shell Commands               */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 26-AUG-1999                                                    */
/****************************************************************************/

/*
 * $Revision: 1219 $
 * $Date: 2011-12-04 16:41:16 +0100 (dim., 04 déc. 2011) $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "useful.h"
#include "shell.h"

#ifdef ENABLE_CMD_ALIAS
#include "shell_alias.h"
#endif

#ifdef ENABLE_CMD_HISTORY
#include "shell_history.h"
#endif


/*=================================================*/
/* Output header                                   */
/*=================================================*/

char *shell_std_header(char *tag)
{
  static char buf[80];

  /* Display local timestamp and tag */
  sprintf(buf, "%s %s ", strtimestamp(), tag);

  return buf;
}


static shell_std_header_t *shell_std_header_proc = shell_std_header;


shell_std_header_t *shell_std_set_header(shell_std_header_t *proc)
{
  shell_std_header_t *previous = shell_std_header_proc;
  shell_std_header_proc = (proc != NULL) ? proc : shell_std_header;
  return previous;
}


/*=================================================*/
/* Input echo                                      */
/*=================================================*/

int shell_std_input_echo(shell_t *shell, shell_argv_t *cmd_argv, char *tag)
{
  /* Assume default TAG if none specified */
  if ( tag == NULL )
    tag = SHELL_STD_INPUT_ECHO_TAG;

  fputs(shell_std_header_proc(tag), stdout);
  shell_argv_fprintf(stdout, cmd_argv, 0);
  printf("\n");
  return 0;
}


/*=================================================*/
/* in-line help management                         */
/*=================================================*/

static shell_std_help_t shell_std_help_std[] = {
  { "echo",    "echo <text>\n  Insert <text> into output log\n"},
  { "argv",    "argv\n  Show arguments list\n"},
#ifdef ENABLE_CMD_SOURCE
  { "source",  "source <command-file> [<argument-list>]\n"
               "  Source command file\n"
#ifdef ENABLE_GLIB
               "source -pause|-resume|-stop|-wait\n"
               "  Control currently active source execution\n"
#endif
  },
#endif
#ifdef ENABLE_CMD_ALIAS
  { "alias",   "alias <keyword> <command> [<args> ...]\n  Define a new command alias\nalias [<keyword>]\n  Show alias(es)\n"},
#endif
#ifdef ENABLE_CMD_HISTORY
  { "history", "history\n  Show history of last " STRINGIFY(SHELL_HISTORY_MAX) " commands\n" },
#endif
  { "exit",    "exit\n  Exit from shell\n" },
  { "!",       "!<OS-command>\n  Execute OS-shell command\n" },
  { NULL,     NULL }
};


static int shell_std_help_usr_size = 0;
static shell_std_help_t **shell_std_help_usr = NULL;


static void shell_std_help_scan(shell_std_help_t *hlp, char *keyword)
{
  while ( hlp->keyword != NULL ) {
    if ( (keyword == NULL) || (strcmp(keyword, hlp->keyword) == 0) )
      printf("%s", hlp->help);
    hlp++;
  }
}


void shell_std_set_help(shell_std_help_t *help)
{
  int i;

  /* Retrieve free help entry */
  i = 0;
  while ( (i < shell_std_help_usr_size) && (shell_std_help_usr[i] != NULL) )
    i++;

  /* Allocate a new help entry if needed */
  if ( i >= shell_std_help_usr_size ) {
    shell_std_help_usr_size++;
    shell_std_help_usr = (shell_std_help_t **) realloc(shell_std_help_usr, sizeof(shell_std_help_t *) * shell_std_help_usr_size);
  }

  /* Setup the new help entry */
  shell_std_help_usr[i] = help;
}


void shell_std_unset_help(shell_std_help_t *help)
{
  int i;

  /* Retrieve the help entry */
  i = 0;
  while ( (i < shell_std_help_usr_size) && (shell_std_help_usr[i] != help) )
    i++;

  /* Cancel the help entry */
  if ( i < shell_std_help_usr_size ) {
    shell_std_help_usr[i] = NULL;
  }
}


void shell_std_help(shell_t *shell, char *keyword)
{
  int i;

  if ( ! shell->interactive )
    return;

  /* Display help title */
  if ( keyword != NULL )
    printf("COMMAND USAGE:\n");
  else
    printf("AVAILABLE COMMANDS:\n");

  /* Display help text */
  shell_std_help_scan(shell_std_help_std, keyword);

  for (i = 0; i < shell_std_help_usr_size; i++) {
    if ( shell_std_help_usr[i] != NULL )
      shell_std_help_scan(shell_std_help_usr[i], keyword);
  }
}



/*=================================================*/
/* echo                                            */
/*=================================================*/

static int shell_std_echo(shell_t *shell, shell_argv_t *cmd_argv, char *tag)
{
  int argc = cmd_argv->argc;
  char **argv = cmd_argv->argv;
  int i;

  /* Display report header */
  fputs(shell_std_header_proc(tag), stdout);

  /* Display argument list */
  for (i = 1; i < argc; i++)
    printf("%s ", argv[i]);
  printf("\n");

  return 0;
}


/*=================================================*/
/* argv                                            */
/*=================================================*/

static int shell_std_argv(shell_t *shell, shell_argv_t *cmd_argv, char *tag)
{
  /* Check arguments */
  if ( cmd_argv->argc != 1 ) {
    shell_std_help(shell, cmd_argv->argv[0]);
    return -1;
  }

  /* Display report header */
  fputs(shell_std_header_proc(tag), stdout);

  /* Display argument list */
  shell_argv_fprintf(stdout, shell->shell_argv, 1);
  printf("\n");

  return 0;
}


/*=================================================*/
/* source                                          */
/*=================================================*/

#ifdef ENABLE_CMD_SOURCE

static int shell_std_source(shell_t *shell, shell_argv_t *cmd_argv, char *tag)
{
  int argc = cmd_argv->argc;
  char **argv = cmd_argv->argv;

  /* Check argument list */
  if ( argc < 2 ) {
    shell_std_help(shell, argv[0]);
    return -1;
  }

  /* Source subshell */
  return shell_source(shell, argc-1, argv+1);
}

#endif


/*=================================================*/
/* alias                                           */
/*=================================================*/

#ifdef ENABLE_CMD_ALIAS

static int shell_std_alias_show(shell_alias_t *alias, char *keyword, char *hdr)
{
  if ( alias == NULL )
    return 0;
  if ( alias->tab == NULL )
    return 0;

  if ( keyword == NULL )
    shell_alias_show(alias, hdr);
  else
    shell_alias_item_show(shell_alias_retrieve(alias, keyword), hdr);

  return 0;
}


static int shell_std_alias(shell_t *shell, shell_argv_t *cmd_argv, char *tag)
{
  char *hdr = shell_std_header_proc(tag);
  int argc = cmd_argv->argc;
  char **argv = cmd_argv->argv;
  shell_alias_item_t *item;

  /* Show aliases if no definition supplied */
  if ( argc <= 2 )
    return shell_std_alias_show(shell->shell_alias, argv[1], hdr);

  /* Add alias */
  item = shell_alias_add(shell->shell_alias, argv[1], argc-2, argv+2);

  /* Add alias and report what has been done */
  shell_alias_item_show(item, hdr);

  return 0;
}

#endif


/*=================================================*/
/* Command history                                 */
/*=================================================*/

#ifdef ENABLE_CMD_HISTORY

static void shell_std_history_item(shell_history_item_t *item, void *data)
{
	char *hdr = data;

	printf("%s%d : %s : %d\n", hdr, item->id, item->str, item->ret);
}

static int shell_std_history(shell_t *shell, shell_argv_t *cmd_argv, char *tag)
{
	char *hdr = shell_std_header_proc(tag);
	shell_history_foreach(&shell->history, shell_std_history_item, hdr);
	return 0;
}

#endif


/*=================================================*/
/* OS-shell command invocation                     */
/*=================================================*/

int shell_std_system(shell_t *shell, shell_argv_t *cmd_argv)
{
  char *cmd = NULL;
  int len = 0;
  int i;

  /* Echo command */
  if ( shell->input_echo_handler != NULL )
    shell->input_echo_handler(shell, cmd_argv, shell->input_echo_tag);

  /* Build command line */
  for (i = 0; i < cmd_argv->argc; i++) {
    char *cp = cmd_argv->argv[i];
    int cl = strlen(cp);

    if ( i == 0 ) {
      cp++;
      cl--;
    }

    cmd = realloc(cmd, len + cl + 2);
    memcpy(cmd+len, cp, cl);
    len += cl;

    cmd[len++] = ' ';
  }

  cmd[len] = NUL;

  /* Invoke system */
  system(cmd);

  /* Free command buffer */
  if ( cmd != NULL )
    free(cmd);

  return 0;
}


/*=================================================*/
/* unknown command                                 */
/*=================================================*/

int shell_std_unknown(shell_t *shell, shell_argv_t *cmd_argv, char *tag)
{
  char **argv = cmd_argv->argv;

  /* OS-shell command ? */
  if ( argv[0][0] == '!' )
    return shell_std_system(shell, cmd_argv);

  /* Ooops... A bad command was entered */
  if ( ! shell->interactive ) {
    shell_error(shell, "Unknown command '%s'\n", argv[0]);
    return -1;
  }

  shell_std_help(shell, NULL);
  return 0;
}


/*=================================================*/
/* Standard commands setup                         */
/*=================================================*/

static shell_command_t shell_std_tab[] = {
  { "echo",    shell_std_echo,    "ECHO   " },
  { "argv",    shell_std_argv,    "ARGV   " },
#ifdef ENABLE_CMD_SOURCE
  { "source",  shell_std_source,  "SOURCE " },
#endif
#ifdef ENABLE_CMD_ALIAS
  { "alias",   shell_std_alias,   "ALIAS  " },
#endif
#ifdef ENABLE_CMD_HISTORY
  { "history", shell_std_history, "HISTORY ", 1 },
#endif
  { "exit",    NULL,              "EXIT   ", 1 },
  { NULL,      shell_std_unknown, NULL }
};


int shell_std_setup(shell_t *shell, shell_std_help_t *help)
{

  /* Setup descriptors and procedures */
  shell_std_set_help(help);

  /* Setup standard commands */
  shell_set_cmd(shell, shell_std_tab);

  return 0;
}


shell_handler_t *shell_std_set_unknown(shell_handler_t *proc)
{
  shell_command_t *entry = shell_std_tab;
  shell_handler_t *previous;

  while ( entry->keyword != NULL )
    entry++;
  previous = entry->handler;
  entry->handler = (proc != NULL) ? proc : shell_std_unknown;

  return previous;

}


shell_handler_t *shell_std_set_echo(shell_handler_t *proc)
{
  shell_handler_t *previous = shell_std_tab[0].handler;
  shell_std_tab[0].handler = proc;
  return previous;
}
