/****************************************************************************/
/* TestFarm                                                                 */
/* Link management commands                                                 */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 25-APR-2000                                                    */
/****************************************************************************/

/*
 * $Revision: 734 $
 * $Date: 2007-09-22 21:46:53 +0200 (sam., 22 sept. 2007) $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "useful.h"
#include "shell.h"

#include "log.h"
#include "link.h"


/*******************************************************/
/* connect                                             */
/* Subprocess connection                               */
/*******************************************************/

static int link_cmd_connect(shell_t *shell, shell_argv_t *cmd_argv, char *tag)
{
  int argc = cmd_argv->argc;
  char **argv = cmd_argv->argv;
  char *hdr = log_hdr_(tag);
  char *errmsg = NULL;
  char *id;
  link_item_t *link;

  /* Report all connections */
  if ( argc == 1 ) {
    link_scan((link_func_t *) link_info, hdr);
    return 0;
  }

  /* Retrieve link */
  id = strupper(argv[1]);
  link = link_retrieve(id);

  /* Check for link */
  if ( link == NULL ) {
    log_error(tag, "%s: unknown link", id);
    return -1;
  }

  /* Make new connection */
  if ( argc > 2 ) {
    /* Perform subprocess connection */
    if ( link_connect(link, argv+2, &errmsg) < 0 ) {
      log_error(tag, "%s: %s", id, errmsg);
      return -1;
    }
  }

  /* Report connection */
  link_info(link, hdr);

  return 0;
}


/*******************************************************/
/* kill                                                */
/* Subprocess kill                                     */
/*******************************************************/

static int link_cmd_kill(shell_t *shell, shell_argv_t *cmd_argv, char *tag)
{
  int argc = cmd_argv->argc;
  char **argv = cmd_argv->argv;
  int i;

  /* Check arguments */
  if ( argc < 2 ) {
    log_error(tag, "Too few arguments");
    shell_std_help(shell, argv[0]);
    return -1;
  }

  /* Kill subprocess(es) */
  for (i = 1; i < argc; i++) {
    link_item_t *link = link_retrieve(strupper(argv[i]));
    if ( link != NULL ) {
      /* Report explicit kill operation */
      link_info(link, log_hdr_(tag));
      link_kill(link);
    }
    else {
      log_error(tag, "%s: unknown link", argv[i]);
    }
  }

  return 0;
}


/*******************************************************/
/* send                                                */
/* Send out-of-band data to subprocess                 */
/*******************************************************/

static int link_cmd_send(shell_t *shell, shell_argv_t *cmd_argv, char *tag)
{
  int argc = cmd_argv->argc;
  char **argv = cmd_argv->argv;
  link_item_t *link;
  int fd;

  /* Check arguments */
  if ( argc < 3 ) {
    log_error(tag, "Too few arguments");
    shell_std_help(shell, argv[0]);
    return -1;
  }

  /* Retrieve link */
  link = link_retrieve(strupper(argv[1]));
  if ( link == NULL ) {
    log_error(tag, "%s: unknown link", argv[1]);
    return 0;
  }

  /* Send data */
  fd = link->fd_out;
  if ( (link_src_desc.send_data != NULL) && (fd >= 0) ) {
    void *buf = NULL;
    int size;

    size = link_src_desc.send_data(&(link->src), argv+2, &buf);
    if ( buf != NULL ) {
      if ( size > 0 )
	write(fd, buf, size);
      free(buf);
    }
  }

  return 0;
}


/*******************************************************/
/* link                                                */
/* Subprocess - dataflow link                          */
/*******************************************************/

