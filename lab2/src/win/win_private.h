#ifndef WIN_PRIVATE_H
#define WIN_PRIVATE_H

#include <windows.h>

struct processInformation {
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	HANDLE lifecycleEvent;
	HANDLE workflowEventChild;
	HANDLE workflowEventParent;
};

typedef struct processInformation PI;

#endif