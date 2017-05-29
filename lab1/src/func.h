#ifndef FUNC_H
#define FUNC_H

struct processInfo;
struct processInfo* createProcess();
void waitForProcess(struct processInfo*);
void terminate();

#endif