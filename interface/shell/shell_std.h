/****************************************************************************/
/* TestFarm                                                                 */
/* Shell Script Interpreter Library - Standard Shell Commands               */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 26-AUG-1999                                                    */
/****************************************************************************/

/*
 * $Revision: 479 $
 * $Date: 2007-04-16 14:44:00 +0200 (lun., 16 avril 2007) $
 */

#ifndef __SHELL_STD_H__
#define __SHELL_STD_H__

/* Output header */
typedef char *shell_std_header_t(char *tag);
extern char *shell_std_header(char *tag);
extern shell_std_header_t *shell_std_set_header(shell_std_header_t *proc);


/* In-line help */
typedef struct {
  char *keyword;
  char *help;
} shell_std_help_t;

extern void shell_std_help(shell_t *shell, char *keyword);

/* OS-shell command invocation */
extern int shell_std_system(shell_t *shell, shell_argv_t *cmd_argv);

/* Standard sleep command */
extern int shell_std_sleep(shell_t *shell, shell_argv_t *cmd_argv, char *tag);

/* Unknown command processing */
extern int shell_std_unknown(shell_t *shell, shell_argv_t *cmd_argv, char *tag);
extern shell_handler_t *shell_std_set_unknown(shell_handler_t *proc);

/* The 'echo' command processing */
extern shell_handler_t *shell_std_set_echo(shell_handler_t *proc);

/* Input echo dump routine */
#define SHELL_STD_INPUT_ECHO_TAG "PROMPT "
extern int shell_std_input_echo(shell_t *shell, shell_argv_t *cmd_argv, char *tag);

/* Help messages */
extern void shell_std_set_help(shell_std_help_t *help);
extern void shell_std_unset_help(shell_std_help_t *help);

/* Global setup for standard shell management */
extern int shell_std_setup(shell_t *shell, shell_std_help_t *help);

#endif /* __SHELL_STD_H__ */
