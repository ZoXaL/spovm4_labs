#include "win_private.h"
#include "interface.h"
#include "stack/stack.h"
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#define BASE_HANDLERS_COUNT 4

int childCount = 0;
HANDLE* handlersArray;

int main() {
	setbuf(stdout, NULL);

	ZSE* childStackTop = NULL;
	ZSE* currentWorkingChild = NULL;

	HANDLE killManagerEvent = OpenEvent(
		EVENT_ALL_ACCESS,	// desired address??
		FALSE,	// no inherit
		"killManager");
	HANDLE createChildEvent = OpenEvent(
		EVENT_ALL_ACCESS,	
		FALSE,	
		"createChild");
	HANDLE killChildEvent = OpenEvent(
		EVENT_ALL_ACCESS,	
		FALSE,	
		"killChild");
	HANDLE testEvent = OpenEvent(
		EVENT_ALL_ACCESS,	
		FALSE,	
		"test");
	if (killManagerEvent == NULL || createChildEvent == NULL || killChildEvent == NULL) {
		printf("Cannot get events from childManager, exit 1\n");
		exit(1);
	}
	HANDLE* handlersArray = (HANDLE*)malloc(sizeof(HANDLE)*(BASE_HANDLERS_COUNT));
	handlersArray[0] = killManagerEvent;
	handlersArray[1] = createChildEvent;
	handlersArray[2] = killChildEvent;
	handlersArray[3] = testEvent;
	
	while(1) {
		//printf("childManager loop, waiting for events\n");
		DWORD signaledEvent = WaitForMultipleObjects(childCount+BASE_HANDLERS_COUNT, handlersArray, FALSE, INFINITE);
		//printf("manager got signal %ld \n", signaledEvent);
		if (signaledEvent == WAIT_OBJECT_0) {
			//printf("Kill child manager event\n");
			while (childStackTop != NULL) {
				PI* topChild = (PI*)zox_pop(&childStackTop);
				killProcess(topChild);
			}
			printf("manager died\n");
			return 0;
		}
		if (signaledEvent == WAIT_OBJECT_0+1) {
			//printf("Create child event\n");
			ResetEvent(handlersArray[1]);	

			PI* newChild = createProcess();
			
			zox_push(&childStackTop, newChild);	
			if (childStackTop->previous == NULL) {
				SetEvent(newChild->workflowEventChild);
			}
			
			// refresh handlers handlersArrayray
			free(handlersArray);
			handlersArray = (HANDLE*)malloc(sizeof(HANDLE)*(childCount+BASE_HANDLERS_COUNT));
			if (handlersArray == NULL) {
				printf("Cannot malloc memory for handlersArray, exit 2\n");
				return 2;
			}
			handlersArray[0] = killManagerEvent;
			handlersArray[1] = createChildEvent;
			handlersArray[2] = killChildEvent;
			handlersArray[3] = testEvent;
			ZSE* zse = childStackTop;
			for (int i = 0; i < childCount; i++) {
				HANDLE* nextChildEvent = ((PI*)zse->data)->workflowEventParent;
				int place = i+BASE_HANDLERS_COUNT;
				handlersArray[place] = nextChildEvent;
				zse = zse->previous;
			}
			continue;	
		}
		if (signaledEvent == WAIT_OBJECT_0+2) {
			short killRunningFlag = 0;
			//printf("Kill child event\n");
			ResetEvent(handlersArray[2]);

			if (childStackTop == NULL) {
				printf("There is no child yet\n");
				fflush(stdout);
				continue;
			}			
			if (childStackTop == currentWorkingChild && childStackTop->previous != NULL) {
				killRunningFlag = 1;
			}
			PI* childToKill = (PI*)zox_pop(&childStackTop);

			killProcess(childToKill);
			
			// // refresh handlers handlersArrayray
			free(handlersArray); 
			handlersArray = (HANDLE*)malloc(sizeof(HANDLE)*(childCount+BASE_HANDLERS_COUNT));
			handlersArray[0] = killManagerEvent;
			handlersArray[1] = createChildEvent;
			handlersArray[2] = killChildEvent;
			handlersArray[3] = testEvent;
			ZSE* zse = childStackTop;
			for (int i = 0; i < childCount; i++) {
				handlersArray[i+BASE_HANDLERS_COUNT] = ((PI*)(zse->data))->workflowEventParent;
				zse = zse->previous;
			}	
			if (killRunningFlag == 1) {
				PI* pi = (PI*)childStackTop->data;
				ResetEvent(pi->workflowEventParent);
				currentWorkingChild = childStackTop;
				SetEvent(pi->workflowEventChild);
			}
			continue;	
		}
		if (signaledEvent == WAIT_OBJECT_0+3) {
			printf("ChildManager got test event\n");
			ResetEvent(handlersArray[3]);			
			continue;
		}

		// Control childs workflow
		DWORD processNumber = (signaledEvent-BASE_HANDLERS_COUNT);
		//printf("Message from child %ld\n", processNumber);

		// ----- reset event from signaled child -----
		ZSE* signaledChild = childStackTop;
		int i = processNumber;
		while (i>0) {
			i--;
			signaledChild = signaledChild->previous;
		}
		PI* pi = (PI*)signaledChild->data;
		ResetEvent(pi->workflowEventParent);
		// --------------------------------------------
		if (processNumber == childCount-1) {	//the last child
			
			currentWorkingChild = childStackTop;
			PI* pi = (PI*)currentWorkingChild->data;
			SetEvent(pi->workflowEventChild);
			continue;
		} else {
			ZSE* childToRun = childStackTop;
			while (processNumber>0) {
				processNumber--;
				childToRun = childToRun->previous;
			}
			childToRun = childToRun->previous;
			currentWorkingChild = childToRun;
			PI* pi = (PI*)currentWorkingChild->data;
			SetEvent(pi->workflowEventChild);
		}	
	}	
	return 0;
}

