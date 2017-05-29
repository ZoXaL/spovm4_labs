#include "func.h"
#include <stdlib.h>		//malloc
#include <stdio.h>		//printf
#include <wait.h>		//wait

#include <unistd.h>		//fork, getpid, getppid, execve
#include <sys/types.h>	//pid_t
#include <time.h>		//ctime, time_t

struct processInfo {
	pid_t pid;
};

struct processInfo* createProcess() {
	struct processInfo* pi;
	pi = (struct processInfo*)malloc(sizeof(struct processInfo));
	pid_t pid = fork();
	pi->pid = pid;
	switch(pid) {
		case -1: {
			printf("Error, can not create child process\n");
			return 0;
		}
		case 0: {
			printf("c:Child process has been created.\n");
			printf("c:child PID: %d\n", getpid());
			printf("c:parent PID: %d\n", getppid());
			printf("c: before execve\n");
			char* const argv[2] = {"empty", NULL};	//argv не должен быть NULL
			execvp("lab1", argv);
			//execv("/home/zoxal/BSUIR/4_sev/SPOVM/lab1");
			printf("c: after execve\n");
			for (int i = 0; i < 6; i++) {
				printf("c:i = %d\n", i);
				sleep(1);
			}
			break;
		}
		default: {
			printf("p:Parent process.\n");
			printf("p:child PID: %d\n", pi->pid);
			printf("p:parent PID: %d\n", getpid());
			for (int i = 0; i < 3; i++) {
				printf("p:i = %d\n", i);
				sleep(1);
			}
			break;
		}
	}
	return pi;
}

void waitForProcess(struct processInfo* pi) {
	printf("Got processInfo with pid %d to wait\n", pi->pid);
	if (pi->pid == 0) {
		return;
	} else {
		wait(&(pi->pid));
	}
}

void terminate() {
	time_t t;
	time(&t);
	printf("Terminated process with pid %d. Exit time: %s\n", getpid(), ctime(&t));
}