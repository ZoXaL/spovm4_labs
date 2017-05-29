#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include "zox-fs.h"
#include "repl.h"
#include "errors.h"

static void _ls(fs_context_t context) {
	long int subnodes_count = zox_fs_pwd(context)->directory.subnodes_count;
	if (subnodes_count == 0) {
		return;
	}
	cluster_t* subnodes_array = (cluster_t*)malloc(subnodes_count
												   * sizeof(struct cluster_t));
	zox_fs_ls(context, subnodes_array);
	for (long int i = 0; i < subnodes_count; i ++) {
		if (subnodes_array[i]->type == CLUSTER_TYPE_FILE) {
			printf("%s \t %ldB\n", subnodes_array[i]->name,
				   zox_fs_fsize(context, subnodes_array[i]));
		} else if (subnodes_array[i]->type == CLUSTER_TYPE_DIRECTORY) {
			printf("%s\n", subnodes_array[i]->name);
		}
	}
	free(subnodes_array);
}

static void _pwd(fs_context_t context) {
	char* wd_name = zox_fs_pwd_s(context);
	printf("%s\n", wd_name);
	free(wd_name);
}

static void _clustmap(fs_context_t context) {
	int i;
	for (i = 0; i < FS_FILE_CLUSTERS_COUNT; i++) {
		if (context->_fs[i].type == CLUSTER_TYPE_EMPTY) printf("-");
		if (context->_fs[i].type == CLUSTER_TYPE_FILE) printf("f");
		if (context->_fs[i].type == CLUSTER_TYPE_DIRECTORY) printf("d");
		if (i > 0 && i % 80 == 0) printf("\n");
	}
	if (i % 80 != 1) printf("\n");
}

static void _free(fs_context_t context) {
	printf("available space: %ld\n", zox_fs_free(context));
}

static void _cd(fs_context_t context, char* directory_path) {
	int result_code = zox_fs_cd(context, directory_path);
	if (result_code == E_NOT_EXISTS || result_code == E_INVALID_FILE_NAME) {
		printf("Invalid cd path\n");
	}
}

static void _rm(fs_context_t context, char* file_name) {
	int result_code = zox_fs_rm_s(context, file_name);
	if (result_code == E_NOT_EXISTS) {
		printf("Invalid cd path\n");
	} else if (result_code == E_INVALID_FILE_NAME) {
		printf("Cannot remove \'%s\'\n", file_name);
	}
}

static void _mkdir(fs_context_t context, char* dir_name) {
	int result_code = zox_fs_mkdir(context, dir_name);
	if (result_code == E_DUPLICATE) {
		printf("There is already file with such name\n");
	} else if (result_code == E_INVALID_FILE_NAME) {
		printf("Invalid directory name\n");
	} else if (result_code == E_FS_FULL) {
		printf("Out of memory\n");
	} else if (result_code == E_FLUSH_ERROR) {
		printf("Can not update fs file: changes will be lost\n");
	} else if (result_code == E_NOT_EXISTS) {
		printf("Wrong directory path\n");
	}
}

static void _load(fs_context_t context,
				  const char* source, char* destination) {
#ifdef DEBUG_MODE
	printf("Loading file from \'%s\' to \'%s\'\n", source, destination);
#endif
	int load_result = zox_fs_load_file(context, source, destination);
	if (load_result == E_FS_FULL) {
		printf("Out of memory\n");
	} else if (load_result == E_OPEN_FILE) {
		printf("Source file doesn't exists\n");
	}
}

static void _extract(fs_context_t context,
					 const char* source, const char* destination) {
#ifdef DEBUG_MODE
	printf("Extracting file from \'%s\' to \'%s\'\n", source, destination);
#endif
	int extract_result = zox_fs_extract_file(context, source, destination);
	if (extract_result == E_OPEN_FILE) {
		printf("Cannot open destination file\n");
	} else if (extract_result == E_NOT_EXISTS) {
		printf("Source file doesnt' exists\n");
	}
}

static void _mv(fs_context_t context, char* from, char* to) {
	int result_code = zox_fs_mv(context, from, to);
	if (result_code == E_SOURCE_NOT_EXISTS) {
		printf("Source does not exist\n");
	} else if (result_code == E_DEST_NOT_EXISTS) {
		printf("Destination path does not exist\n");
	} else if (result_code == E_DUPLICATE) {
		printf("Destination already exists\n");
	}
}

