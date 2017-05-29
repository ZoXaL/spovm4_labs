#include "func.h"

int main() {
	struct processInfo* processInfo = createProcess();
	waitForProcess(processInfo);
	terminate();
	return 0;
}