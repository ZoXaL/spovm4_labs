#ifndef ZOX_AIO_CONTEXT_H
#define ZOX_AIO_CONTEXT_H
	
#include <glib.h>	// GArray
#include <pthread.h>// pthread_mutex_t
#include <stddef.h>	// size_t

struct zox_aio_executor;

// контекст -- описывает операцию конкатенации с одним результирующим файлом
struct zox_aio_context {
	int target_file_fd;
	char* target_file_path;
	size_t target_file_expected_size;

	GArray* tasks;
	guint tasks_count;
	pthread_mutex_t tasks_mutex;	// makes library mt-safe
	
	struct zox_aio_executor* reader;
	struct zox_aio_executor* writer;
	
	int buffer_size;
	int ms_delay;
	int is_debug_enabled;
};
typedef struct zox_aio_context zox_aio_context;

#endif