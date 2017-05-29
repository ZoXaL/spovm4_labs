#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <zconf.h>
#include <memory.h>
#include <errno.h>
#include "zox-mem.h"
#include "zox-mem_private.h"

static void* _heap_ptr = NULL;
static size_t _heap_size = 0;
static char _heap_file_name[80] = "heap.txt";
static int _heap_file = -1;

size_t get_internal_offset(void* ptr) {
	return ptr - _heap_ptr;
}

static void update_heap(size_t new_heap_size) {
	//printf("updating heap size: %ld, was %ld\n", new_heap_size, _heap_size);
	if (_heap_ptr == NULL) {
		_heap_file = open(_heap_file_name, O_CREAT | O_RDWR | O_TRUNC, (mode_t)0600);
		if (_heap_file == -1) {
			fprintf(stderr, "cannot open swap file %s: %s",
					_heap_file_name, strerror(errno));
			return;
		}
	}
	if (ftruncate(_heap_file, new_heap_size) == -1) {
		fprintf(stderr, "cannot truncate swap file %s: %s",
				_heap_file_name, strerror(errno));
		return;
	} else {
		munmap(_heap_ptr, _heap_size);
	}
	_heap_ptr = mmap(_heap_ptr, new_heap_size,
					 PROT_READ | PROT_WRITE, MAP_SHARED, _heap_file, 0);
	printf("heap was raised from %ld to %ld\n", _heap_size, new_heap_size);
	_heap_size = new_heap_size;

}

// если можно вставить в какой-то промежуток, вставляет.
// если нет, расширяет heap.
static void* _zox_malloc(size_t size) {
	if (_heap_ptr == NULL) {
		update_heap(size + BLOCK_OVERHEAD);
		heap_block_t block = markup_block(_heap_ptr, 0, size);
		mark_block_used(block);
		return block.data;
	}
	heap_block_t block = get_block(_heap_ptr, 0);
	heap_block_t last = block;

	while(block.offset < _heap_size
		  && !(is_block_free(block)
			   && calculate_block_size(block) >= size + BLOCK_OVERHEAD)) {
		last = block;
		block = get_next_block(block, _heap_size);
	}
	if (block.offset < _heap_size) {
		split_block(block, size);
		mark_block_used(block);
		return block.data;
	} else {
		if (is_block_free(last)) {
			update_heap(last.offset + size + BLOCK_OVERHEAD);
			resize_block(last, size);
			mark_block_used(last);
			return last.data;
		} else {
			update_heap(block.offset + size + BLOCK_OVERHEAD);
			markup_block(_heap_ptr, block.offset, size);
			mark_block_used(block);
			return block.data;
		}
	}
}

static void _zox_free(void* free_ptr) {
	heap_block_t block = get_block(_heap_ptr, free_ptr - _heap_ptr - 1);
	mark_block_free(block);

	heap_block_t prev = get_previous_block(block);
	if (is_block_free(prev) && prev.offset != block.offset) {
		char* block_offset = (char*)(block.data) - 1 - sizeof(EOB);
		for (int i = 0; i < sizeof(EOB); i++) {
			block_offset[i] = '\0';
		}
	}
	heap_block_t next = get_next_block(block, _heap_size);
	if (next.offset < _heap_size && is_block_free(next)) {
		char* block_offset = (char*)(next.data) - 1 - sizeof(EOB);
		for (int i = 0; i < sizeof(EOB); i++) {
			block_offset[i] = '\0';
		}
	}
}

static void _zox_defrag() {
	size_t used_offset = 0;
	heap_block_t block = get_block(_heap_ptr, 0);
	heap_block_t next_block;
	while (block.offset < _heap_size) {
		next_block = get_next_block(block, _heap_size);
		if (!is_block_free(block)) {
			if (block.offset != used_offset) {
				hard_block_move(block, used_offset);
				mark_block_free(block);
			}
			used_offset = get_next_block(get_block(_heap_ptr, used_offset), _heap_size).offset;
		}
		block = next_block;
	}
	ftruncate(_heap_file, used_offset);
	return;
}

static void* _zox_realloc(void* pointer, size_t new_size) {
	heap_block_t old_block = get_block(_heap_ptr, pointer - _heap_ptr);
	size_t current_size = calculate_block_size(old_block);
	if (current_size >= new_size + BLOCK_OVERHEAD) {
		split_block(old_block, new_size);
		return old_block.data;
	}
	heap_block_t next_block = get_next_block(old_block, _heap_size);
	if (next_block.offset < _heap_size
		&& calculate_block_size(next_block)+BLOCK_OVERHEAD+current_size >= new_size) {
		char* block_offset = (char*)(next_block.data) - 1 - sizeof(EOB);
		for (int i = 0; i < sizeof(EOB); i++) {
			block_offset[i] = '\0';
		}
		split_block(old_block, new_size);
		return old_block.data;
	} else {
		_zox_free(pointer);
		return _zox_malloc(new_size);
	}
}

static void _close() {
	munmap(_heap_ptr, _heap_size);
	close(_heap_file);
	remove(_heap_file_name);
}

const zox_memory_t zox_memory = {
	malloc: _zox_malloc,
	free: _zox_free,
	realloc: _zox_realloc,
	exit: _close
};