#include "stack.h"
#include <stdlib.h>

void zox_push(ZSE** top, void* data) {
	ZSE* newZSE = (ZSE*)malloc(sizeof(ZSE));
	newZSE->data = data;
	newZSE->previous = *top;
	*top = newZSE;
}

void* zox_pop(ZSE** top) {
	if (top == NULL || *top == NULL) {
		return NULL;
	}
	ZSE* poppedZSE = *top;
	void* poppedData = poppedZSE->data;
	*top = (*top)->previous;
	free(poppedZSE);
	return poppedData;
}