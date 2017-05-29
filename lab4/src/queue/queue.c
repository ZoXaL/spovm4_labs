#include "queue.h"
#include <stdlib.h>

static void offer(ZQE* tail, void* data) {
	ZQE newZQE = (ZQE)malloc(sizeof(struct zox_queue_element));
	newZQE->data = data;
	newZQE->next = NULL;

	if (*tail == NULL) {
		*tail = newZQE;
		return;
	}
	while((*tail)->next != NULL) *tail = (*tail)->next;	
	(*tail)->next = newZQE;
} 

static void* poll(ZQE* tail) {
	if (tail == NULL || *tail == NULL) {
		return NULL;
	}
	ZQE poppedZQE = *tail;
	void* poppedData = poppedZQE->data;
	*tail = (*tail)->next;
	free(poppedZQE);
	return poppedData;
}

static void erase(ZQE* tail) {
	if (tail == NULL || *tail == NULL) {
		return;
	}
	while((*tail)->next != NULL) {
		ZQE tmp = *tail;
		*tail = (*tail)->next;	
		free(tmp->data);
		free(tmp);
	}
}

static void empty(ZQE* tail) {
	if (tail == NULL || *tail == NULL) {
		return;
	}
	while((*tail)->next != NULL) {
		ZQE tmp = *tail;
		*tail = (*tail)->next;	
		free(tmp);
	}
}

const struct zox_queue_library ZOX_QUEUE = {
	.offer = offer,
	.poll = poll,
	.erase = erase,
	.empty = empty
};