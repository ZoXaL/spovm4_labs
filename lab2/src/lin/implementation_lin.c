#define _POSIX_SOURCE	// required for kill
#define _GNU_SOURCE
#include <sys/types.h>
#include <signal.h>		// kill
#include <unistd.h>
#include <wait.h>	
#include <time.h>
#include <sys/sem.h>
#include <stdlib.h>		// malloc
#include <stdio.h>
#include <string.h>		// memset

#include "interface.h"
#include "stack/stack.h"

ZSE* childToCall = NULL;
ZSE* childStackTop = NULL;
char newProcessSymbol = 'a'-1;
long int semID;

struct processInformation {
	pid_t pid;
};

void initChildrenHandler() {
	// ----- initializing semaphore -----
	long int pseudoUniqueID = 211232113;
	semID = semget(pseudoUniqueID, 1, IPC_CREAT | 0666);
	if (semID == -1) {
		printf("Cannot get semaphore, exit 3\n");
		exit(3);
	}
	semctl(semID, 0, SETVAL, 1);
	// ----------------------------------
}

PI* createProcess() {
	PI* pi = (PI*)malloc(sizeof(PI*));
	newProcessSymbol++;
	if (newProcessSymbol > 'z') {
		newProcessSymbol = 'a';
	}
	pid_t newProcessPid = fork();
	switch (newProcessPid) {
		case -1 : {
			printf("Fork error, exit 1\n");
			killChildren();
			exit(1);
		}
		case 0 : {	// child
			printf("It's child process\n");
			char* argv[2] = {&newProcessSymbol, NULL};
			execv("child_lin", argv);
			break;
		}
		default : {
			pi->pid = newProcessPid;
			printf("Created child %c with pid %d\n", newProcessSymbol, newProcessPid);
			return pi;
		}
	}
	return pi;
}

void killProcess(PI* pi) {
	kill(pi->pid, SIGTERM);
	int exitCode = 0;
	waitpid(pi->pid, &exitCode, 0);	
	newProcessSymbol--;
}

void initNewChild() {
	PI* newChild = createProcess();
	zox_push(&childStackTop, newChild);	
}

void killLastChild() {
	if (childStackTop != NULL) {
		PI* lastChild = (PI*)zox_pop(&childStackTop);
		killProcess(lastChild);
	} else {
		printf("There is no any child yet\n");
	}
}

void killChildren() {
	while(childStackTop != NULL) {
		killLastChild();
	}
	semctl(semID, 0, IPC_RMID);
}

void test() {
	printf("Test success\n");
	return;
}