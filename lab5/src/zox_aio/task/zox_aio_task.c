#define _POSIX_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <fcntl.h>
#include <error.h>
#include <pthread.h>
#include <aio.h>

#include "utilities.h"
#include "task/zox_aio_task.h"
#include "zox_aio_context.h"
#include "task_executor/task_executor.h"

void aio_callback(union sigval callback_data) {
	aio_callback_data* data = (aio_callback_data*)(callback_data.sival_ptr);
	notify_executor(data->executor, data->task);
	return;
} 
 
zox_aio_task* create_task(zox_aio_context* context, const char* file) {

	int task_file_fd = open(file, O_RDONLY);
	if (task_file_fd == -1) {
		perror(file);
		return NULL;
	} 
	// using dup here to leave task_file_fd opened after closing FILE* by fclose
	int fdcopy = dup(task_file_fd);
	FILE* task_file = fdopen(fdcopy, "r");	
	if (task_file == NULL) {
		perror("target FILE* fdopen");
		close(task_file_fd);
		return NULL;
	}
	fseek(task_file, 0, SEEK_END);	
	long file_size = ftell(task_file);
	fclose(task_file);
	
	zox_aio_task* new_task = (zox_aio_task*)malloc(sizeof(zox_aio_task));
	if (!new_task) {
		perror("new task malloc");
		close(task_file_fd);
		return NULL;
	}
	memset(new_task, '\0', sizeof(zox_aio_task));

	struct aiocb* cb = (struct aiocb*)malloc(sizeof(struct aiocb));
	if (cb == NULL) {
		perror("aiocb malloc");
		return NULL;
	}
	memset(cb, '\0', sizeof(struct aiocb));

	aio_callback_data* callback_data = (aio_callback_data*)malloc(sizeof(aio_callback_data));
	if (cb == NULL) {
		perror("aio_callback_data malloc");
		close(task_file_fd);
		free(cb);
		free(new_task);
		return NULL;
	}
	memset(callback_data, '\0', sizeof(aio_callback_data));
	callback_data->task = new_task;
	
	cb->aio_buf = malloc(context->buffer_size);
	if (!cb->aio_buf) {
		perror("aiocb buffer malloc");
		close(task_file_fd);
		free(callback_data);
		free(cb);
		free(new_task);
		return NULL;
	}
	memset((void*)cb->aio_buf, '\0', context->buffer_size);
	cb->aio_nbytes = context->buffer_size;
	cb->aio_sigevent.sigev_notify = SIGEV_THREAD;
	cb->aio_sigevent.sigev_notify_function = aio_callback;
	cb->aio_sigevent.sigev_notify_attributes = NULL;
	cb->aio_sigevent.sigev_value.sival_ptr = callback_data;	

	new_task->cb = cb;
	new_task->context = context;
	new_task->source_file_path = zox_strdup(file);
	new_task->source_file_size = file_size;
	new_task->source_file_fd = task_file_fd;
	new_task->read_offset = 0;
	new_task->buffers_written = 0;

	pthread_mutex_lock(&(context->tasks_mutex));
		new_task->write_offset = context->target_file_expected_size;
		context->target_file_expected_size += file_size;
	pthread_mutex_unlock(&(context->tasks_mutex));
	return new_task;
}

void delete_task(zox_aio_task* task) {
	if (task->context->is_debug_enabled == 1) {
		printf("deleting task %s\n", task->source_file_path);
	}	
	free((void*)task->source_file_path);
	close(task->cb->aio_fildes);
	free((void*)task->cb->aio_buf);
	free(task->cb);
	free(task);
}

int calculate_task_progress(zox_aio_context* context, zox_aio_task* task) {
	int bytes_written = task->buffers_written * context->buffer_size;
	if (task->buffers_written > 0) {
		bytes_written -= context->buffer_size;
		bytes_written += task->cb->aio_nbytes;
	}
	float percent_written = ((float)bytes_written/task->source_file_size);
	return (int)(percent_written*100);
}