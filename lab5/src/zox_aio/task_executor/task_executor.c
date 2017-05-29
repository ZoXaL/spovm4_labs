#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <glib.h>
#include <error.h>
#include <string.h>
#include "task_executor/task_executor.h"
#include "zox_aio_context.h"
#include "task/zox_aio_task.h"

static void* executor_thread(void* data) {
	zox_aio_executor* this = (zox_aio_executor*)data;
	while(1) {
		pthread_mutex_lock(&(this->notification_cond_mutex));
		while(g_queue_get_length(this->notification_queue) == 0) {
			pthread_cond_wait(&(this->notification_cond), 
								&(this->notification_cond_mutex));
		}
		gpointer notification = g_queue_pop_head(this->notification_queue);
		pthread_mutex_unlock(&(this->notification_cond_mutex));
		if (notification == NULL) {
			if (this->context->is_debug_enabled == 1) {
				printf("executor was killed\n");
			}			
			return NULL;
		} else {
			(*(this->task_listener))((zox_aio_task*)notification);
		}
	}
	return NULL;
}

zox_aio_executor* create_executor(struct zox_aio_context* context) {
	zox_aio_executor* executor = (zox_aio_executor*)malloc(sizeof(zox_aio_executor));
	if (!executor) {
		perror("zox_aio_executor malloc");
		return NULL;
	}
	memset(executor, 0, sizeof(zox_aio_executor));

	pthread_cond_t notification_cond;
	pthread_cond_init(&notification_cond, NULL);
	pthread_mutex_t notification_cond_mutex;
	pthread_mutex_init(&notification_cond_mutex, NULL);

	executor->context = context;
	executor->notification_queue = g_queue_new();
	executor->notification_cond = notification_cond;	
	executor->notification_cond_mutex = notification_cond_mutex;

	if (pthread_create(&(executor->thread), NULL, executor_thread, executor) == 0) {
		return executor;
	} else {
		perror("executor pthread_create");
		return NULL;
	}
}

void notify_executor(zox_aio_executor* executor, struct zox_aio_task* task) {
	pthread_mutex_lock(&(executor->notification_cond_mutex));
		g_queue_push_tail(executor->notification_queue, task);
		pthread_cond_signal(&(executor->notification_cond));
	pthread_mutex_unlock(&(executor->notification_cond_mutex));
} 

void kill_executor(zox_aio_executor* executor) {
	pthread_mutex_lock(&(executor->notification_cond_mutex));
		g_queue_push_tail(executor->notification_queue, NULL);
		pthread_cond_signal(&(executor->notification_cond));
	pthread_mutex_unlock(&(executor->notification_cond_mutex));	
	pthread_join(executor->thread, NULL);
}