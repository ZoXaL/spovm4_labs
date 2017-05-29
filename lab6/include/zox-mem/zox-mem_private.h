#ifndef ZOX_MEM_PRIVATE_H
#define ZOX_MEM_PRIVATE_H

#include <stdint.h>
#include <stdbool.h>

union EOB {
	int32_t as_int;
	char as_char[4];
};

extern const union EOB EOB;

#define BLOCK_OVERHEAD sizeof(EOB) + sizeof(char)
#define FREE_MARK_CHAR 'f'
#define USED_MARK_CHAR 'u'

typedef struct heap_block_t {
	void* heap;
	size_t offset;
	void* data;
} heap_block_t;

heap_block_t markup_block(void*, size_t, size_t);
heap_block_t get_block(void*, size_t);
heap_block_t get_next_block(heap_block_t, size_t);
heap_block_t get_previous_block(heap_block_t);
void resize_block(heap_block_t, size_t);
void mark_block_used(heap_block_t);
void mark_block_free(heap_block_t);
bool is_block_free(heap_block_t);
size_t calculate_block_size(heap_block_t);
void split_block(heap_block_t, size_t);
void hard_block_move(heap_block_t, size_t);

#endif