#include <stdlib.h>
#include "ring.h"

static void add(ZRE* current, void* data) {
	ZRE new_element = (ZRE)malloc(sizeof(struct zox_ring_element));
	if (!new_element) {
		return;
	}
	new_element->data = data;
	if (*current == NULL) {
		new_element->previous = new_element;
		new_element->next = new_element;
		*current = new_element;
	} else {
		new_element->previous = *current;
		new_element->next = (*current)->next;
		(*current)->next = new_element;
		new_element->next->previous = new_element;
		*current = new_element;
	}
	return;
}

static void* remove(ZRE* current) {
	void* data = NULL;
	if (*current == NULL) {
		return data;
	} else {
		data = (*current)->data;
		ZRE new_current = NULL;

		if ((*current)->next != (*current)) {
			(*current)->next->previous = (*current)->previous;
			(*current)->previous->next = (*current)->next;
			new_current = (*current)->previous;
		}		

		free(*current);
		*current = new_current;
	}
	return data;
}

static void next(ZRE* current) {
	if (*current == NULL) return;
	*current = (*current)->next;
}

static void previous(ZRE* current) {
	if (*current == NULL) return;
	*current = (*current)->previous;
}

static void* data(ZRE current) {
	if (current == NULL) return NULL;
	return current->data;
}

const struct zox_ring_library ZOX_RING = {
	.add = add,
	.remove = remove,
	.next = next,
	.previous = previous,
	.data = data
};