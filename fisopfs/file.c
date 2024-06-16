#include "file.h"
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>

void
deserialize(int fp)
{
	int i = 0;
	bool error = false;
	int inodes_amount = 0;
	read(fp, &inodes_amount, sizeof(int));
	while (i < inodes_amount) {
		inode_t *inode = &superblock.inodes[i];

		int read_inode = read(fp, inode, sizeof(inode_t));
		if (read_inode <= 0) {
			error = read_inode != 0;
			break;
		}

		inode->content = malloc(sizeof(char) * inode->size);
		if (inode->content == NULL) {
			error = true;
			break;
		}
		superblock.inode_bitmap[i] = 1;
		i++;

		int read_content = read(fp, inode->content, sizeof(inode->size));
		if (read_content < 0) {
			error = true;
			break;
		}
	}

	if (error) {
		perror("Error loading filesystem\n");
		for (int j = 0; j < MAX_INODES; j++) {
			free(superblock.inodes[j].content);
		}
		return;
	}
	superblock.inode_amount = inodes_amount;
}

void
serialize(int fp)
{
	write(fp, &superblock.inode_amount, sizeof(int));
	for (int i = 0; i < MAX_INODES; i++) {
		inode_t *inode = &superblock.inodes[i];
		if (superblock.inode_bitmap[i] != 1)
			continue;

		int write_inode = write(fp, inode, sizeof(inode_t));
		int write_content = 0;
		if (inode->content != NULL) {
			write_content =
			        write(fp, inode->content, sizeof(inode->size));
			free(superblock.inodes[i].content);
		}


		if (write_inode < 0 || write_content < 0) {
			perror("Error saving filesystem\n");

			break;
		}
	}
}

char *
get_parent_path(const char path[MAX_PATH])
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

inode_t *
get_parent(const char path[MAX_PATH], int *error)
{
	char *parent_path = get_parent_path(path);
	int parent_index = search_inode(parent_path);
	if (parent_index == ERROR) {
		*error = -ENOENT;
		free(parent_path);
		return NULL;
	}
	free(parent_path);
	return &superblock.inodes[parent_index];
}

void
format_fs()
{
	create_inode("/", __S_IFDIR | 0755, INODE_DIR);
	printf("superblock amount: %d\n", superblock.inode_amount);
}

int
search_inode(const char *path)
{
	int i = 0;
	while (i < MAX_INODES && (strcmp(superblock.inodes[i].path, path) != 0 ||
	                          superblock.inode_bitmap[i] == 0)) {
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
read_line(const char *content, char *buffer, off_t offset)
{
	int i = 0;

	while (content[offset] != '\n' && content[offset] != '\0') {
		buffer[i] = content[offset];
		i++;
		offset++;
	}
	if (content[offset] == '\0' && i == 0) {
		return 0;
	}

	buffer[i++] = '\0';
	return i;
}

void
get_next_entry(char *content, off_t *offset, char *buff)
{
	int read = read_line(content, buff, *offset);
	*offset += read;
}

void
remove_inode(const char *path, int inode_index)
{
	superblock.inode_bitmap[inode_index] = 0;

	// we have to change dentries logic so as to remove
	// the entry from the parent directory in an easy way
	int error;
	inode_t *parent = get_parent(path, &error);

	char *dir_entry = strrchr(path, '/');
	dir_entry++;

	char *new_content = malloc(sizeof(char) * parent->size);
	strcpy(new_content, parent->content);
	int new_size = 0;
	off_t offset = 0;

	while (offset < parent->size) {
		char buff[parent->size];
		get_next_entry(parent->content, &offset, buff);

		if (strcmp(buff, dir_entry) != 0) {
			int buff_entry_size = strlen(buff);
			buff[buff_entry_size] = '\n';
			buff[++buff_entry_size] = '\0';
			strcpy(new_content + new_size, buff);
			new_size += strlen(buff);
		}
	}

	// free(parent->content);
	parent->content = new_content;
	parent->size = new_size;
}

int
add_dentry_to_parent_dir(const char *path)
{
	char *parent_path = get_parent_path(path);
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

	if (!parent_inode->content) {
		parent_inode->content = malloc(sizeof(char) * entry_size);
		if (parent_inode->content == NULL) {
			errno = ENOMEM;
			return -ENOMEM;
		}
	}

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

	inode_t *inode = &superblock.inodes[free_index];
	strcpy(inode->path, path);
	inode->content = NULL;
	inode->type = type;
	inode->size = 0;
	inode->last_access = time(NULL);
	inode->last_modification = time(NULL);
	inode->creation_time = time(NULL);
	inode->group = getgid();
	inode->owner = getuid();
	inode->permissions = mode;

	superblock.inode_bitmap[free_index] = 1;
	superblock.inode_amount++;
	return free_index;
}
