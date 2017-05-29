#define _GNU_SOURCE
#include <stdlib.h>
#include <glib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include <errno.h>
#include "child.h"
#include "tm.h"

gint child_delay = 100;			// can be set by command line arg
gint child_letters_count = 5;	// can be set by cla
int outputFD = 1;

void* child_thread(void* arg);

child_thread_t* start_child(char name) {
	child_thread_t* new_child = (child_thread_t*)malloc(sizeof(child_thread_t));
	if (!new_child) return NULL;

	new_child->name = name;
	new_child->message = 0;
	pthread_mutex_init(&(new_child->mutex), NULL);
	pthread_cond_init(&(new_child->cond), NULL);

	int call_result = pthread_create(&(new_child->thread), NULL, child_thread, new_child);
	if (call_result != 0) {
		errno = call_result;
		return NULL;
	}
	return new_child;
}

void wake_child(child_thread_t* child) {
	pthread_mutex_lock(&(child->mutex));
	child->message = 1;
	pthread_mutex_unlock(&(child->mutex));
	pthread_cond_signal(&(child->cond));
}

void kill_child(child_thread_t* child) {
	// to kill sleeping child, signal him by his mutex 
	pthread_mutex_lock(&(child->mutex));
	child->message = -1;
	pthread_mutex_unlock(&(child->mutex));
	pthread_cond_signal(&(child->cond));

	pthread_cancel(child->thread);
	pthread_join(child->thread, NULL);
	printf("Child %c was killed\n", child->name);

	pthread_mutex_destroy(&(child->mutex));
	pthread_cond_destroy(&(child->cond));
}

void* child_thread(void* arg) {	
    if (pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL) != 0) {
    	perror("pthread_setcanceltype: ");
    }

	child_thread_t* this = (child_thread_t*)arg;
	int message;
	setvbuf(stdout, NULL, _IONBF, 0);
	while(1) {
		pthread_mutex_lock(&(this->mutex));
		while (this->message == 0) pthread_cond_wait(&(this->cond), &(this->mutex));
		message = this->message;
		this->message = 0;
		pthread_mutex_unlock(&(this->mutex));
		if (message == 1) {
			for (int i = 0; i < child_letters_count; i++) {
    			write(outputFD, &this->name, 1);
				usleep(1000*child_delay);
				pthread_setcancelstate(PTHREAD_CANCEL_ENABLE,NULL);
			    pthread_testcancel();
			    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE,NULL);
			}
			write(outputFD, "\n", 1);
		} else {
			// child death
			return NULL;
		}
		send_tm('x');
	}
	return NULL;
}