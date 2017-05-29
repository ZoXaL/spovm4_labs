#include <stdio.h>
#include <memory.h>
#include "zox-mem.h"

typedef struct student_t {
	char name[20];
	int age;
} student_t;


int main(int argc, char* argv[]) {
	char* string_1 = (char*)zox_memory.malloc(sizeof(char)*14);
	strcpy(string_1, "hello mike yo");
	printf("allocated string \"%s\", offset %ld\n",
		   string_1, get_internal_offset(string_1));

	char* string_2 = (char*)zox_memory.malloc(sizeof(char)*5);
	strcpy(string_2, "hell");
	printf("allocated string \"%s\", offset %ld\n",
		   string_2, get_internal_offset(string_2));

	string_2 = (char*)zox_memory.realloc(string_2, sizeof(char)*11);
	strcat(string_2, "o mike");
	printf("reallocated string \"%s\", offset %ld\n",
		   string_2, get_internal_offset(string_2));

	zox_memory.free(string_1);
	printf("first string was deleted\n");

	int* pipe = (int*)zox_memory.malloc(sizeof(int)*2);
	pipe[0] = 22;
	pipe[1] = 42;
	printf("allocated int array [%d, %d], offset %ld\n", pipe[0],
		   pipe[1], get_internal_offset(pipe));

	zox_memory.free(string_2);
	zox_memory.free(pipe);
	printf("All memory was freed\n");

	student_t* me = (student_t*)zox_memory.malloc(sizeof(student_t));
	strcpy(me->name, "Mikhail Chystsakou");
	me->age = 19;
	printf("allocated student [%s, %d], offset %ld\n", me->name,
		   me->age, get_internal_offset(me));

	//zox_memory.exit();
	printf("memory manager close\n");
	return 0;
}