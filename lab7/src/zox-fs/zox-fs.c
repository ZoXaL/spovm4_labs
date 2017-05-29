#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <zox-fs/cluster.h>
#include "zox-fs.h"
#include "errors.h"

int zox_fs_ls(fs_context_t context, cluster_t* array) {
	cluster_t wd = context->_working_directory;
	for (int i = 0; i < wd->directory.subnodes_count; i++) {
		array[i] = &cluster(context, wd->directory.subnodes[i]);
	}
	return CALL_SUCESS;
}

const cluster_t zox_fs_pwd(fs_context_t context) {
	return context->_working_directory;
}

char* zox_fs_pwd_s(fs_context_t context) {
	cluster_t* tree_path = (cluster_t*)malloc(sizeof(cluster_t));
	cluster_t wd = context->_working_directory;
	int path_size = 1;
	while (wd->parent != -1) {
		tree_path[path_size - 1] = wd;
		wd = &cluster(context, wd->parent);
		path_size++;
		tree_path = realloc(tree_path, sizeof(cluster_t) * (path_size+1));
		if (path_size > 1000) break;
	}
	tree_path[path_size-1] = wd;		// root

	char* wd_full_name = (char*)malloc(MAX_NAME_LENGTH * path_size * sizeof(char));
	memset(wd_full_name, '\0', MAX_NAME_LENGTH * path_size * sizeof(char));
	for (int i = path_size-1; i >=0 ; i--) {
		strcat(wd_full_name, tree_path[i]->name);
		if (i < path_size - 1 && i != 0) strcat(wd_full_name, "/");
	}
	free(tree_path);
	return wd_full_name;
}

int zox_fs_cd(fs_context_t context, const char* directory_path) {
	if (strcmp(directory_path, "..") == 0) {
		long int parent_offset = context->_working_directory->parent;
		if (parent_offset != -1) {
			context->_working_directory = &cluster(context, parent_offset);
		}
		return CALL_SUCESS;
	}

	if (!validate_file_name(directory_path)) {
		return E_INVALID_FILE_NAME;
	}
	cluster_t cd_dir = cluster_by_name(context, directory_path);
	if (cd_dir == NULL || cd_dir->type != CLUSTER_TYPE_DIRECTORY) return E_NOT_EXISTS;

	context->_working_directory = cd_dir;
	return CALL_SUCESS;
}

static int _zox_fs_rm(fs_context_t context, cluster_t clust_to_delete) {
	if (clust_to_delete->type == CLUSTER_TYPE_EMPTY) return CALL_SUCESS;
	if (clust_to_delete->type == CLUSTER_TYPE_DIRECTORY) {
		while (clust_to_delete->directory.subnodes_count != 0) {
			_zox_fs_rm(context, &cluster(context, clust_to_delete->directory.subnodes[0]));
		}
	}
	if (clust_to_delete->type == CLUSTER_TYPE_FILE) {
		long int next_part = clust_to_delete->file.next_cluster;
		if (next_part != -1) {
			clust_to_delete->type = CLUSTER_TYPE_EMPTY;
			flush_cluster(context, clust_to_delete);
		}
		while(next_part != -1) {
			cluster_t tmp = &cluster(context, next_part);
			next_part = tmp->file.next_cluster;
			tmp->type = CLUSTER_TYPE_EMPTY;
			flush_cluster(context, tmp);
		}
	}

	clust_to_delete->type = CLUSTER_TYPE_EMPTY;
	cluster_t parent = &cluster(context, clust_to_delete->parent);
	remove_direcotory_subnode(context, clust_to_delete);
	flush_cluster(context, clust_to_delete);
	flush_cluster(context, parent);
	return 0;
}

int zox_fs_rm_s(fs_context_t context, char* file_to_delete) {
	if (strcmp(file_to_delete, "/") == 0) return E_INVALID_FILE_NAME;
	char* wd = zox_fs_pwd_s(context);
	to_absolute_path(context, file_to_delete);
	if (file_to_delete == NULL
		|| (strstr(wd, file_to_delete) == wd)) {
		return E_INVALID_FILE_NAME;
	}
	free(wd);

	cluster_t clust_to_delete = cluster_by_name(context, file_to_delete);
	if (clust_to_delete == NULL) return E_NOT_EXISTS;
	_zox_fs_rm(context, clust_to_delete);
	return CALL_SUCESS;
}

int zox_fs_mkdir(fs_context_t context, const char *directory_path) {
	cluster_t new_dir;
	int creating_result = create_cluster(context, directory_path, &new_dir);
	if (creating_result != CALL_SUCESS) return creating_result;
	new_dir->type = CLUSTER_TYPE_DIRECTORY;
	new_dir->directory.subnodes_count = 0;

	int error_code = flush_cluster(context, new_dir);
	if(error_code != CALL_SUCESS) return error_code;

	return CALL_SUCESS;
}

