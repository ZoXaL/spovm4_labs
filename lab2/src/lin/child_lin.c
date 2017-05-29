#define _POSIX_SOURCE
#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <errno.h>

int semID;
struct sembuf releaseSemaphore;
struct sembuf takeSemaphore;
char c;


void killHandler(int signal) {
	printf("Process %c id killed\n", c);
	//perror("Ok");
	if (errno == 0 && errno != EINTR) {
		// Killed during working
		perror("All ok");				
		printf("Killed process was working\n");
		errno = 0;
		semop(semID, &releaseSemaphore, 1);
	} 
	exit(0);
	return;
}

int main(int argc, char* argv[]) {
	c = *argv[0];
	long int pseudoUniqueID = 211232113;
	semID = semget(pseudoUniqueID, 0, 0666);
	if (semID == -1) {
		printf("Cannot get parent semaphore, exit 3\n");
		exit(3);
	}
	releaseSemaphore.sem_num = 0;
	releaseSemaphore.sem_op = 1;
	releaseSemaphore.sem_flg = 0;

	takeSemaphore.sem_num = 0;
	takeSemaphore.sem_op = -1;
	takeSemaphore.sem_flg = 0;

	// ----------initializing parent handler----------
	struct sigaction killAction;
	memset(&killAction, '\0', sizeof(killAction));
	killAction.sa_handler = &killHandler;
	killAction.sa_flags = 0;
	if (sigaction(SIGTERM, &killAction, NULL) <= -1) {
		printf("Cannot set handler for SIGTERM\n"); 
	}
	
	while(1) {	
		semop(semID, &takeSemaphore, 1);
		for (int i = 0; i < 6; i++) {
			printf("%c", c);
		}
		printf("\n");
		semop(semID, &releaseSemaphore, 1);			
	}
	
		
	return 0;
}

