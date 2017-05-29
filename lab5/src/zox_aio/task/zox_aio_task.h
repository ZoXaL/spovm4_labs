#ifndef ZOX_AIO_TASK_H
#define ZOX_AIO_TASK_H

#include <sys/types.h>	// off_t
struct aiocb;
struct zox_aio_context;
struct zox_aio_executor;

// Описывает один файл-слагаемое
struct zox_aio_task {
	struct zox_aio_context* context;	// путь к файлу для конкатенации
	const char* source_file_path;	// путь к файлу для конкатенации
	off_t source_file_size;	// размер файла для конкатенации
	int source_file_fd;	
	off_t read_offset;		// текущая позиция в исходном файле (для читателя)
	off_t write_offset;		// текущая позиция в результирущем файле (для писателя)
	struct aiocb* cb;		// aiocb (тут же храню буффер для работы)
	int buffers_written;	// количество записанных буфферов для анализа завершённости задачи
};
typedef struct zox_aio_task zox_aio_task;

// callback data wich aiocb will use to notify
struct aio_callback_data {
	struct zox_aio_executor* executor;
	zox_aio_task* task;
};
typedef struct aio_callback_data aio_callback_data;

zox_aio_task* create_task(struct zox_aio_context*, const char* file);
void delete_task(zox_aio_task*);
int calculate_task_progress(struct zox_aio_context*, zox_aio_task*);

#endif

