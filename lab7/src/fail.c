#include <stdio.h>
#include "cluster.h"
#include "zox-fs.h"
#include "repl.h"

union {
	struct cluster_file_t file;
} data1;

union data2{
	char filler[CLUSTER_SIZE - sizeof(int)];
} data2;

union data3 {
	struct cluster_file_t file;
	char filler[CLUSTER_SIZE - sizeof(int)];
};


typedef union data3 data3;


int main(int argc, char* argv[]) {
//	struct cluster_t cluster;
//	printf("cluster size: %ld\n", sizeof(struct cluster_t));
//	printf("cluster type size: %ld\n", sizeof(cluster.type));
//	printf("cluster data size: %ld\n", sizeof(cluster.data));
	printf("data1 size: %ld\n", sizeof(data1));
	printf("data2 size: %ld\n", sizeof(data2));
	printf("data3 size: %ld\n", sizeof(data3));
	return 0;
}