int zox_fs_load_file(fs_context_t context, const char* source, char* destination) {
	// ----- open file ------
	FILE* source_stream = fopen(source, "rb");
	if (source_stream == NULL) {
#ifdef DEBUG_MODE
		printf("cannot open source file %s: %s\n", source, strerror(errno));
#endif
		return E_OPEN_FILE;
	}

	cluster_t clust_to_write = cluster_by_name(context, destination);

	// ----- check enough memory -----
	size_t free_space = zox_fs_free(context);
	if (clust_to_write != NULL) {
		free_space += zox_fs_fsize(context, clust_to_write);
	}
	fseek(source_stream, 0, SEEK_END);
	if (ftell(source_stream) > free_space) {
		fclose(source_stream);
		return E_FS_FULL;
	}
	// ----- create/truncate destination file -----
	if (clust_to_write != NULL) zox_fs_rm_s(context, destination);
	int create_file_result = create_cluster(context, destination, &clust_to_write);
	if (create_file_result != CALL_SUCESS) return create_file_result;
	clust_to_write->type = CLUSTER_TYPE_FILE;

	// ----- read first block -----
	fseek(source_stream, 0, SEEK_SET);
	size_t bytes_read = fread(clust_to_write->file.data, 1, FILE_CLUSTER_DATA_SIZE, source_stream);
	clust_to_write->file.bytes_used = bytes_read;
	clust_to_write->file.next_cluster = -1;
	if (bytes_read < FILE_CLUSTER_DATA_SIZE) flush_cluster(context, clust_to_write);

	// ----- read other blocks -----
	while (bytes_read >= FILE_CLUSTER_DATA_SIZE) {
		cluster_t next_part = get_free_cluster(context);
		next_part->type = CLUSTER_TYPE_FILE;
		next_part->file.next_cluster = -1;
		clust_to_write->file.next_cluster = next_part->offset;

		bytes_read = fread(next_part->file.data, 1, FILE_CLUSTER_DATA_SIZE, source_stream);
		next_part->file.bytes_used = bytes_read;

		flush_cluster(context, clust_to_write);
		flush_cluster(context, next_part);
		clust_to_write = next_part;
	}
	fclose(source_stream);

	return CALL_SUCESS;
}

int zox_fs_extract_file(fs_context_t context, const char* source, const char* destination) {
	// ----- open file ------
	FILE* destination_stream = fopen(destination, "wb");
	if (destination_stream == NULL) {
#ifdef DEBUG_MODE
		printf("cannot open destination file %s: %s", destination, strerror(errno));
#endif
		return E_OPEN_FILE;
	}

	cluster_t clust_to_read = cluster_by_name(context, source);
	if (clust_to_read == NULL) {
		return E_NOT_EXISTS;
	}
	// ----- write out first block -----
	fseek(destination_stream, 0, SEEK_SET);
	printf("bytes written: %ld\n", fwrite(clust_to_read->file.data, 1, clust_to_read->file.bytes_used, destination_stream));

	// ----- write out other blocks -----
	while (clust_to_read->file.next_cluster != -1) {
		clust_to_read = &cluster(context, clust_to_read->file.next_cluster);
		printf("bytes written: %ld\n", fwrite(clust_to_read->file.data, 1, clust_to_read->file.bytes_used, destination_stream));
	}
	fclose(destination_stream);
	return CALL_SUCESS;
}

int zox_fs_mv(fs_context_t context, char* from, char* to) {

	cluster_t cluster = cluster_by_name(context, from);
	if (cluster == NULL) return E_SOURCE_NOT_EXISTS;

	cluster_t old_parent = &cluster(context, cluster->parent);

	cluster_t destination = cluster_by_name(context, to);
	if (destination != NULL) return E_DUPLICATE;

	char directory_name[MAX_NAME_LENGTH];
	char subnode_name[MAX_NAME_LENGTH];
	to_absolute_path(context, to);
	split_absolute_path(to, directory_name, subnode_name);

	cluster_t new_parent = cluster_by_name(context, directory_name);
	//printf("%s vs %s (%s)", subnode_name, strstr(to, from), to);
	to_absolute_path(context, from);
	if (new_parent == NULL
		|| (strstr(to, from) == to)) {
		return E_DEST_NOT_EXISTS;
	}
	remove_direcotory_subnode(context, cluster);
	add_direcotory_subnode(new_parent, cluster);
	strcpy(cluster->name, subnode_name);
	flush_cluster(context, cluster);
	flush_cluster(context, new_parent);
	flush_cluster(context, old_parent);
	return CALL_SUCESS;
}

