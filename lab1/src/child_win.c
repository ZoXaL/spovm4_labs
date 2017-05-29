#include <windows.h>
#include <stdio.h>
#include <time.h>

int main() {
	for (int i = 0; i < 8; i++) {
		printf("c:i = %d\n", i);
		fflush(stdout);
		Sleep(500);
	}
	time_t t;
	time(&t);
	printf("Terminated child. Exit time: %s\n", ctime(&t));
	return 0;
}