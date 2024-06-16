#include "file.h"
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>

void *
recalloc(void *ptr, size_t old_size, size_t new_count, size_t new_size)
{
	void *new_ptr = calloc(new_count, new_size);
	if (new_ptr == NULL) {
		return NULL;
	}
	memcpy(new_ptr, ptr, old_size);
	free(ptr);
	return new_ptr;
}

void
deserialize(int fp)
{
	int i = 0;
	bool error = false;
	int inodes_amount = 0;
	read(fp, &inodes_amount, sizeof(int));

	superblock.inodes = calloc(inodes_amount, sizeof(inode_t));
	superblock.inode_bitmap = calloc(inodes_amount, sizeof(int));
	superblock.inode_amount = inodes_amount;


	while (i < inodes_amount) {
		inode_t *inode = &superblock.inodes[i];

		int read_inode = read(fp, inode, sizeof(inode_t));
		if (read_inode <= 0) {
			error = read_inode != 0;
			break;
		}


		inode->content = calloc(inode->size, sizeof(char));
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
		for (int j = 0; j < i; j++) {
			free(superblock.inodes[j].content);
		}
		free(superblock.inodes);
		free(superblock.inode_bitmap);
		superblock.inodes = NULL;
		superblock.inode_bitmap = NULL;
		superblock.inode_amount = 0;
	}
}

void
serialize(int fp)
{
	int inodes_amount = 0;
	for (int i = 0; i < superblock.inode_amount; i++) {
		if (superblock.inode_bitmap[i] == 1) {
			inodes_amount++;
		}
	}
	write(fp, &inodes_amount, sizeof(int));
	for (int i = 0; i < superblock.inode_amount; i++) {
		inode_t *inode = &superblock.inodes[i];
		if (superblock.inode_bitmap[i] != 1)
			continue;

		int write_inode = write(fp, inode, sizeof(inode_t));
		int write_content =
		        write(fp, inode->content, sizeof(inode->size));
		free(superblock.inodes[i].content);


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
}

int
search_inode(const char *path)
{
	int i = 0;
	while (i < superblock.inode_amount &&
	       (superblock.inode_bitmap[i] == 0 ||
	        strcmp(superblock.inodes[i].path, path) != 0)) {
		i++;
	}


	if (i == superblock.inode_amount) {
		return ERROR;
	}

	return i;
}

int
search_next_free_inode()
{
	int i = 0;
	while (i < superblock.inode_amount && superblock.inode_bitmap[i] != 0) {
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
add_dentry_to_content(char **content, int *content_size, char *dentry)
{
	int dentry_size = strlen(dentry);
	dentry[dentry_size] = '\n';
	dentry[++dentry_size] = '\0';

	int content_length = strlen(*content);

	if (content_length + dentry_size > *content_size) {
		*content = realloc(*content,
		                   content_length + dentry_size +
		                           INITIAL_CONTENT_LENGTH);
		if (content == NULL) {
			errno = ENOMEM;
			return;
		}
		*content_size += dentry_size + INITIAL_CONTENT_LENGTH;
	}

	strcpy(*content + content_length, dentry);
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

	char *new_content = calloc(parent->size, sizeof(char));
	int new_size = parent->size;

	off_t offset = 0;
	int content_length = strlen(parent->content);
	while (offset < content_length) {
		char buff[MAX_PATH + 1];
		get_next_entry(parent->content, &offset, buff);

		if (strcmp(buff, dir_entry) != 0) {
			add_dentry_to_content(&new_content, &new_size, buff);
		}
	}

	free(parent->content);
	free(superblock.inodes[inode_index].content);
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

	add_dentry_to_content(&parent_inode->content,
	                      &parent_inode->size,
	                      dir_entry);

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

	if (free_index == superblock.inode_amount) {
		void *new_inodes =
		        recalloc(superblock.inodes,
		                 superblock.inode_amount * sizeof(inode_t),
		                 superblock.inode_amount + INITIAL_INODES_AMOUNT,
		                 sizeof(inode_t));

		if (new_inodes == NULL) {
			return -ENOMEM;
		}

		superblock.inodes = new_inodes;

		void *new_inode_bitmap =
		        recalloc(superblock.inode_bitmap,
		                 superblock.inode_amount * sizeof(int),
		                 superblock.inode_amount + INITIAL_INODES_AMOUNT,
		                 sizeof(int));
		if (new_inode_bitmap == NULL) {
			return -ENOMEM;
		}

		superblock.inode_bitmap = new_inode_bitmap;
		superblock.inode_amount += INITIAL_INODES_AMOUNT;
	}

	inode_t *inode = &superblock.inodes[free_index];
	strcpy(inode->path, path);
	inode->content = calloc(INITIAL_CONTENT_LENGTH, sizeof(char));
	if (inode->content == NULL) {
		return -ENOMEM;
	}
	inode->content[0] = '\0';
	inode->type = type;
	inode->size = INITIAL_CONTENT_LENGTH;
	inode->last_access = time(NULL);
	inode->last_modification = time(NULL);
	inode->creation_time = time(NULL);
	inode->group = getgid();
	inode->owner = getuid();
	inode->permissions = mode;

	superblock.inode_bitmap[free_index] = 1;
	return free_index;
}