PI* createProcess() {
	//printf("Creating process\n");
	PI* childData;
	childData = (PI*)malloc(sizeof(PI));	
	
	ZeroMemory(&(childData->si), sizeof(childData->si));
	childData->si.cb = sizeof(childData->si);
	char commandLine[80] = "child_win a";
	char childName = 'a'+((childCount)%25);
	commandLine[10] = childName;
	if (!CreateProcess(
				NULL, 	//module name
				commandLine, 
				NULL ,           // Process handle not inheritable
		        NULL,           // Thread handle not inheritable
		        FALSE,          // Set handle inheritance to FALSE
				NORMAL_PRIORITY_CLASS, //class of creation
				NULL, 	//new env var block
				NULL, 	//current catalog
				&(childData->si), 	//child window parameters
				&(childData->pi) 	//process infomation
			)
		)	{		
		printf("Cannot create child process\n");
		fflush(stdout);
		exit(0);
		return NULL;
	}
	childCount++;
	// Process handles
	char childPid[80];
	sprintf(childPid, "%ld", childData->pi.dwProcessId);
	printf("Created child %c\n", childName);
	childData->workflowEventChild = CreateEvent(
		NULL , 
		TRUE, 	
		FALSE , 
		childPid);
	if (childData->workflowEventChild == NULL) {		
		printf("Cannot create workflow child event %s\n", childPid);
	}
	char childPid_2[80];
	sprintf(childPid_2, "%ld", (childData->pi.dwProcessId*2));
	childData->workflowEventParent = CreateEvent(
		NULL , 
		TRUE, 	
		FALSE , 
		childPid_2);
	if (childData->workflowEventChild == NULL) {		
		printf("Cannot create workflow parent event %s\n", childPid_2);
	}
	// printf("Created workflow parent event %s\n", childPid_2);
	char childPid_3[80];
	sprintf(childPid_3, "%ld", (childData->pi.dwProcessId*3));
	childData->lifecycleEvent = CreateEvent(
		NULL , 
		TRUE, 	
		FALSE , 
		childPid_3);
	if (childData->workflowEventChild == NULL) {		
		printf("Cannot create lifecycleEvent event %s\n", childPid_3);
	}
	return childData;
}

void killProcess(PI* process) {
	childCount--;
	SetEvent(process->lifecycleEvent);
	WaitForSingleObject(process->pi.hProcess, INFINITE);
	CloseHandle(process->workflowEventChild);
	CloseHandle(process->workflowEventParent);
	CloseHandle(process->lifecycleEvent);
	CloseHandle(process->pi.hProcess); 
	CloseHandle(process->pi.hThread);
	free(process);
}