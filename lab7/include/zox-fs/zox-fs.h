#ifndef ZOX_FS_H
#define ZOX_FS_H

#include "cluster.h"
#include <stdio.h>

#define FS_FILE_CLUSTERS_COUNT 20//2048
#define fs_root(fs) fs->_fs[0]

struct fs_context_t {
	FILE* fs_file;
	cluster_t _working_directory;
	struct cluster_t _fs[FS_FILE_CLUSTERS_COUNT];
};

typedef struct fs_context_t* fs_context_t;

fs_context_t initialize_context(const char*);
void fs_context_destroy(fs_context_t);

const cluster_t zox_fs_pwd(fs_context_t);
char* zox_fs_pwd_s(fs_context_t);
int zox_fs_ls(fs_context_t, cluster_t*);
int zox_fs_cd(fs_context_t, const char *);

int zox_fs_mkdir(fs_context_t, const char *);
int zox_fs_rm_s(fs_context_t, char *);

int zox_fs_load_file(fs_context_t, const char* source, char* destination);
int zox_fs_extract_file(fs_context_t, const char *source, const char *destination);

size_t zox_fs_fsize(fs_context_t, cluster_t);
size_t zox_fs_free(fs_context_t);

int zox_fs_mv(fs_context_t, char* from, char* to);
int zox_fs_cp(fs_context_t, const char* from, char* to);

#endif //ZOX_FS_H
