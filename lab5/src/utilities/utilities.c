#include <string.h>
#include <stdio.h>

int string_ends_with(const char * str, const char * suffix) {
  int str_len = strlen(str);
  int suffix_len = strlen(suffix);
  if (str_len < suffix_len) return 0;
  return (strcmp(str + (str_len-suffix_len), suffix) == 0);
}

void print_progress_bar(const char* label, int percent) {
	int PERCENTS_BY_STEP = 5;
	int step = percent / PERCENTS_BY_STEP;
	int total_steps = 100 / PERCENTS_BY_STEP;

	printf("%s: ", label);
	printf("[");
	for (int i = 0; i < step; i++) {
		printf("|");
	}
	for (int i = 0; i < total_steps - step; i++) {
		printf("_");
	}
	printf("] %d %%", percent);
	printf("\n");
}