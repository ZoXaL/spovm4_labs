#include <stdio.h>
#include <memory.h>
#include <stdlib.h>
#include <stdbool.h>
#include "zox-mem_private.h"

const union EOB EOB = {.as_int = 461143067};	// 1B7C7C1B // " || "

heap_block_t markup_block(void* heap, size_t offset, size_t size) {
	heap_block_t block;
	block.heap = heap;
	block.offset = offset;
	block.data = (void*)((char*)heap + 1);

	*((char*)heap + offset) = FREE_MARK_CHAR;

	strcpy((char*)heap + offset + size + 1, EOB.as_char);
	return block;
}
heap_block_t get_block(void* heap, size_t offset) {
	heap_block_t block;
	block.heap = heap;
	block.offset = offset;
	block.data = (void*)((char*)heap + offset + 1);
	return block;
}
heap_block_t get_next_block(heap_block_t previous_block, size_t total_size) {
	char* heap = (char*)(previous_block.heap);
	size_t offset = previous_block.offset;
	bool found_flag = false;
	while(offset++ < (total_size - sizeof(EOB)) && !found_flag) {
		int i = 0;
		for(; i < sizeof(EOB); i++) {
			if (heap[offset+i] != EOB.as_char[i]) break;
		}
		if (i == sizeof(EOB)) found_flag = true;
	}
	return get_block(previous_block.heap, offset + sizeof(EOB) - 1);
}

heap_block_t get_previous_block(heap_block_t block) {
	char* heap = (char*)(block.heap);
	size_t offset = block.offset - sizeof(EOB);
	if (block.offset < sizeof(EOB)) return get_block(block.heap, 0);
	bool found_flag = false;
	while((offset > 0) && !found_flag) {
		int i = sizeof(EOB) - 1;
		for(; i >= 0; i--) {
			if (heap[offset + i - sizeof(EOB)] != EOB.as_char[i]) break;
		}
		if (i == -1) found_flag = true;
		offset--;
	}
	return get_block(block.heap, offset+1) ;
}
void resize_block(heap_block_t block, size_t size) {
	char* heap = (char*)(block.heap);
	size_t block_size = calculate_block_size(block);
	for(int i = 0; i < sizeof(EOB); i++) {
		heap[block.offset + block_size + i] = '\0';
	}
	markup_block(block.heap, block.offset, size);
}
void mark_block_used(heap_block_t block) {
	*((char*)(block.data) - 1) = USED_MARK_CHAR;
}
void mark_block_free(heap_block_t block) {
	*((char*)(block.data) - 1) = FREE_MARK_CHAR;
}

bool is_block_free(heap_block_t block) {
	return *((char*)(block.data) - 1) == FREE_MARK_CHAR;
}

size_t calculate_block_size(heap_block_t block) {
	char* data = (char*)(block.data);
	size_t size = 0;
	bool found_flag = false;
	while(!found_flag) {
		int i = 0;
		for(; i < sizeof(EOB); i++) {
			if (data[size + i] != EOB.as_char[i]) break;
		}
		size++;
		if (i == sizeof(EOB)) found_flag = true;
	}
	return size;
}
void split_block(heap_block_t block, size_t size) {
	char* heap = (char*)(block.heap);
	char was_used = *((char*)block.data - 1);

	for(int i = 0; i < sizeof(EOB); i++) {
		heap[block.offset + 1 + size + i] = EOB.as_char[i];
	}
	heap[block.offset + size + sizeof(EOB) + 1] = was_used;
}

void hard_block_move(heap_block_t block, size_t move_to) {
	size_t block_size = calculate_block_size(block);
	char* heap = (char*)(block.heap);
	for (int i = 0; i < block_size + sizeof(EOB); i++) {
		heap[move_to + i] = heap[block.offset + i];
	}
}