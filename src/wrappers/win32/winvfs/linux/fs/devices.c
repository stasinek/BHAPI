/*
 *  Portions of this file are taken from linux kernel 2.4.25
 *  linux/fs/devices.c
 *
 * (C) 1993 Matthias Urlichs -- collected common code and tables.

 */

#include "todo.h"

#include <linux/config.h>
#include <linux/fs.h>
//#include <linux/major.h>
#include <linux/string.h>
#include <linux/sched.h>
#include <linux/stat.h>
#include <linux/fcntl.h>
#include <linux/errno.h>
//#include <linux/module.h>
#include <linux/smp_lock.h>

const char * kdevname(kdev_t dev)
{
	// should be good for now until i figure out how to get the real device name
	return bdevname(dev);
}

void init_special_inode(struct inode *inode, umode_t mode, int rdev)
{
	inode->i_mode = mode;
	if (S_ISCHR(mode)) {
		;//inode->i_fop = &def_chr_fops;
		;//inode->i_rdev = to_kdev_t(rdev);
		;//inode->i_cdev = cdget(rdev);
	} else if (S_ISBLK(mode)) {
		;//inode->i_fop = &def_blk_fops;
		;//inode->i_rdev = to_kdev_t(rdev);
	} else if (S_ISFIFO(mode))
		;//inode->i_fop = &def_fifo_fops;
	else if (S_ISSOCK(mode))
		;//inode->i_fop = &bad_sock_fops;
	else
		printk(KERN_DEBUG "init_special_inode: bogus imode (%o)\n", mode);
}