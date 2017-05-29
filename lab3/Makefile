os:=undefined

ifeq (${OS},Windows_NT)
	os:=win
	specialDependencies=${buildFolder}/win/messageListener
else
	os:=lin
endif

srcFolder:=src
buildFolder:=build

CFLAGS:=-Wall -pedantic -std=c11 -g -I${srcFolder} -Isrc

lab3: ${buildFolder}/${os}/messager ${specialDependencies}

${buildFolder}/${os}/messager: ${srcFolder}/messager.c ${srcFolder}/${os}/messager_${os}.c 
	gcc ${CFLAGS} $^ -o $@

${buildFolder}/win/messageListener: ${srcFolder}/win/messageListener.c
	gcc ${CFLAGS} $^ -o $@
