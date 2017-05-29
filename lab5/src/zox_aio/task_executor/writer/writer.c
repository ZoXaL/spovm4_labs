#include <stdlib.h>
#include <stdio.h>
#include <aio.h>
#include <string.h>
#include "task_executor/writer/writer.h"
#include "task/zox_aio_task.h" 
#include "zox_aio_context.h"

void writer_job(zox_aio_task* task) {
	zox_aio_context* context = task->context;

	struct aiocb* cb = task->cb;
	aio_callback_data* callback_data = (aio_callback_data*)(cb->aio_sigevent.sigev_value.sival_ptr);
	callback_data->executor = context->reader;	
	cb->aio_offset = task->write_offset;
	cb->aio_fildes = context->target_file_fd;
	task->write_offset+=context->buffer_size;
	 
	// char* buf = (char*)malloc(context->buffer_size+1);
	// strcpy(buf, (char*)cb->aio_buf);
	// buf[context->buffer_size] = '\0';
	// printf("writer is going to write %s\n", buf);
	aio_write(cb);
}

zox_aio_executor* create_writer(struct zox_aio_context* context) {
	zox_aio_executor* writer = create_executor(context);
	writer->task_listener = writer_job;
	return writer;
}