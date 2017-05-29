srcFolder:=src
buildFolder:=build
glib-include:=${shell pkg-config --cflags glib-2.0}
glib-lib:=${shell pkg-config --libs glib-2.0}

CFLAGS:=-Wall -Werror -pedantic -std=gnu11 -g \
	-I${srcFolder} -I${srcFolder}/ring -I${srcFolder}/queue \
	-I${srcFolder}/tm -I${srcFolder}/child 

${buildFolder}/main: 	${srcFolder}/main.c ${srcFolder}/tm/tm.c \
						${srcFolder}/child/child.c ${buildFolder}/libring.a \
						${buildFolder}/libqueue.a
	gcc ${CFLAGS} -pthread ${srcFolder}/main.c ${srcFolder}/tm/tm.c \
		${srcFolder}/child/child.c -o $@ ${glib-include} \
		-L./${buildFolder} -Wl,-Bstatic -lring -lqueue -Wl,-Bdynamic ${glib-lib} -lpthread

${buildFolder}/libring.a: ${srcFolder}/ring/ring.c 
	gcc -c ${CFLAGS} $^ -o ${buildFolder}/ring.o
	ar rcs $@ ${buildFolder}/ring.o

${buildFolder}/libqueue.a: ${srcFolder}/queue/queue.c 
	gcc -c ${CFLAGS} $^ -o ${buildFolder}/queue.o
	ar rcs $@ ${buildFolder}/queue.o

run:
	@cd build&&./main