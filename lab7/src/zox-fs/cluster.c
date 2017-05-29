#include <memory.h>
#include <zox-fs/zox-fs.h>
#include <errno.h>
#include "errors.h"


cluster_t get_subnode(fs_context_t context,
					  cluster_t directory,
					  const char *subnode_name) {
	if (directory->type != CLUSTER_TYPE_DIRECTORY) return NULL;
#ifdef DEBUG_MODE
	printf("finding subnode %s under %s, that has total %ld subnodes\n", subnode_name, directory->name, directory->directory.subnodes_count);
#endif
	for (int i = 0; i < directory->directory.subnodes_count; i++) {
		long int offset = directory->directory.subnodes[i];
		if (strcmp(cluster(context, offset).name, subnode_name) == 0) {
			return &(cluster(context, offset));
		}
	}
	return NULL;
}

cluster_t get_free_cluster(fs_context_t context) {
	for (int i = 0; i < FS_FILE_CLUSTERS_COUNT; i++) {
		if (cluster(context, i).type == CLUSTER_TYPE_EMPTY) {
			return &(cluster(context, i));
		}
	}
#ifdef DEBUG_MODE
	printf("out of memory:(\n");
#endif
	return NULL;
}

bool validate_file_name(const char* filename) {
	return (strlen(filename) != 0
			&& strstr(filename, "..") == NULL
			&& strstr(filename, " ") == NULL);
}

int flush_cluster(fs_context_t context, cluster_t cluster) {
#ifdef DEBUG_MODE
	printf("flushing cluster: %ld\n", cluster->offset);
#endif
	FILE* file = context->fs_file;
	fseek(file, cluster->offset * CLUSTER_SIZE, SEEK_SET);
	if (fwrite(cluster, CLUSTER_SIZE, 1, file) != 1) {
#ifdef DEBUG_MODE
		printf("Cannot update fs file: %s", strerror(errno));
#endif
		return E_FLUSH_ERROR;
	}
	return CALL_SUCESS;
}

cluster_t cluster_by_name(fs_context_t context, const char *name) {
	if (!validate_file_name(name)) {
		return NULL;
	}
	cluster_t file = context->_working_directory;

	char file_path_copy[MAX_NAME_LENGTH];
	strcpy(file_path_copy, name);
	char* tmp;

	if (file_path_copy[0] == '/') {
		file = &fs_root(context);
		strcpy(file_path_copy, file_path_copy + 1);		// got first '/'
	}
	tmp = strtok(file_path_copy, "/");
	while (tmp != NULL) {
		file = get_subnode(context, file, tmp);
		if (file == NULL) return NULL;
		tmp = strtok(NULL, "/");
	}
	return file;
}

int create_cluster(fs_context_t context, const char *cluster_path, cluster_t* retval) {
	if (!validate_file_name(cluster_path)) {
		return E_INVALID_FILE_NAME;
	}
	cluster_t parent_dir = context->_working_directory;
	char cluster_path_copy[MAX_NAME_LENGTH];
	strcpy(cluster_path_copy, cluster_path);
	char parent_clust_name[MAX_NAME_LENGTH];
	char new_clust_name[MAX_NAME_LENGTH];
	char* tmp = NULL;

	if (cluster_path[0] == '/') {
		parent_dir = &fs_root(context);
		strcpy(cluster_path_copy, cluster_path_copy + 1);		// got first '/'
	}

	tmp = strtok(cluster_path_copy, "/");
	strcpy(new_clust_name, tmp);
	strcpy(parent_clust_name, tmp);
	while ((tmp = strtok(NULL, "/")) != NULL) {
		parent_dir = get_subnode(context, parent_dir, parent_clust_name);
		if (parent_dir == NULL) return E_NOT_EXISTS;
		strcpy(new_clust_name, tmp);
		strcpy(parent_clust_name, new_clust_name);
	}
	if (strlen(new_clust_name) == 0) return E_INVALID_FILE_NAME;

	if (get_subnode(context, parent_dir, new_clust_name) != NULL) return E_DUPLICATE;

	(*retval) = get_free_cluster(context);
	if ((*retval) == NULL) return E_FS_FULL;
#ifdef DEBUG_MODE
	printf("Creating new cluster \'%s\' under %s\n", new_clust_name, parent_dir->name);
#endif
	strcpy((*retval)->name, new_clust_name);
	(*retval)->type = CLUSTER_TYPE_EMPTY;
	add_direcotory_subnode(parent_dir, *retval);

	int error_code = flush_cluster(context, (*retval));
	if(error_code != CALL_SUCESS) return error_code;

	error_code = flush_cluster(context, parent_dir);
	if(error_code != CALL_SUCESS) return error_code;
	return CALL_SUCESS;
}

void add_direcotory_subnode(cluster_t directory, cluster_t subnode) {
	subnode->parent = directory->offset;
	directory->directory.subnodes[directory->directory.subnodes_count] = subnode->offset;
	directory->directory.subnodes_count++;
}

void remove_direcotory_subnode(fs_context_t context, cluster_t subnode) {
	cluster_t directory = &cluster(context, subnode->parent);
	for (int i = 0; i < directory->directory.subnodes_count; i++) {
		if (directory->directory.subnodes[i] == subnode->offset) {
			for (int j = i; j < directory->directory.subnodes_count-1; j++) {
				directory->directory.subnodes[j] = directory->directory.subnodes[j+1];
			}
			break;
		}
	}
	directory->directory.subnodes_count--;
}

void split_absolute_path(const char *absolute_path, char *directory, char *subnode_name) {
	if (absolute_path == NULL || directory == NULL || subnode_name == NULL) return;
	char* last_slash = strchr(absolute_path, '/');
	char* tmp;
	while((tmp = strchr(last_slash+1, '/')) != NULL) last_slash = tmp;
	size_t last_slash_index = last_slash - absolute_path;
	for (size_t i = 0; i <= last_slash_index; i++) {
		directory[i] = absolute_path[i];
	}
	for (size_t i = last_slash_index+1; i <= strlen(absolute_path); i++) {
		subnode_name[i - (last_slash_index+1)] = absolute_path[i];
	}
}

void to_absolute_path(fs_context_t context, char* path) {
	if (path[0] != '/') {
		char absolute_path[MAX_NAME_LENGTH];
		strcpy(absolute_path, zox_fs_pwd_s(context));
		if (context->_working_directory->offset != 0) {
			strcat(absolute_path, "/");
		}
		strcat(absolute_path, path);
		strcpy(path, absolute_path);
	}
}