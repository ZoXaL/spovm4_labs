#include "interface.h"
#include <stdio.h>

int main(int argc, char* argv[]) {  
	
	setbuf(stdout, NULL);
	
	initChildrenHandler();	

	int action;
	
	while(1) {
		action = getchar();
		while(action == 10) {
			action = getchar();
		}
		switch(action) {
			case '+' : {
				initNewChild();				
				break;
			}
			case '-' : {
				killLastChild();				
				break;
			}
			case 't' : {
				test();				
				break;
			}
			case 'q' : {
				killChildren();
				return 0;
			}
		}
	}
	return 0;
}