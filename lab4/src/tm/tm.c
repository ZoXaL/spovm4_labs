#define _GNU_SOURCE
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <pthread.h>
#include "child.h"
#include "ring.h"
#include "queue.h"

static pthread_t tm;
static pthread_mutex_t tm_data_mutex;
static pthread_cond_t wake_up_tm_cond;
static pthread_rwlock_t current_child_lock;
static ZQE tm_data = NULL;
static void* thread_manager(void* arg);

static ZRE current_child = NULL;
static ZRE top_child = NULL;
static char last_child_name = 'a';
// flags
static int tm_paused = 0;
static int tm_initialized = 0; 
static int tm_child_paused = 0;

int start_tm() {
    pthread_mutex_init(&tm_data_mutex, NULL);
    pthread_cond_init(&wake_up_tm_cond, NULL);
    pthread_rwlock_init(&current_child_lock, NULL);
    tm_initialized = 1;
    return pthread_create(&tm, NULL, thread_manager, NULL);    
}

void send_tm(char data) {
    // pushes new command to queue
    char* data_ptr = (char*)malloc(sizeof(char));
    *data_ptr = data;
    pthread_mutex_lock(&tm_data_mutex);
    ZOX_QUEUE.offer(&tm_data, data_ptr);
    pthread_mutex_unlock(&tm_data_mutex);
    pthread_cond_signal(&wake_up_tm_cond);
}

void kill_tm() {
    send_tm('q');

    pthread_join(tm, NULL);

    pthread_mutex_destroy(&tm_data_mutex);      
    pthread_cond_destroy(&wake_up_tm_cond); 
    pthread_rwlock_destroy(&current_child_lock); 
}

void print_current_child() {
    if (!current_child || !tm_initialized) {
        printf("Threre is no children yet\n");
        return;
    }
    pthread_rwlock_rdlock(&current_child_lock);
    child_thread_t* child = (child_thread_t*)(current_child->data);
    printf("Current running child: %c\n", child->name);
    pthread_rwlock_unlock(&current_child_lock);
}

void update_current_child(ZRE newValue) {
    pthread_rwlock_wrlock(&current_child_lock);
    current_child = newValue;
    pthread_rwlock_unlock(&current_child_lock);
}

static char getData() {
    // synchronously pops command from queue
    char data;
    char* tmp;
    pthread_mutex_lock(&tm_data_mutex);
    while(tm_data == NULL) pthread_cond_wait(&wake_up_tm_cond, &tm_data_mutex);
    tmp = (char*)ZOX_QUEUE.poll(&tm_data);
    pthread_mutex_unlock(&tm_data_mutex);
    data = *tmp;
    free(tmp);
    return data;
}

static void addChild() {
    // creates new child and calls if need
    int isFirst = (top_child == NULL) ? 1 : 0;
    ZOX_RING.add(&top_child, start_child(last_child_name));
    if (isFirst) {
        update_current_child(top_child);
        wake_child((child_thread_t*)(ZOX_RING.data(current_child)));
    }
    if (last_child_name!='z') {
        last_child_name++;
    } else {
        last_child_name = 'a';
    } 
}

static void removeChild() {
    kill_child((child_thread_t*)ZOX_RING.remove(&top_child));
    if (top_child == NULL) {
        update_current_child(NULL);
    }
    last_child_name--;
}

static void removeChildren() {
    while (top_child != NULL) {
        removeChild();
    }
}

static void* thread_manager(void* arg) {
    char data;
    while(1) {
        data = getData();
        switch (data) {
            case 'q' : {
                removeChildren();
                return NULL;
            }
            case '+' : {
                addChild();
                break;
            }
            case '-' : {
                if (top_child == NULL) {
                    printf("There is no any children yet\n");
                    break;
                }
                int flag = 0;
                if (current_child == top_child && top_child->next != top_child) flag = 1;
                removeChild();
                if (flag) send_tm('x');     // should send child signal
                
                break;
            }
            case 'p' : {
                // can not continue if paused child not finished
                if (tm_paused == 1 && tm_child_paused == 0) break;
                tm_paused ^= 1;
                tm_child_paused = 0;
                if (tm_paused == 0) send_tm('x');
                break;
            }
            case 'x' : {    // child signal
                if (current_child == NULL) break;   // last child sends signal before death 

                if (tm_paused == 1)  {
                    tm_child_paused = 1;
                    break;
                }
                update_current_child(current_child->next);
                wake_child((child_thread_t*)(current_child->data));
                break;
            }
        }
    }
    return NULL;
}