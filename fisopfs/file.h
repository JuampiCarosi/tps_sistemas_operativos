#ifndef _FILE_H_
#define _FILE_H_
#include <stdio.h>
#define MAX_CONTENT 1024
#define MAX_INODES 100

enum inode_type { INODE_FILE, INODE_DIR };

typedef struct inode {
	char *path;
	char content[MAX_CONTENT];
	int size;
	enum inode_type type;
} inode_t;

extern inode_t inodes[MAX_INODES];

void deserialize(FILE *fp);
void serialize(FILE *fp);

#endif  // _FILE_H_
