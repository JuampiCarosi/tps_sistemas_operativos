#ifndef _FILE_H_
#define _FILE_H_
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#define MAX_CONTENT 1024
#define MAX_INODES 100
#define MAX_PATH 256

enum inode_type { INODE_FILE, INODE_DIR };

typedef struct inode {
	char path[MAX_PATH];
	char content[MAX_CONTENT];
	int size;
	enum inode_type type;
	time_t last_access;
	time_t last_modification;
	time_t creation_time;
	gid_t group;
	uid_t owner;
	mode_t permissions;

} inode_t;

typedef struct superblock {
	inode_t inodes[MAX_INODES];
	int inode_bitmap[MAX_INODES];
} superblock_t;

extern superblock_t superblock;

void deserialize(int fp);
void serialize(int fp);
void format_fs();

#endif  // _FILE_H_
