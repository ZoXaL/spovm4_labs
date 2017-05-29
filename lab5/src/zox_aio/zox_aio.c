#define _POSIX_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include <error.h>

#include <glib.h>

#include "utilities.h"
#include "zox_aio.h"
#include "zox_aio_context.h"
#include "task/zox_aio_task.h" 
#include "task_executor/reader/reader.h" 
#include "task_executor/writer/writer.h"

static int initialize_context(zox_aio_context** new_context, zox_aio_settings settings) {
	int target_file_fd = creat(settings.target_file, 0);
	if (target_file_fd == -1) {
		perror("target file creat");
		return -1; 
	} 
	 
	(*new_context) = (zox_aio_context*)malloc(sizeof(zox_aio_context));
	if (!(*new_context)) {
		perror("zox_aio_context malloc");
		close(target_file_fd);
		return -1; 
	}
	memset((*new_context), 0, sizeof(zox_aio_context));

	(*new_context)->target_file_fd = target_file_fd;
	(*new_context)->target_file_path = zox_strdup(settings.target_file);
	(*new_context)->target_file_expected_size = 0;
	(*new_context)->tasks = g_array_new(FALSE, FALSE, sizeof(zox_aio_task*));
	(*new_context)->tasks_count = 0;
	pthread_mutex_init(&((*new_context)->tasks_mutex), NULL);
	(*new_context)->reader = create_reader((*new_context));
	(*new_context)->writer = create_writer((*new_context));
	(*new_context)->buffer_size = settings.buffer_size;
	(*new_context)->ms_delay = settings.ms_delay;
	(*new_context)->is_debug_enabled = settings.is_debug_enabled;
	return 0;
}

static void destroy_context(zox_aio_context* context) {
	char* file_path_copy = zox_strdup(context->target_file_path);
	int should_debug = context->is_debug_enabled;
	
	kill_executor(context->reader);
	kill_executor(context->writer);

	int i = 0;
	for (i = 0; i < context->tasks_count; i++) {
		delete_task(g_array_index(context->tasks, zox_aio_task*, i));
	}

	pthread_mutex_destroy(&(context->tasks_mutex));
	g_array_free(context->tasks, TRUE);	
	free(context->target_file_path);
	close(context->target_file_fd);
	free(context);

	if (should_debug) {
		printf("context %s destroyed\n", file_path_copy);
	}
	free(file_path_copy);
}

static int add_task(zox_aio_context* context, const char* file) {
	
	zox_aio_task* new_task = create_task(context, file);
	if (new_task == NULL) return -1;
	pthread_mutex_lock(&(context->tasks_mutex));
		g_array_append_val(context->tasks, new_task);
		context->tasks_count++;
	pthread_mutex_unlock(&(context->tasks_mutex));

	notify_executor(context->reader, new_task);
	return context->tasks_count - 1;
}

static int get_task_progress(zox_aio_context* context, int task_id) {
	zox_aio_task* task = g_array_index(context->tasks, zox_aio_task*, task_id);
	return calculate_task_progress(context, task);
}

static const char* get_task_filename(zox_aio_context* context, int task_id) {
	zox_aio_task* task = g_array_index(context->tasks, zox_aio_task*, task_id);
	return task->source_file_path;
}

const struct zox_aio ZOX_AIO = {
	.initialize_context = initialize_context,
	.destroy_context = destroy_context,
	.add_task = add_task,
	.get_task_progress = get_task_progress,
	.get_task_filename = get_task_filename
};