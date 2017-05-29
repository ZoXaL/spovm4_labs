#ifndef ZOX_STACK_H
#define ZOX_STACK_H

struct zox_StackElement {
	void* data;
	struct zox_StackElement* previous;
};

typedef struct zox_StackElement ZSE;
void zox_push(ZSE**, void*);
void* zox_pop(ZSE**);

#endif