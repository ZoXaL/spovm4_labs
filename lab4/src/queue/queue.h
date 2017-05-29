#ifndef ZOX_QUEUE_H
#define ZOX_QUEUE_H

/**
*	Simple queue data structure.
*	You have the queue head; data will be popped from head and added to tail.
*
*	Created on 17.04.17 by zoxal
*/

struct zox_queue_element {
	void* data;
	struct zox_queue_element* next;
};

typedef struct zox_queue_element* ZQE;

struct zox_queue_library {
	void (*offer)(ZQE* tail, void* data);
	void* (*poll)(ZQE* tail);
	void (*erase)(ZQE* tail);
	void (*empty)(ZQE* tail);
};

extern const struct zox_queue_library ZOX_QUEUE;
#endif