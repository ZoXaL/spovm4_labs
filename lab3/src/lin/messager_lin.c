#define _GNU_SOURCE
#define _POSIX_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include "messager_lin.h"
#include "messager.h"

pid_t messageReceiverID;

int sharedMemoryID;
int semaphoresID;

char* sharedMemory;

extern short int isFirst;

void ctrlCHandler(int signal) {

	kill(messageReceiverID, SIGTERM);
	int returnCode = 0;
	waitpid(messageReceiverID, &returnCode, 0);
	if (isFirst) {
		semop(semaphoresID, 0, IPC_RMID);
		shmdt(sharedMemory);
		struct shmid_ds sdstruct;		
		shmctl(sharedMemoryID, IPC_RMID, &sdstruct);	// free shared memory
	}
	printf("Messager has been gracefully closed\n");
	exit(0);
}

void messageRecieverLife() {
	struct sembuf receiveMessage;
	receiveMessage.sem_num = (isFirst) ? 0 : 1;
	receiveMessage.sem_op = -1;
	receiveMessage.sem_flg = 0;

	// TODO: initilize semaphore set of 2 sems
	// 1: message send by first; 2: message send by second
	while(1) {
		semop(semaphoresID, &receiveMessage, 1);
		printf("\nGot message: %s", sharedMemory);
		printf("Enter message: ");
	}; 
}

void establishConnection() {
	struct sembuf releaseSemaphore;
	releaseSemaphore.sem_num = (isFirst) ? 0 : 1;
	releaseSemaphore.sem_op = 1;
	releaseSemaphore.sem_flg = 0;

	struct sembuf takeSemaphore;
	takeSemaphore.sem_num = (isFirst) ? 1 : 0;
	takeSemaphore.sem_op = -1;
	takeSemaphore.sem_flg = 0;

	semop(semaphoresID, &releaseSemaphore, 1);
	semop(semaphoresID, &takeSemaphore, 1);

	// ----- Set up message receiver -----
	messageReceiverID = fork();
	switch (messageReceiverID) {
		case -1 : {
			printf("Can not initialize message receiver, exit 2\n");
			if (isFirst) {
				semop(semaphoresID, 0, IPC_RMID);
				shmdt(sharedMemory);
				struct shmid_ds sdstruct;		
				shmctl(sharedMemoryID, IPC_RMID, &sdstruct);	// free shared memory
			}
			exit(2);
			break;
		}
		case 0 : {
			// message receiver
			messageRecieverLife();
			exit(0);
		}
		default : {
			printf("Successfully initialized message receiver with pid %d\n", messageReceiverID);
		}
	}	
}

void sendMessage(char* message) {
	struct sembuf sendMessage;
	sendMessage.sem_num = (isFirst) ? 1 : 0;
	sendMessage.sem_op = 1;
	sendMessage.sem_flg = 0;

	strcpy(sharedMemory, message);
	semop(semaphoresID, &sendMessage, 1);
}



void initialize() {
	// ----- Set up CTRL-C hook -----
	struct sigaction ctrlCAction;
	memset(&ctrlCAction, '\0', sizeof(ctrlCAction));
	ctrlCAction.sa_handler = &ctrlCHandler;
	ctrlCAction.sa_flags = 0;
	if (sigaction(SIGINT, &ctrlCAction, NULL) < 0) {
		fprintf(stderr, "ERROR: Cannot set ctrl-c handler. Exit 1\n");
		exit(1);
	}
	// ----------------------------------

	// ----- Setup semaphores and shared memory -----
	// 1) 2 semaphores for messages and establising connections
	// 2) shared memory
	long int pseudoUniqueSemID = ftok(ANY_FILE, SEM_PROJ);
	semaphoresID = semget(pseudoUniqueSemID, 2, IPC_CREAT | 0666);
	int e = errno;
	if (semaphoresID == -1) {
		fprintf(stderr, "ERROR: Cannot get semaphores. Exit with errno\n");
		exit(e);
	}

	long int pseudoUniqueShmID = ftok(ANY_FILE, SHM_PROJ);
	sharedMemoryID = shmget(pseudoUniqueShmID, BUFFER_SIZE, IPC_CREAT | 0666);
	e = errno;
	if (semaphoresID == -1) {
		fprintf(stderr, "ERROR: Cannot get shared memory. Exit with errno\n");
		exit(e);
	}
	sharedMemory = shmat(sharedMemoryID, NULL, 0);
}