int zox_fs_cp(fs_context_t context, const char* from, char* to) {
	cluster_t file_to_copy = cluster_by_name(context, from);
	if (file_to_copy == NULL) return E_SOURCE_NOT_EXISTS;
	if (zox_fs_fsize(context, file_to_copy) > zox_fs_free(context)) return E_FS_FULL;

	char directory_name[MAX_NAME_LENGTH];
	char subnode_name[MAX_NAME_LENGTH];
	to_absolute_path(context, to);
	split_absolute_path(to, directory_name, subnode_name);

	cluster_t copy_parent = cluster_by_name(context, directory_name);
	if (copy_parent == NULL) return E_DEST_NOT_EXISTS;

	cluster_t destination = cluster_by_name(context, to);
	if (destination != NULL) return E_DUPLICATE;

	cluster_t file_copy = get_free_cluster(context);
	file_copy->type = CLUSTER_TYPE_FILE;
	file_copy->file.bytes_used = file_to_copy->file.bytes_used;
	file_copy->file.next_cluster = -1;
	for (int i = 0; i < file_to_copy->file.bytes_used; i++) {
		file_copy->file.data[i] = file_to_copy->file.data[i];
	}
	strcpy(file_copy->name, subnode_name);

	add_direcotory_subnode(copy_parent, file_copy);

	flush_cluster(context, copy_parent);
	flush_cluster(context, file_copy);

	while (file_to_copy->file.next_cluster != -1) {
		file_to_copy = &cluster(context, file_to_copy->file.next_cluster);

		cluster_t file_part = get_free_cluster(context);
		file_copy->file.next_cluster = file_part->offset;
		file_part->file.next_cluster = -1;
		file_part->type = CLUSTER_TYPE_FILE;
		file_part->file.bytes_used = file_to_copy->file.bytes_used;
		for (int i = 0; i < file_to_copy->file.bytes_used; i++) {
			file_part->file.data[i] = file_to_copy->file.data[i];
		}

		flush_cluster(context, file_copy);
		flush_cluster(context, file_part);
		file_copy = file_part;
	}
	return CALL_SUCESS;
}

size_t zox_fs_fsize(fs_context_t context, cluster_t file) {
	long int clusters_used = 0;
	while (file->file.next_cluster != -1) {
		file = &cluster(context, file->file.next_cluster);
		clusters_used++;
	}
	return clusters_used * FILE_CLUSTER_DATA_SIZE + file->file.bytes_used;
}

fs_context_t initialize_context(const char* fname) {
	fs_context_t context = (fs_context_t)malloc(sizeof(struct fs_context_t));
	if (context == NULL) {
#ifdef DEBUG_MODE
		perror("fs_context_t malloc");
#endif
		return NULL;
	}
	memset(context, '\0', sizeof(fs_context_t));

	context->fs_file = fopen(fname, "rb+");
	if (context->fs_file == NULL) {
#ifdef DEBUG_MODE
		printf("creating new fs file\n");
#endif
		for (int i = 0; i < FS_FILE_CLUSTERS_COUNT; i++) {
			cluster_t cluster = &cluster(context, i);
			cluster->offset = i;
			cluster->type = CLUSTER_TYPE_EMPTY;
		}

		fs_root(context).parent = -1;
		fs_root(context).type = CLUSTER_TYPE_DIRECTORY;
		fs_root(context).directory.subnodes_count = 0;
		strcpy(fs_root(context).name, "/");

		context->fs_file = fopen(fname, "wb");
		if (context->fs_file == NULL) {
#ifdef DEBUG_MODE
			printf("Cannot create fs context->fs_file \'%s\': %s\n", fname, strerror(errno));
#endif
			free(context);
			return NULL;
		}
#ifdef DEBUG_MODE
		printf("cluster size: %ld\n", sizeof(struct cluster_t));
		printf("cluster file size: %ld\n", sizeof(fs_root(context).file));
		printf("cluster dir size: %ld\n", sizeof(fs_root(context).directory));
		printf("cluster empty size: %ld\n", sizeof(fs_root(context).empty));
		printf("cluster filler size: %ld\n", sizeof(fs_root(context).filler));
		printf("write size (expected): %ld\n", sizeof(struct cluster_t)*FS_FILE_CLUSTERS_COUNT);
		printf("write size (real): %ld\n", sizeof(context->_fs));
#endif
		if (fwrite(context->_fs, 1, FS_FILE_CLUSTERS_COUNT * sizeof(struct cluster_t),
				   context->fs_file) != FS_FILE_CLUSTERS_COUNT * sizeof(struct cluster_t)) {
#ifdef DEBUG_MODE
			printf("cannot write fs file \'%s\': %s", fname, strerror(errno));
#endif
			free(context);
			return NULL;
		}
	} else {
		if (fread(context->_fs, 1, FS_FILE_CLUSTERS_COUNT * sizeof(struct cluster_t),
				  context->fs_file) != FS_FILE_CLUSTERS_COUNT * sizeof(struct cluster_t)) {
#ifdef DEBUG_MODE
			printf("cannot read fs file \'%s\': %s", fname, strerror(errno));
#endif
			free(context);
			return NULL;
		}
	}
	context->_working_directory = &fs_root(context);
	return context;
}

void fs_context_destroy(fs_context_t context) {
	fclose(context->fs_file);
	return;
}

size_t zox_fs_free(fs_context_t context) {
	long int empty_clusters_count = 0;
	for (int i = 0; i < FS_FILE_CLUSTERS_COUNT; i++) {
		if (context->_fs[i].type == CLUSTER_TYPE_EMPTY) empty_clusters_count++;
	}
	return empty_clusters_count * FILE_CLUSTER_DATA_SIZE;
}
