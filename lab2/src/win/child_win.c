#include <stdlib.h>
#include <windows.h>
#include <stdio.h>

int main(int argc, char* argv[]) {
	//Sleep(1000);
	setbuf(stdout, NULL);
	DWORD pidI = GetCurrentProcessId();
	char pidC[80];
	sprintf(pidC, "%ld", pidI);
	HANDLE workflowEventChild = OpenEvent (
		EVENT_ALL_ACCESS,	// rights
		FALSE,				// no inherit
		pidC);
	char pidC2[80];
	sprintf(pidC2, "%ld", pidI*2);
	HANDLE workflowEventParent = OpenEvent(
		EVENT_ALL_ACCESS,	
		FALSE,	
		pidC2);
	char pidC3[80];
	sprintf(pidC3, "%ld", pidI*3);
	HANDLE lifecycleEvent = OpenEvent(
		EVENT_ALL_ACCESS,	
		FALSE,	
		pidC3);
	HANDLE arr[2] = {workflowEventChild, lifecycleEvent};
	while(1) {
		DWORD event = WaitForMultipleObjects(2, arr, FALSE, INFINITE);
		//printf("Event in child: %ld\n", event);
		if (event == WAIT_OBJECT_0) {
			for (int i = 0; i < 3; i++) {
				printf("%s", argv[1]);
				fflush(stdout);
				if (WaitForSingleObject(lifecycleEvent, 1) != WAIT_TIMEOUT) {
					printf("Child %s exit\n", argv[1]);
					exit(0);
				}
			}			
			printf("\n");
			ResetEvent(workflowEventChild);
			SetEvent(workflowEventParent);
		} else {
			printf("Child %s exit\n", argv[1]);
			SetEvent(workflowEventParent);
			exit(0);
		}
	}
	return 0;
}