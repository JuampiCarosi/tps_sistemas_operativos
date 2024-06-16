#ifndef _FILE_H_
#define _FILE_H_
#include <sys/types.h>
#include <stdio.h>
#include <time.h>
#define INITIAL_CONTENT_LENGTH 1024
#define INITIAL_INODES_AMOUNT 1024
#define MAX_PATH 256
#define ERROR -1

enum inode_type { INODE_FILE, INODE_DIR };

typedef struct inode {
	char path[MAX_PATH];
	char *content;
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
	inode_t *inodes;
	int *inode_bitmap;
	int inode_amount;
} superblock_t;

extern superblock_t superblock;

void deserialize(int fp);
void serialize(int fp);
void format_fs();
int search_inode(const char *path);
char *get_parent_path(const char path[MAX_PATH]);
int search_next_free_inode();
int create_inode(const char *path, mode_t mode, enum inode_type type);
int add_dentry_to_parent_dir(const char *path);
void remove_inode(const char *path, int node_index);
void get_next_entry(char *content, off_t *offset, char *buff);
int read_line(const char *content, char *buffer, off_t offset);

#endif  // _FILE_H_
