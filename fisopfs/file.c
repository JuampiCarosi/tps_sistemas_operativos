#include "file.h"
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <errno.h>

void
deserialize(int fp)
{
	int res = read(fp, &superblock, sizeof(superblock_t));

	if (res < 0) {
		perror("Error loading filesystem\n");
	}
}

void
serialize(int fp)
{
	int res = write(fp, &superblock, sizeof(superblock_t));

	if (res < 0) {
		perror("Error saving filesystem\n");
	}
}

char *
get_parent(const char path[MAX_PATH])
{
	char *parent = malloc(sizeof(char) * (strlen(path) + 1));
	strcpy(parent, path);
	char *last_slash = strrchr(parent, '/');
	if (last_slash == parent) {
		last_slash++;
	}
	*last_slash = '\0';
	return parent;
}

void
format_fs()
{
	superblock.inode_bitmap[0] = 1;
	create_inode("/", __S_IFDIR | 0755, INODE_DIR);
}

int
search_inode(const char *path)
{
	int i = 0;
	while (i < MAX_INODES && strcmp(superblock.inodes[i].path, path) != 0) {
		i++;
	}

	if (i == MAX_INODES) {
		return ERROR;
	}

	return i;
}

int
search_next_free_inode()
{
	int i = 0;
	while (i < MAX_INODES && superblock.inode_bitmap[i] != 0) {
		i++;
	}
	return i;
}


int
add_dentry_to_parent_dir(const char *path)
{
	char *parent_path = get_parent(path);
	int dir_index = search_inode(parent_path);
	if (dir_index == ERROR) {
		errno = ENOENT;
		return -ENOENT;
	}

	inode_t *parent_inode = &superblock.inodes[dir_index];

	if (parent_inode->type != INODE_DIR) {
		errno = ENOTDIR;
		return -ENOTDIR;
	}
	char *dir_entry = strrchr(path, '/');
	dir_entry++;
	int entry_size = strlen(dir_entry);
	dir_entry[entry_size] = '\n';
	dir_entry[++entry_size] = '\0';

	strcpy(superblock.inodes[dir_index].content +
	               superblock.inodes[dir_index].size,
	       dir_entry);
	superblock.inodes[dir_index].size += entry_size;

	free(parent_path);
	return 0;
}

int
create_inode(const char *path, mode_t mode, enum inode_type type)
{
	if (strlen(path) > MAX_PATH) {
		return -ENAMETOOLONG;
	}

	int free_index = search_next_free_inode();

	if (free_index == MAX_INODES) {
		return -ENOSPC;
	}

	inode_t *directory = &superblock.inodes[free_index];
	strcpy(directory->path, path);
	memset(directory->content, 0, MAX_CONTENT);
	directory->type = type;
	directory->size = 0;
	directory->last_access = time(NULL);
	directory->last_modification = time(NULL);
	directory->creation_time = time(NULL);
	directory->group = getgid();
	directory->owner = getuid();
	directory->permissions = mode;

	superblock.inode_bitmap[free_index] = 1;

	return free_index;
}
