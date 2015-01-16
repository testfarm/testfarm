/****************************************************************************/
/* TestFarm                                                                 */
/* Shell script interpreter - Alias resolution manager                      */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 26-AUG-1999                                                    */
/****************************************************************************/

/*
 * $Revision: 29 $
 * $Date: 2006-06-03 10:39:29 +0200 (sam., 03 juin 2006) $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "shell_argv.h"
#include "shell_alias.h"


shell_alias_item_t *shell_alias_item_alloc(char *keyword, int argc, char **argv)
{
  shell_alias_item_t *item;
  int i;

  item = (shell_alias_item_t *) malloc(sizeof(shell_alias_item_t));
  item->keyword = strdup(keyword);
  item->argc = argc;
  item->argv = (char **) malloc(sizeof(char *) * (argc+1));
  for (i = 0; i < argc; i++)
    item->argv[i] = strdup(argv[i]);
  item->argv[argc] = NULL;

  return item;
}


void shell_alias_item_free(shell_alias_item_t *item)
{
  int i;

  free(item->keyword);
  for (i = 0; i < item->argc; i++)
    free(item->argv[i]);
  free(item->argv);
  free(item);
}


void shell_alias_item_show(shell_alias_item_t *item, char *hdr)
{
  int i;

  if ( item == NULL )
    return;

  if ( hdr != NULL )
    fputs(hdr, stdout);

  printf("%s = ", item->keyword);
  for (i = 0; i < item->argc; i++)
    printf("%s ", item->argv[i]);
  printf("\n");
}



shell_alias_t *shell_alias_init(shell_alias_t *inherit)
{
  /* Alloc alias mangement descriptor */
  shell_alias_t *alias = (shell_alias_t *) malloc(sizeof(shell_alias_t));
  alias->tab = NULL;
  alias->count = 0;

  /* Inherit from parent if any */
  if ( inherit != NULL ) {
    int i;

    alias->tab = (shell_alias_item_t **) malloc(sizeof(shell_alias_item_t *) * (inherit->count+1));
    alias->count = inherit->count;

    for (i = 0; i < inherit->count; i++) {
      shell_alias_item_t *item = inherit->tab[i];
      alias->tab[i] = shell_alias_item_alloc(item->keyword, item->argc, item->argv);
    }
    alias->tab[alias->count] = NULL;
  }

  return alias;
}


void shell_alias_done(shell_alias_t *alias)
{
  int i;

  if ( alias->tab != NULL ) {
    for (i = 0; i < alias->count; i++)
      shell_alias_item_free(alias->tab[i]);
    free(alias->tab);
  }

  free(alias);
}


shell_alias_item_t *shell_alias_retrieve(shell_alias_t *alias, char *keyword)
{
  int i;

  if ( alias == NULL )
    return NULL;
  if ( alias->tab == NULL )
    return NULL;

  for (i = 0; i < alias->count; i++) {
    shell_alias_item_t *item = alias->tab[i];
    if ( strcmp(item->keyword, keyword) == 0 )
      return item;
  }

  return NULL;
}


shell_alias_item_t *shell_alias_add(shell_alias_t *alias, char *keyword, int argc, char **argv)
{
  shell_alias_item_t *item;
  shell_alias_item_t **p = NULL;

  /* Alias already defined ? */  
  if ( (item = shell_alias_retrieve(alias, keyword)) != NULL ) {
    p = alias->tab;
    while ( (*p != item) && (*p != NULL) )
      p++;
  }

  /* Alloc new entry in alias table */
  if ( p == NULL ) {
    alias->tab = (shell_alias_item_t **) realloc(alias->tab, (alias->count+2) * sizeof(shell_alias_item_t *));
    p = &(alias->tab[(alias->count)++]);
    alias->tab[alias->count] = NULL;
  }
  else {
    shell_alias_item_free(*p);
  }

  /* Fill entry with new item */
  *p = shell_alias_item_alloc(keyword, argc, argv);

  return *p;
}


void shell_alias_resolve(shell_alias_t *alias, shell_argv_t *shell_argv)
{
  int argc = shell_argv->argc;
  char **argv = shell_argv->argv;
  shell_alias_item_t *item = shell_alias_retrieve(alias, argv[0]);
  int i;

  if ( item == NULL )
    return;

  /* Expand arguments list if needed */
  argc += item->argc - 1;
  argv = (char **) realloc(argv, sizeof(char *) * (argc+1));

  for (i = argc; i >= item->argc; i--)
    argv[i] = argv[i - item->argc + 1];
  for (i = 0; i < item->argc; i++)
    argv[i] = item->argv[i];

  shell_argv->argc = argc;
  shell_argv->argv = argv;
}


void shell_alias_show(shell_alias_t *alias, char *hdr)
{
  int i;

  for (i = 0; i < alias->count; i++)
    shell_alias_item_show(alias->tab[i], hdr);
}
