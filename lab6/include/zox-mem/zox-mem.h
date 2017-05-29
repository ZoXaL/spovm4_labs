#ifndef ZOX_MEM_H
#define ZOX_MEM_H

#include <sys/types.h>

typedef struct zox_memory_t {
	void* (*malloc)(size_t);
	void (*free)(void*);
	void* (*realloc)(void*, size_t);
	void (*exit)();
} zox_memory_t;

size_t get_internal_offset(void*);

extern const zox_memory_t zox_memory;

#endif