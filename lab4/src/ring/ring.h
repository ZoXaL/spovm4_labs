#ifndef ZOX_RING_H
#define ZOX_RING_H

/**
*	Simple ring data structure.
*
*	Created on 17.04.17 by zoxal
*/

typedef struct zox_ring_element* ZRE;

struct zox_ring_element {
	void* data;
	ZRE next;
	ZRE previous;
};

struct zox_ring_library {
	void (*add)(ZRE* current, void* data);	// add after current
	void* (*remove)(ZRE* current);			// current will be next
	void (*next)(ZRE* current);
	void (*previous)(ZRE* current);
	void* (*data)(ZRE current);
};

extern const struct zox_ring_library ZOX_RING;
#endif