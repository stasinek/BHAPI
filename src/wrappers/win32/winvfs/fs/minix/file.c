/*
 *  linux/fs/minix/file.c
 *
 *  Copyright (C) 1991, 1992 Linus Torvalds
 *
 *  minix regular file handling primitives
 */

#include "linux/fs.h"
#include "linux/minix_fs.h"
#include "asm/errno.h"

/*
 * We have mostly NULLs here: the current defaults are OK for
 * the minix filesystem.
 */
int minix_sync_file(struct file *, struct dentry *, int);

struct file_operations minix_file_operations = {
	NULL, generic_file_llseek, generic_file_read, generic_file_write,
	NULL, NULL, NULL, generic_file_mmap, NULL, NULL, NULL, 
	minix_sync_file, NULL, NULL, NULL, NULL, NULL, NULL
};

struct inode_operations minix_file_inode_operations = {
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 
	NULL, NULL, minix_truncate, NULL, NULL, NULL, NULL, 
	NULL, NULL, NULL, NULL
};

int minix_sync_file(struct file * file, struct dentry *dentry, int datasync)
{
	struct inode *inode = dentry->d_inode;
	int err;

	err = fsync_inode_buffers(inode);
	err |= fsync_inode_data_buffers(inode);
	if (!(inode->i_state & I_DIRTY))
		return err;
	if (datasync && !(inode->i_state & I_DIRTY_DATASYNC))
		return err;
	
	err |= minix_sync_inode(inode);
	return err ? -EIO : 0;
}
