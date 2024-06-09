#include "file.h"
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <stdlib.h>

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
get_parent(const char path[MAX_PATH]) {
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
initialize_root_dir()
{
	inode_t *root = &superblock.inodes[0];
	strcpy(root->path, "/");
	memset(root->content, 0, MAX_CONTENT);
	root->type = INODE_DIR;
	root->size = 0;
	root->last_access = time(NULL);
	root->last_modification = time(NULL);
	root->creation_time = time(NULL);
	root->group = getgid();
	root->owner = getuid();
	root->permissions = __S_IFDIR | 0755;
}

void
format_fs()
{
	superblock.inode_bitmap[0] = 1;
	initialize_root_dir();
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