#ifndef CHILD_H
#define CHILD_H
 
typedef struct {
	pthread_t thread;
	pthread_mutex_t mutex;
	pthread_cond_t cond;
	char name;
	int message;	// message from tm, 1 to continue, -1 to die
} child_thread_t;

child_thread_t* start_child(char name);
void wake_child(child_thread_t*);
void kill_child(child_thread_t*);

#endif