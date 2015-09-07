/****************************************************************************/
/* TestFarm                                                                 */
/* Shell command history                                                    */
/****************************************************************************/
/* Author: Sylvain Giroudon                                                 */
/* Creation: 03-MAY-2010                                                    */
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
#include <glib.h>

#include "shell_history.h"

void shell_history_init(shell_history_t *hst)
{
	int i;

	/* Reset history counter */
	hst->count = 0;

	/* Clear history content */
	for (i = 0; i < SHELL_HISTORY_MAX; i++) {
		shell_history_item_t *item = &hst->tab[i];
		item->id = -1;
		item->str = NULL;
		item->ret = 0;
	}

	/* Clear ordered history list */
	hst->id = 0;
	hst->list = NULL;

	/* Clear callback */
	hst->callback_func = NULL;
	hst->callback_data = NULL;
}


void shell_history_set_callback(shell_history_t *hst, shell_history_callback_t *func, void *data)
{
	/* Set callback */
	hst->callback_func = func;
	hst->callback_data = data;
}


static int shell_history_cmp_str(shell_history_item_t *item, char *str)
{
	return strcmp(item->str, str);
}

static shell_history_item_t *shell_history_retrieve(shell_history_t *hst, char *str)
{
	GList *l = g_list_find_custom(hst->list, str, (GCompareFunc) shell_history_cmp_str);
	return (l != NULL) ? l->data : NULL;
}


shell_history_item_t *shell_history_update(shell_history_t *hst, char *str)
{
	shell_history_item_t *item = shell_history_retrieve(hst, str);

	/* Add new item if not found in the history */
	if (item == NULL) {
		hst->id++;

		/* If there are available history items, take one */
		if (hst->count < SHELL_HISTORY_MAX) {
			item = &(hst->tab[(hst->count)++]);
		}

		/* If the history is full, replace the oldest one */
		else {
			GList *l = g_list_last(hst->list);
			if (l == NULL) {
				fprintf(stderr, "*PANIC* History list corrupted (1).\n");
				return NULL;
			}

			item = l->data;
			if (item == NULL) {
				fprintf(stderr, "*PANIC* History list corrupted (2)\n");
				return NULL;
			}

			if (hst->callback_func)
				hst->callback_func(item, SHELL_HISTORY_DEL, hst->callback_data);

			/* Free command buffer */
			if (item->str != NULL) {
				free(item->str);
				item->str = NULL;
			}

			/* Remove item from list */
			hst->list = g_list_remove(hst->list, item);
		}

		item->id = hst->id;
		item->str = strdup(str);
		item->ret = 0;

		if (hst->callback_func)
			hst->callback_func(item, SHELL_HISTORY_ADD, hst->callback_data);
	}
	else {
		hst->list = g_list_remove(hst->list, item);

		if (hst->callback_func)
			hst->callback_func(item, SHELL_HISTORY_MOVE_UP, hst->callback_data);
	}

	hst->list = g_list_prepend(hst->list, item);

	return item;
}


void shell_history_first(shell_history_t *hst, shell_history_item_t *item)
{
	hst->list = g_list_remove(hst->list, item);
	hst->list = g_list_prepend(hst->list, item);
}


static int shell_history_cmp_id(shell_history_item_t *item, int id)
{
	return (item->id == id) ? 0 : 1;
}

shell_history_item_t *shell_history_get(shell_history_t *hst, int id)
{
	GList *l = g_list_find_custom(hst->list, GINT_TO_POINTER(id), (GCompareFunc) shell_history_cmp_id);

	if (l == NULL)
		return NULL;
	return (shell_history_item_t *) l->data;
}


void shell_history_foreach(shell_history_t *hst, void (*func)(shell_history_item_t *item, void *data), void *data)
{
	g_list_foreach(hst->list, (GFunc) func, data);
}
