#ifndef INETERFACE_H
#define INETERFACE_H

// low-level process management
struct processInformation;
typedef struct processInformation PI;
PI* createProcess();
void killProcess(PI*);

// high-level process management
void initChildrenHandler();
void initNewChild();
void killLastChild();
void killChildren();

void test();

#endif