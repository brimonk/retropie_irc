/*
 * Brian Chrzanowski
 * Mon Mar 05, 2018 18:12
 *
 * some nice doubly linked list functions
 * create
 * add
 * delete
 */
#include <stdlib.h>

#include "data_types.h"

list_t *list_create(void *ptr)
{
	list_t *node;

	node = NULL;

	if (ptr) {
		node = malloc(sizeof(struct list_t));

		if (node) {
			node->next = NULL;
			node->prev = NULL;
			node->data = ptr;
		}
	}

	return node;
}

list_t *list_add(list_t *head, list_t *ptr)
{
	/* marches to the end of the list, plops ptr at the end */
	list_t *tmp;

	if (!head) {
		return ptr; /* single node list where ptr is the only node */
	}

	for (tmp = head; tmp->next != NULL; tmp = tmp->next)
		;

	/* tmp->next is the next node in the list; hook it up */
	tmp->next = ptr;
	ptr->prev = tmp;

	return head; /* we return the current head */
}

