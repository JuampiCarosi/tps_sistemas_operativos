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
remove_dentry_from_parent_dir(const char *path, inode_t *parent)
{
	char *dir_entry = strrchr(path, '/');
	dir_entry++;

	char *new_content = calloc(MAX_CONTENT, sizeof(char));
	int new_size = 0;
	off_t offset = 0;

	while (offset < parent->size) {
		char buff[MAX_CONTENT];
		get_next_entry(parent->content, &offset, buff);

		if (strcmp(buff, dir_entry) != 0) {
			int buff_entry_size = strlen(buff);
			buff[buff_entry_size] = '\n';
			buff[++buff_entry_size] = '\0';
			strcpy(new_content + new_size, buff);
			new_size += strlen(buff);
		}
	}

	strcpy(parent->content, new_content);
	parent->size = new_size;
	free(new_content);
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

	inode_t *inode = &superblock.inodes[free_index];
	strcpy(inode->path, path);
	memset(inode->content, 0, MAX_CONTENT);
	inode->type = type;
	inode->size = 0;
	inode->last_access = time(NULL);
	inode->last_modification = time(NULL);
	inode->creation_time = time(NULL);
	inode->group = getgid();
	inode->owner = getuid();
	inode->permissions = mode;

	superblock.inode_bitmap[free_index] = 1;

	return free_index;
}
