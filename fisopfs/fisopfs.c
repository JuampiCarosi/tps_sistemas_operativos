#define FUSE_USE_VERSION 30

#include <fuse.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include "file.h"
#define FS_PATH "fs.fisopfs"

superblock_t superblock = {};

char *
strip_newline_character(char *str)
{
	char *pos;
	if ((pos = strchr(str, '\n')) != NULL) {
		*pos = '\0';
	}
	return str;
}

static int
fisopfs_getattr(const char *path, struct stat *st)
{
	printf("[debug] fisopfs_getattr - path: %s\n", path);

	char path_copy[MAX_PATH];
	strcpy(path_copy, path);

	int inode_index = search_inode(strip_newline_character(path_copy));

	if (inode_index == ERROR) {
		errno = ENOENT;
		return -ENOENT;
	}

	inode_t inode = superblock.inodes[inode_index];
	st->st_nlink = 2;
	st->st_uid = inode.owner;
	st->st_gid = inode.group;
	st->st_size = inode.size;
	st->st_atime = inode.last_access;
	st->st_mtime = inode.last_modification;
	st->st_ctime = inode.creation_time;
	st->st_mode = __S_IFDIR | 0755;
	if (inode.type == INODE_FILE) {
		st->st_mode = __S_IFREG | 0644;
		st->st_nlink = 1;
	}

	return 0;
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
		return ERROR;
	}

	buffer[i++] = '\0';
	return i;
}

int
get_next_entry(char *content, off_t *offset, char *buff)
{
	int read = read_line(content, buff, *offset);

	if (read == ERROR) {
		return ERROR;
	}

	*offset += read;
	return 0;
}


static int
fisopfs_readdir(const char *path,
                void *buffer,
                fuse_fill_dir_t filler,
                off_t offset,
                struct fuse_file_info *fi)
{
	printf("[debug] fisopfs_readdir - path: %s\n", path);

	// Los directorios '.' y '..'
	filler(buffer, ".", NULL, 0);
	filler(buffer, "..", NULL, 0);

	int inode_index = search_inode(path);

	if (inode_index == ERROR) {
		errno = ENOENT;
		return -ENOENT;
	}

	if (superblock.inodes[inode_index].type != INODE_DIR) {
		errno = ENOTDIR;
		return -ENOTDIR;
	}

	superblock.inodes[inode_index].last_access = time(NULL);

	inode_t inode = superblock.inodes[inode_index];

	while (offset < (superblock.inodes[inode_index].size - 1)) {
		char buff[MAX_CONTENT];
		int result = get_next_entry(inode.content, &offset, buff);

		if (result == ERROR) {
			errno = ENOENT;
			return -ENOENT;
		}

		filler(buffer, buff, NULL, 0);
	}

	return 0;
}

static int
fisopfs_read(const char *path,
             char *buffer,
             size_t size,
             off_t offset,
             struct fuse_file_info *fi)
{
	printf("[debug] fisopfs_read - path: %s, offset: %lu, size: %lu\n",
	       path,
	       offset,
	       size);

	int inode_index = search_inode(path);

	if (inode_index == ERROR) {
		errno = ENOENT;
		return -ENOENT;
	}

	if (superblock.inodes[inode_index].type != INODE_FILE) {
		errno = EISDIR;
		return -EISDIR;
	}

	inode_t *file = &superblock.inodes[inode_index];

	if (offset + size > file->size)
		size = file->size - offset;

	size = size > 0 ? size : 0;

	memcpy(buffer, file->content + offset, size);
	file->last_access = time(NULL);

	return size;
}

void
remove_dentry_from_parent_dir(const char *path, inode_t *parent)
{
	char *dir_entry = strrchr(path, '/');
	dir_entry++;

	char *new_content = malloc(MAX_CONTENT);
	int new_size = 0;
	off_t offset = 0;

	while (offset < parent->size) {
		char buff[MAX_CONTENT];
		int result = get_next_entry(parent->content, &offset, buff);

		if (result == ERROR) {
			break;
		}

		if (strcmp(buff, dir_entry) != 0) {
			strcpy(new_content + new_size, buff);
			new_size += strlen(buff);
		}
	}

	strcpy(parent->content, new_content);
	parent->size = new_size;
	free(new_content);
}

static int
fisopfs_unlink(const char *path)
{
	printf("[debug] fisopfs_unlink - path: %s\n", path);

	int inode_index = search_inode(path);

	if (inode_index == ERROR) {
		errno = ENOENT;
		return -ENOENT;
	}

	inode_t inode = superblock.inodes[inode_index];

	if (inode.type != INODE_FILE) {
		errno = EISDIR;
		return -EISDIR;
	}

	superblock.inode_bitmap[inode_index] = 0;

	// we have to change dentries logic so as to remove
	// the entry from the parent directory in an easy way
	char *parent_path = get_parent(path);
	int parent_index = search_inode(parent_path);
	inode_t *parent = &superblock.inodes[parent_index];

	remove_dentry_from_parent_dir(path, parent);
	free(parent_path);

	return 0;
}

static int
fisopfs_mkdir(const char *path, mode_t mode)
{
	printf("[debug] fisopfs_mkdir - path: %s\n", path);

	int inode_index = create_inode(path, mode, INODE_DIR);

	if (inode_index < 0) {
		errno = inode_index;
		return inode_index;
	}

	int result = add_dentry_to_parent_dir(path);

	if (result < 0) {
		errno = result;
		return result;
	}

	return 0;
}

static int
fisopfs_create(const char *path, mode_t mode, struct fuse_file_info *fi)
{
	printf("[debug] fisopfs_create - path: %s\n", path);

	int inode_index = create_inode(path, mode, INODE_FILE);

	if (inode_index < 0) {
		errno = inode_index;
		return inode_index;
	}

	int result = add_dentry_to_parent_dir(path);

	if (result < 0) {
		errno = result;
		return result;
	}

	return 0;
}

static int
fisopfs_utimens(const char *path, const struct timespec ts[2])
{
	printf("[debug] fisop_utimens - path: %s\n", path);

	int inode_index = search_inode(path);

	if (inode_index == ERROR) {
		errno = ENOENT;
		return -ENOENT;
	}

	superblock.inodes[inode_index].last_access = ts[0].tv_sec;
	superblock.inodes[inode_index].last_modification = ts[1].tv_sec;
	return 0;
}

static void *
fisopfs_init(struct fuse_conn_info *conn)
{
	printf("[debug] fisopfs_init\n");

	int fp = open(FS_PATH, O_RDONLY);

	if (fp < 0) {
		format_fs();
		fp = open(FS_PATH, O_WRONLY | O_CREAT, 0644);
	} else {
		deserialize(fp);
	}

	close(fp);
	return 0;
}

static void
fisopfs_destroy(void *userdata)
{
	printf("[debug] fisopfs_destroy\n");

	int fp = open(FS_PATH, O_WRONLY | O_TRUNC, 0644);

	if (fp < 0) {
		perror("Error saving filesystem\n");
		return;
	}

	serialize(fp);
	close(fp);
}

static struct fuse_operations operations = {
	.getattr = fisopfs_getattr,
	.readdir = fisopfs_readdir,
	.read = fisopfs_read,
	.init = fisopfs_init,
	.destroy = fisopfs_destroy,
	.create = fisopfs_create,
	.utimens = fisopfs_utimens,
	.mkdir = fisopfs_mkdir,
	.unlink = fisopfs_unlink,
};

int
main(int argc, char *argv[])
{
	return fuse_main(argc, argv, &operations, NULL);
}