static void _cp(fs_context_t context, const char* from, char* to) {
	int result_code = zox_fs_cp(context, from, to);
	if (result_code == E_SOURCE_NOT_EXISTS) {
		printf("Source file does not exist\n");
	} else if (result_code == E_DUPLICATE) {
		printf("Destination file already exists\n");
	} else if (result_code == E_FS_FULL) {
		printf("Out of memory\n");
	} else if (result_code == E_DEST_NOT_EXISTS) {
		printf("Destination path does not exists\n");
	}
}


static void _help() {
	printf("ls \t\t- list elements in working directory\n");
	printf("pwd \t\t- print working directory\n");
	printf("cd \t\t- change working directory\n");
	printf("rm \t\t- remove file or directory\n");
	printf("mkdir \t\t- create directory\n");
	printf("load \t\t- load file from host filesystem\n");
	printf("extract \t- extract file to host filesystem\n");
	printf("mv \t\t- move fs element from one place to another\n");
	printf("cp \t\t- copy file from one place ot another\n");
	printf("free \t\t- see available fs memory\n");
	printf("clustmap \t- print fs clusters\n");
	printf("help \t\t- print this help\n");
}

void start_repl(fs_context_t context) {
	while (1) {
		char* wd_name = zox_fs_pwd_s(context);
		printf("%s>", wd_name); fflush(stdout);
		free(wd_name);

		char command[MAX_COMMAND_NAME_LENGTH];

		memset(command, '\0', MAX_COMMAND_NAME_LENGTH);
		fgets(command, MAX_COMMAND_NAME_LENGTH, stdin);

		command[strlen(command)-1] = '\0';
		char* first_command_token = strtok(command, " ");
		if (first_command_token == NULL) continue;

		if (strcmp(first_command_token, "ls") == 0) {
			_ls(context);
		} else if (strcmp(first_command_token, "pwd") == 0) {
			_pwd(context);
		} else if (strcmp(first_command_token, "clustmap") == 0) {
			_clustmap(context);
		} else if (strcmp(first_command_token, "rm") == 0
					|| strcmp(first_command_token, "mkdir") == 0
					|| strcmp(first_command_token, "cd") == 0) {
			char first_command_token_copy[MAX_NAME_LENGTH];
			strcpy(first_command_token_copy, first_command_token);
			char* what_ptr = strtok(NULL, " ");
			if (what_ptr == NULL) {
				printf("Usage: %s [what] \n", first_command_token_copy);
				continue;
			}
			if ((strcmp(first_command_token_copy, "rm") == 0)) {
				_rm(context, what_ptr);
			} else if ((strcmp(first_command_token_copy, "mkdir") == 0)) {
				_mkdir(context, what_ptr);
			} else if ((strcmp(first_command_token_copy, "cd") == 0)) {
				_cd(context, what_ptr);
			}
		} else if (strcmp(first_command_token, "free") == 0) {
			_free(context);
		} else if (strcmp(first_command_token, "load") == 0
					|| strcmp(first_command_token, "extract") == 0
					|| strcmp(first_command_token, "mv") == 0
					|| strcmp(first_command_token, "cp") == 0) {
			char first_command_token_copy[MAX_NAME_LENGTH];
			strcpy(first_command_token_copy, first_command_token);

			char source[MAX_COMMAND_NAME_LENGTH];
			char* source_ptr = strtok(NULL, " ");
			if (source_ptr == NULL) {
				printf("Usage: %s [source] [destination]\n", first_command_token_copy);
				continue;
			}
			strcpy(source, source_ptr);

			char* destination_ptr = strtok(NULL, " ");
			if (destination_ptr == NULL) {
				printf("Usage: %s [source] [destination]\n", first_command_token_copy);
				continue;
			}

			if ((strcmp(first_command_token_copy, "load") == 0)) {
				_load(context, source, destination_ptr);
			} else if ((strcmp(first_command_token_copy, "extract") == 0)) {
				_extract(context, source, destination_ptr);
			} else if ((strcmp(first_command_token_copy, "mv") == 0)) {
				_mv(context, source, destination_ptr);
			} else if ((strcmp(first_command_token_copy, "cp") == 0)) {
				_cp(context, source, destination_ptr);
			}
		} else if (strcmp(first_command_token, "help") == 0) {
			_help();
		}  else if (strcmp(first_command_token, "exit") == 0) {
			return;
		} else {
			printf("Unknown command \'%s\', %s",
				   command, "print \'help\' to see available commands\n");
		}
	}
}