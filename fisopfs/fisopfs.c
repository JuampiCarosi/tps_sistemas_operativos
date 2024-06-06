#define FUSE_USE_VERSION 30

#include <fuse.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include "file.h"
#define FS_PATH "fs.fisopfs"

inode_t inodes[MAX_INODES];

static int
fisopfs_getattr(const char *path, struct stat *st)
{
	printf("[debug] fisopfs_getattr - path: %s\n", path);

	if (strcmp(path, "/") == 0) {
		printf("path root inode = %s\n", inodes[0].path);
		printf("content root inode = %s\n", inodes[0].content);
		printf("size root inode = %d\n", inodes[0].size);
		printf("type root inode = %d\n", inodes[0].type);
		st->st_uid = 1717;
		st->st_mode = __S_IFDIR | 0755;
		st->st_nlink = 2;
	} else if (strcmp(path, "/fisop") == 0) {
		printf("content fisop inode = %s\n", inodes[1].content);
		printf("size fisop inode = %d\n", inodes[1].size);
		printf("type fisop inode = %d\n", inodes[1].type);
		printf("path fisop inode = %s\n", inodes[1].path);
		st->st_uid = 1818;
		st->st_mode = __S_IFREG | 0644;
		st->st_size = 2048;
		st->st_nlink = 1;
	} else {
		return -ENOENT;
	}

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

	// Si nos preguntan por el directorio raiz, solo tenemos un archivo
	if (strcmp(path, "/") == 0) {
		filler(buffer, "fisop", NULL, 0);
		return 0;
	}

	return -ENOENT;
}

#define MAX_CONTENIDO 100
static char fisop_file_contenidos[MAX_CONTENIDO] = "hola fisopfs!\n";

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

	// Solo tenemos un archivo hardcodeado!
	if (strcmp(path, "/fisop") != 0)
		return -ENOENT;

	if (offset + size > strlen(fisop_file_contenidos))
		size = strlen(fisop_file_contenidos) - offset;

	size = size > 0 ? size : 0;

	memcpy(buffer, fisop_file_contenidos + offset, size);

	return size;
}

static void *
fisopfs_init(struct fuse_conn_info *conn)
{
	printf("[debug] fisopfs_init\n");

	FILE *fp = fopen(FS_PATH, "r");

	if (!fp) {
		inodes[0].path = "/";
		memset(inodes[0].content, 0, MAX_CONTENT);
		inodes[0].size = 0;
		inodes[0].type = INODE_DIR;

		inodes[1].path = "/fisop";
		strncpy(inodes[1].content, fisop_file_contenidos, MAX_CONTENT);
		inodes[1].size = strlen(fisop_file_contenidos);
		inodes[1].type = INODE_FILE;

	} else {
		deserialize(fp);
		fclose(fp);
	}


	return NULL;
}

static void
fisopfs_destroy(void *userdata)
{
	printf("[debug] fisopfs_destroy\n");

	FILE *fp = fopen(FS_PATH, "w");

	if (!fp) {
		perror("fopen");
		return;
	}

	serialize(fp);
	fclose(fp);
}

static struct fuse_operations operations = {
	.getattr = fisopfs_getattr,
	.readdir = fisopfs_readdir,
	.read = fisopfs_read,
	.init = fisopfs_init,
	//.destroy = fisopfs_destroy,
};

int
main(int argc, char *argv[])
{
	return fuse_main(argc, argv, &operations, NULL);
}
