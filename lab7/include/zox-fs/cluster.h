#ifndef CLUSTER_H
#define CLUSTER_H

#include <stdbool.h>
#include <sys/types.h>

#define CLUSTER_SIZE 4096
#define CLUSTER_OVERHEAD (sizeof(int) \
							+ sizeof(long int) \
							+ sizeof(long int) \
							+ MAX_NAME_LENGTH)

#define FILE_CLUSTER_DATA_SIZE (CLUSTER_SIZE \
								- sizeof(size_t) \
								- sizeof(long int) \
								- CLUSTER_OVERHEAD)

#define MAX_NAME_LENGTH 80

#define CLUSTER_TYPE_DIRECTORY 	1
#define CLUSTER_TYPE_FILE 		2
#define CLUSTER_TYPE_EMPTY		3

#define cluster(context, num) context->_fs[num]

struct cluster_file_t {
	size_t bytes_used;
	long int next_cluster;		// -1 for last
	char data[FILE_CLUSTER_DATA_SIZE];
} __attribute__((packed));
typedef struct cluster_file_t cluster_file_t;

struct cluster_directory_t {
	long int subnodes_count;
	long int subnodes[(CLUSTER_SIZE
					 	- sizeof(long int)
				   		- CLUSTER_OVERHEAD)
					  	/ sizeof(long int)];
} __attribute__((packed));
typedef struct cluster_directory_t cluster_directory_t;

typedef struct cluster_empty_t {

} cluster_empty_t;

struct cluster_t {
	int type;
	long int parent;		// -1 for root
	long int offset;
	char name[MAX_NAME_LENGTH];
	union {
		cluster_file_t file;
		cluster_directory_t directory;
		cluster_empty_t empty;
		char filler[CLUSTER_SIZE - CLUSTER_OVERHEAD];
	};
} __attribute__((packed));
typedef struct cluster_t* cluster_t;

struct fs_context_t;

cluster_t get_free_cluster(struct fs_context_t*);
cluster_t get_subnode(struct fs_context_t *, cluster_t, const char *);
bool validate_file_name(const char*);
int flush_cluster(struct fs_context_t*, cluster_t);
cluster_t cluster_by_name(struct fs_context_t*, const char*);
int create_cluster(struct fs_context_t*, const char*, cluster_t* retval);
void remove_direcotory_subnode(struct fs_context_t*, cluster_t subnode);
void add_direcotory_subnode(cluster_t directory, cluster_t subnode);
void split_absolute_path(const char *full_name, char *directory, char *subnode_name);
void to_absolute_path(struct fs_context_t*, char* path);

#endif //CLUSTER_H
