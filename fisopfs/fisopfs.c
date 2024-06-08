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

static int
fisopfs_getattr(const char *path, struct stat *st)
{
	printf("[debug] fisopfs_getattr - path: %s\n", path);

	int i = 0;
	while (i < MAX_INODES && strcmp(superblock.inodes[i].path, path) != 0) {
		i++;
	}

	if (i == MAX_INODES) {
		errno = ENOENT;
		return -ENOENT;
	}

	st->st_nlink = 1;
	st->st_uid = superblock.inodes[i].owner;
	st->st_gid = superblock.inodes[i].group;
	st->st_mode = superblock.inodes[i].permissions;
	st->st_size = superblock.inodes[i].size;
	st->st_atime = superblock.inodes[i].last_access;
	st->st_mtime = superblock.inodes[i].last_modification;
	st->st_ctime = superblock.inodes[i].creation_time;

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

	inode_t file = superblock.inodes[inode_index];

	if (offset + size > file.size)
		size = file.size - offset;

	size = size > 0 ? size : 0;

	memcpy(buffer, file.content + offset, size);

	return size;
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
};

int
main(int argc, char *argv[])
{
	return fuse_main(argc, argv, &operations, NULL);
}
