#include <stdlib.h>
#include <stdio.h>
#include <windows.h>
#include "messager.h"
#include "common.h"

int main(int argc, char* argv[]) {
	unsigned short isFirst;
	HANDLE acceptMessage;
	if (argc > 1 && strcmp(argv[1], "-s") == 0) {
		isFirst = 0;
		acceptMessage = OpenEvent(
			EVENT_ALL_ACCESS,	
			FALSE,	
			"secondGotMessage");
	} else {
		isFirst = 1;
		acceptMessage = OpenEvent(
			EVENT_ALL_ACCESS,	
			FALSE,	
			"firstGotMessage");
	}
	HANDLE firstSemaphore = CreateSemaphore(NULL, 0, 1, FIRST_SEM);
    if (firstSemaphore == INVALID_HANDLE_VALUE) {
        printf("Cannot open first semaphore at messageListener, exit 2\n");
        exit(2);
    }

    HANDLE secondSemaphore = CreateSemaphore(NULL, 0, 1, SECOND_SEM);
    if (secondSemaphore == INVALID_HANDLE_VALUE) {
        printf("Cannot open second semaphore at messageListener, exit 2\n");
        CloseHandle(firstSemaphore);
        exit(2);
    }

    HANDLE messagePipe;
    if (isFirst) {
    	messagePipe = CreateFile(SECOND_PIPE,
            GENERIC_READ,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL
            );
    } else {
    	messagePipe = CreateFile(FIRST_PIPE,
            GENERIC_READ,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL
            );
    } 
     if (messagePipe == INVALID_HANDLE_VALUE) {
     	printf("%ld\n", GetLastError());
        printf("Cannot open pipe at messageListener, exit 2\n");
        exit(2);
    }
    while(1) {
    	if (isFirst) {
			WaitForSingleObject(firstSemaphore, INFINITE);
    	} else {
    		WaitForSingleObject(secondSemaphore, INFINITE);
    	}
    	char message[BUFFER_SIZE];            
	    long unsigned int readBytes;
	    ReadFile(messagePipe, message, BUFFER_SIZE, &readBytes, NULL);
	    printf("\nGot message: %s", message);   
	    printf("Enter message: ");
    	SetEvent(acceptMessage);
    }
	return 0;
}