#include <stdlib.h>
#include <string.h>

char* zox_strdup(const char* str) {
	char* dup = (char*)malloc(strlen(str));
	strcpy(dup, str);
	return dup;
} 