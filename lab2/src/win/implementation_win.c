#include "win_private.h"
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>


PI* childManager;
HANDLE killManagerEvent;
HANDLE createChildEvent;
HANDLE killChildEvent;
HANDLE testEvent;

// high-level abstract process management
void initChildrenHandler() {
	// event initializing
	killManagerEvent = CreateEvent(
		NULL , 	// security flags ?
		TRUE, 	// manual event control
		FALSE , // initial state
		"killManager"); // unique name
	createChildEvent = CreateEvent(
		NULL , 
		TRUE, 	
		FALSE , 
		"createChild");
	killChildEvent = CreateEvent(
		NULL , 	
		TRUE, 	
		FALSE , 
		"killChild");
	testEvent = CreateEvent(
		NULL , 	
		TRUE, 	
		FALSE , 
		"test");
	if (killManagerEvent == NULL || createChildEvent == NULL || killChildEvent == NULL) {
		printf("Cannot initialize events, exit 1\n");
		exit(1);
	}

	childManager = (PI*)malloc(sizeof(PI));	
	ZeroMemory(&childManager->si, sizeof(childManager->si));
	childManager->si.cb = sizeof(childManager->si);

	if (!CreateProcess(
				NULL, 	//module name
				"childManager_win", 
				NULL, 	// process attr
				NULL, 	// thread attr
				FALSE, 	//inherit descriptor
				NORMAL_PRIORITY_CLASS, //class of creation
				NULL, 	//new env var block
				NULL, 	//current catalog
				&(childManager->si), 	//child window parameters
				&(childManager->pi) 	//process infomation
			)
		)	{	
		printf("Cannot create manager process, exit 1\n");	
		free(childManager);
		exit(1);
	}
}

void initNewChild() {
	SetEvent(createChildEvent);  
}

void killLastChild() {
	SetEvent(killChildEvent);
}

void test() {
	printf("Sending test event\n");
	SetEvent(testEvent);
}

void killChildren() {
	if (!SetEvent(killManagerEvent)) {
		printf("Cannot set killManagerEvent\n");
		return;
	}
	WaitForSingleObject(childManager->pi.hProcess, INFINITE);
	CloseHandle(killManagerEvent); 
	CloseHandle(createChildEvent); 
	CloseHandle(killChildEvent); 
	CloseHandle(childManager->pi.hProcess); 
	CloseHandle(childManager->pi.hThread);
	free(childManager);
}
