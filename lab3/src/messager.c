#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "messager.h"

short int isFirst = 1;

int main(int argc, char* argv[]) {
	setvbuf(stdout, NULL, _IONBF, 0);
	if (argc > 1 && strcmp(argv[1], "-s") == 0) {
		printf("Second messager has been initialized\n");
		isFirst = 0;
	} else {
		printf("First messager has been initialized\n");
	}
	initialize();
	printf("Waining for connection...\n");
	establishConnection();
	printf("Connection establised\n");	

	while(1) {
		printf("Enter message: ");
		char message[BUFFER_SIZE];
		fgets(message, BUFFER_SIZE-1, stdin);
		sendMessage(message);
	} 
	return 0;
}