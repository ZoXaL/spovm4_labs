#ifndef TASK_EXECUTOR_H
#define TASK_EXECUTOR_H

#include <pthread.h>
#include <glib.h>

struct zox_aio_context;
struct zox_aio_task;

struct zox_aio_executor {
	struct zox_aio_context* context;
	GQueue* notification_queue;
	pthread_cond_t notification_cond;
	pthread_mutex_t notification_cond_mutex;
	pthread_t thread;
	void (*task_listener)(struct zox_aio_task*);
};
typedef struct zox_aio_executor zox_aio_executor;

zox_aio_executor* create_executor(struct zox_aio_context*);
void notify_executor(zox_aio_executor*, struct zox_aio_task*);
void kill_executor(zox_aio_executor*);

#endif