static int link_cmd_link(shell_t *shell, shell_argv_t *cmd_argv, char *tag)
{
  int argc = cmd_argv->argc;
  char **argv = cmd_argv->argv;
  int argx = 1;
  char *hdr = log_hdr_(tag);
  char *id;
  link_item_t *link;

  /* Report all links */
  if ( argc == 1 ) {
    link_scan((link_func_t *) link_report, hdr);
    return 0;
  }

  /* Retrieve link entry */
  id = strupper(argv[argx++]);
  link = link_retrieve(id);

  /* Spawn new subprocess */
  if ( argx < argc ) {
    /* Get output format option */
    if ( argx >= argc ) {
      log_error(tag, "%s: too few arguments");
      shell_std_help(shell, argv[0]);
      return -1;
    }

    /* Check for link duplication */
    if ( link != NULL ) {
      log_error(tag, "%s: link already exists", id);
      return -1;
    }

    /* Create new link entry */
    if ( (link = link_new(id)) == NULL ) {
      log_error(tag, "%s: cannot create new link", id);
      return -1;
    }

    /* Parse link source settings */
    if ( link_parse(link, argc-argx, argv+argx) == -1 ) {
      link_free(link);
      log_error(tag, "%s: illegal link specification", id);
      shell_std_help(shell, argv[0]);
      return -1;
    }
  }

  /* Report link status */
  if ( link != NULL ) {
    link_report(link, hdr);
  }

  return 0;
}


/*******************************************************/
/* unlink                                              */
/* Subprocess - dataflow unlink                        */
/*******************************************************/

static int link_cmd_unlink(shell_t *shell, shell_argv_t *cmd_argv, char *tag)
{
  int argc = cmd_argv->argc;
  char **argv = cmd_argv->argv;
  char *id;
  link_item_t *link;

  /* Check arguments */
  if ( (argc < 2) || (argc > 3) ) {
    log_error(tag, "Too %s arguments", (argc < 2) ? "few":"many");
    shell_std_help(shell, argv[0]);
    return -1;
  }

  /* Retrieve link entry */
  id = strupper(argv[1]);
  if ( (link = link_retrieve(id)) == NULL ) {
    log_error(tag, "%s: unknown link", id);
    return 0;
  }

  /* Destroy link */
  link_info(link, log_hdr_(tag));
  link_free(link);

  return 0;
}


/*******************************************************/
/* Command interpreter setup                           */
/*******************************************************/

static char *link_cmd_help_link_fmt =
  "link <tag> %s\n"
  "  Create a new data flow link <tag>.\n"
  "%s"
  "link <tag>\n"
  "  Show description for link <tag> (all links if <tag> ommited).\n";

static shell_std_help_t link_cmd_help[] = {
  { "link",    NULL },
  { "unlink",  "unlink <tag>\n"
               "  Destroy link <tag>\n" },
  { "connect", "connect <tag> [<method>://]<address>\n"
               "  Connect link <tag> to subprocess using the specified connection method at <address>\n"
               "  Available connection methods are:\n"
               "  - Local piped subprocess: method='proc', <address>=<command line>\n"
               "  - Remote TCP/IP connection: method='tcpip', <address>=<IP-address>:<IP-port>\n"
               "  Default connection methods is 'proc' (local piped subprocess)\n"
               "connect [<tag>]\n"
               "  Report subprocess connection for link <tag> (all connections if <tag> ommited).\n" },
  { "kill",    "kill <tag> ...\n"
               "  Kill subprocess connection for link <tag>\n" },
  { "send",    "send <tag> <data>\n"
               "  Send out-of-band data to subprocess connection for link <tag>\n" },
  { NULL,     NULL }
};


static shell_command_t link_cmd_tab[] = {
  { "connect", link_cmd_connect, "CONNECT" },
  { "kill",    link_cmd_kill,    "KILL   " },
  { "send",    link_cmd_send,    "SEND   " },
  { "link",    link_cmd_link,    "LINK   " },
  { "unlink",  link_cmd_unlink,  "UNLINK " },
  { NULL,      shell_std_unknown, NULL }
};


void link_cmd_init(shell_t *shell)
{
  int size;

  if ( shell != NULL )
    shell_set_cmd(shell, link_cmd_tab);

  size = strlen(link_cmd_help_link_fmt) + strlen(link_src_desc.help_str0) + strlen(link_src_desc.help_str1) + 1;
  link_cmd_help[0].help = malloc(size);
  snprintf(link_cmd_help[0].help, size, link_cmd_help_link_fmt, link_src_desc.help_str0, link_src_desc.help_str1);

  shell_std_set_help(link_cmd_help);
}


void link_cmd_done(shell_t *shell)
{
  shell_std_unset_help(link_cmd_help);

  if ( link_cmd_help[0].help != NULL ) {
    free(link_cmd_help[0].help);
    link_cmd_help[0].help = NULL;
  }

  if ( shell != NULL )
    shell_unset_cmd(shell, link_cmd_tab);
}
