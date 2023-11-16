/*
 *  Portions of this file are taken from linux kernel 2.4.25
 *  linux/fs/block_dev.c
 *
 *  Copyright (C) 1991, 1992  Linus Torvalds
 *  Copyright (C) 2001  Andrea Arcangeli <andrea@suse.de> SuSE
 *
 *	Copyright (C) 2004  Dan Mihailescu
 */

#include "todo.h"
#include <linux/slab.h>
#include <linux/blkdev.h>



int set_blocksize(kdev_t dev, int size)
{
	/* Size must be a power of two */
	if (size < 512 || (size & (size-1)))
		return -EINVAL;
	/* Size cannot be smaller than the size supported by the device */
	if (size < get_hardsect_size(dev))
		return -EINVAL;
	// nothing more to check
	// i think i won't need the block size set in the block device
	// since i have it in the super block, let's hope it's enough
	// i don't understand why this is needed, the block size is passed 
	// as a parameter to bread; maybe for read ahead ...
	return 0;
}

const char * bdevname(kdev_t dev);

static __inline void __bd_forget(struct inode *inode)
{
	list_del_init(&inode->i_devices);
	inode->i_bdev = NULL;
	inode->i_mapping = &inode->i_data;
}


/* Call when you free inode */
void bd_forget(struct inode *inode)
{
	if (inode->i_bdev)
		__bd_forget(inode);
}

