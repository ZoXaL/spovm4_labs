#include <stdlib.h>
#include <stdio.h>
#include <windows.h>
#include <string.h>
#include "common.h"
#include "messager.h"

extern unsigned short isFirst;

HANDLE firstSemaphore;
HANDLE secondSemaphore;
HANDLE messagePipe;
PROCESS_INFORMATION messageListener;
HANDLE acceptMessageEvent;

BOOL WINAPI ctrlCHandler(DWORD signal) {
    if (isFirst && signal == CTRL_C_EVENT) {        
        CloseHandle(firstSemaphore);        
        CloseHandle(secondSemaphore);               
    }
    DisconnectNamedPipe(messagePipe);
    CloseHandle(messagePipe);
    TerminateProcess(messageListener.hProcess, 0);
    printf("Server was gracefully closed\n"); // do cleanup
    exit(0);
    return 1;
}


void establishConnection() {
    if (isFirst) {
        ReleaseSemaphore(secondSemaphore, 1, NULL);
        WaitForSingleObject(firstSemaphore, INFINITE);
        acceptMessageEvent = CreateEvent(
                NULL , 
                TRUE,   
                FALSE , 
                "secondGotMessage");
    } else {
        ReleaseSemaphore(firstSemaphore, 1, NULL);
        WaitForSingleObject(secondSemaphore, INFINITE);
        acceptMessageEvent = CreateEvent(
                NULL , 
                TRUE,   
                FALSE , 
                "firstGotMessage");
    }

    STARTUPINFO si; 
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);

    char commandLine[80] = "messageListener -f";
    if (!isFirst) commandLine[17] = 's';
    if (!CreateProcess(
                NULL,           //module name
                commandLine, 
                NULL,            // Process handle not inheritable
                NULL,           // Thread handle not inheritable
                FALSE,          // Set handle inheritance to FALSE
                NORMAL_PRIORITY_CLASS, //class of creation
                NULL,   //new env var block
                NULL,   //current catalog
                &si,   //child window parameters
                &messageListener    //process infomation
            )
        )   {       
        printf("ERROR: Cannot create message listener process. Exit 3\n");
        exit(3);
    }
    
    ConnectNamedPipe(messagePipe, NULL);
}

void sendMessage(char* message) {
    long unsigned int writtenBytes;
    WriteFile(messagePipe, message, BUFFER_SIZE, &writtenBytes, NULL);
    if (isFirst) {
        ReleaseSemaphore(secondSemaphore, 1, NULL);
    } else {
        ReleaseSemaphore(firstSemaphore, 1, NULL);
    }
    // printf("Waiting for acception...\n");
    WaitForSingleObject(acceptMessageEvent, INFINITE);
    ResetEvent(acceptMessageEvent);
    // printf("Got acception...\n");
}
void initialize() {
    // init semaphores
    // init shared memory
     if (!SetConsoleCtrlHandler(ctrlCHandler, TRUE)) {
        printf("\nERROR: Could not set control handler. Exit 1\n");
        exit(1); 
    }
    firstSemaphore = CreateSemaphore(NULL, 0, 1, FIRST_SEM);
    if (firstSemaphore == INVALID_HANDLE_VALUE) {
        printf("Cannot create first semaphore, exit 2\n");
        exit(2);
    }

    secondSemaphore = CreateSemaphore(NULL, 0, 1, SECOND_SEM);
    if (secondSemaphore == INVALID_HANDLE_VALUE) {
        printf("Cannot create pipe, exit 2\n");
        CloseHandle(firstSemaphore);
        exit(2);
    }
    
    if (isFirst) {
        printf("Creating pipe\n");
        messagePipe = CreateNamedPipe(FIRST_PIPE, 
                PIPE_ACCESS_OUTBOUND,
                PIPE_TYPE_BYTE,
                4,
                BUFFER_SIZE,
                BUFFER_SIZE,
                0,
                NULL);
        if (messagePipe == INVALID_HANDLE_VALUE) {
            printf("Cannot create pipe, exit 2\n");
            CloseHandle(firstSemaphore);
            CloseHandle(secondSemaphore);
            exit(2);
        }
    } else {
        printf("Creating pipe\n");
        messagePipe = CreateNamedPipe(SECOND_PIPE, 
                PIPE_ACCESS_OUTBOUND,
                PIPE_TYPE_BYTE,
                4,
                BUFFER_SIZE,
                BUFFER_SIZE,
                0,
                NULL);
        if (messagePipe == INVALID_HANDLE_VALUE) {
            printf("Cannot create pipe, exit 2\n");
            CloseHandle(firstSemaphore);
            CloseHandle(secondSemaphore);
            exit(2);
        }
    }
}