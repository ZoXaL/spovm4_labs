#define _POSIX_C_SOURCE 199309L		// ?? is this good? need for nanosleep
#include <stdio.h>
#include <aio.h>
#include <time.h>
#include "task_executor/reader/reader.h"
#include "task/zox_aio_task.h"
#include "zox_aio_context.h"
 
void reader_job(zox_aio_task* task) {
	zox_aio_context* context = task->context;
	struct aiocb* cb = task->cb;
	aio_callback_data* callback_data = (aio_callback_data*)(cb->aio_sigevent.sigev_value.sival_ptr);

	callback_data->executor = context->writer; 
	
	if (*((char*)cb->aio_buf) != '\0') task->buffers_written++;
	int bytes_written = task->buffers_written * context->buffer_size;
	if (bytes_written >= task->source_file_size) {
		if (context->is_debug_enabled == 1) {
			printf("File %s is written\n", task->source_file_path);	
		}
		return;
	}

	if ((task->source_file_size - bytes_written) < context->buffer_size) {
		cb->aio_nbytes = (task->source_file_size - bytes_written);	// last reading
	}
 
	cb->aio_offset = task->read_offset;	
	cb->aio_fildes = task->source_file_fd;	
	task->read_offset+=context->buffer_size;

	// ----- delay -----
	struct timespec sleep_time; 
	sleep_time.tv_sec = 0;
	sleep_time.tv_nsec = 1000 * 1000 * context->ms_delay;
	nanosleep(&sleep_time, NULL);
	// -----------------

	aio_read(cb);
	
	// execute aiocb from task
}

zox_aio_executor* create_reader(struct zox_aio_context* context) {
	zox_aio_executor* reader = create_executor(context);
	reader->task_listener = reader_job;
	return reader;
}