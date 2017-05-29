#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <time.h>
#include "func.h"

struct processInfo {
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
};

struct processInfo* createProcess() {
	printf("Creating process\n");
	fflush(stdout);
	struct processInfo* data;
	data = (struct processInfo*)malloc(sizeof(struct processInfo));
	ZeroMemory(&data->si, sizeof(data->si));
	data->si.cb = sizeof(data->si);
	if (!CreateProcess(
				NULL, 	//module name
				"d:\\BSUIR\\4sem\\SPOVM\\lab1\\obj\\win\\child_win.exe", 
				NULL, 	//command line
				NULL, 	//security processor flags
				FALSE, 	//inherit descriptor
				NORMAL_PRIORITY_CLASS, //class of creation
				NULL, 	//new env var block
				NULL, 	//current catalog
				&(data->si), 	//child window parameters
				&(data->pi) 	//process infomation
			)
		)	{		
		return NULL;
	}
	printf("Created process with pid %lu\n", data->pi.dwProcessId);
	printf("Parent pid: %lu\n", GetCurrentProcessId());

	for (int i = 0; i < 4; i++) {
			printf("p:i = %d\n", i);
			fflush(stdout);
			Sleep(500);
		}
	return data;
}
void waitForProcess(struct processInfo* process) {
	printf("Waiting for child\n");
	fflush(stdout);
	if (!process) {
		printf("Undefined process");
		return;
	}
	WaitForSingleObject(process->pi.hProcess, INFINITE);
	CloseHandle(process->pi.hProcess); 
	CloseHandle(process->pi.hThread);
}
void terminate() {
	time_t t;
	time(&t);
	printf("Terminated parent. Exit time: %s\n", ctime(&t));
}