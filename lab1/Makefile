#os := undefined
dependencies := main.o func.o
CLFAGS := -g -std=c11 -Wall -pedantic 

ifeq ($(OS),Windows_NT)
    os := win
    dependencies := ${dependencies} child_win.o
else
    os := lin# $(shell uname -s)
endif

lab1: ${dependencies}
	@echo ${dependencies}
	gcc ${CLFAGS} -o ./obj/${os}/lab1 ./obj/${os}/main.o ./obj/${os}/func_${os}.o

main.o: ./src/main.c
	gcc -c ${CLFAGS} -o ./obj/${os}/main.o ./src/main.c

func.o: ./src/func_${os}.c ./src/func.h
	gcc -c ${CLFAGS} -o ./obj/${os}/func_${os}.o ./src/func_${os}.c

child_win.o : ./src/child_win.c
	gcc ${CLFAGS} -o ./obj/win/child_win.exe ./src/child_win.c