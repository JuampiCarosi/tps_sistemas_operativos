#include "file.h"
#include <unistd.h>

void
deserialize(FILE *fp)
{
	read(fp, inodes, sizeof(inode_t) * MAX_INODES);
}

void
serialize(FILE *fp)
{
	write(fp, inodes, sizeof(inode_t) * MAX_INODES);
}