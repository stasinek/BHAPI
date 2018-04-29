/*
 *  This file is a modified version from linux kernel 2.4.25
 *  linux/fs/readdir.c
 *
 *  Copyright (C) 1995  Linus Torvalds
 *
 *  Changes Copyright (C) 2004 Adrian Stanciu (adrian@sadyc.net)
 */

#include "todo.h"
#include <linux/sched.h>
#include <linux/mm.h>
#include <linux/errno.h>
#include <linux/stat.h>
#include <linux/file.h>
#include <linux/smp_lock.h>

#include <asm/uaccess.h>


int vfs_readdir(struct file *file, filldir_t filler, void *buf)
{
	struct inode *inode = file->f_dentry->d_inode;
	int res = -ENOTDIR;
//	printk("readdir: vfs_readdir\n");
	if (!file->f_op || !file->f_op->readdir)
		goto out;
	down(&inode->i_sem);
	down(&inode->i_zombie);
	res = -ENOENT;
	if (!IS_DEADDIR(inode)) {
		lock_kernel();
		res = file->f_op->readdir(file, buf, filler);
		unlock_kernel();
	}
	up(&inode->i_zombie);
	up(&inode->i_sem);
out:
	return res;
}

int dcache_dir_open(struct inode *inode, struct file *file)
{
//	static struct qstr cursor_name = {len:1, name:"."};
	static struct qstr cursor_name = {".", 1, 0};	//supported in VC

	file->private_data = d_alloc(file->f_dentry, &cursor_name);

	return file->private_data ? 0 : -ENOMEM;
}

int dcache_dir_close(struct inode *inode, struct file *file)
{
	dput(file->private_data);
	return 0;
}

loff_t dcache_dir_lseek(struct file *file, loff_t offset, int origin)
{
	down(&file->f_dentry->d_inode->i_sem);
	switch (origin) {
		case 1:
			offset += file->f_pos;
		case 0:
			if (offset >= 0)
				break;
		default:
			up(&file->f_dentry->d_inode->i_sem);
			return -EINVAL;
	}
	if (offset != file->f_pos) {
		file->f_pos = offset;
		if (file->f_pos >= 2) {
			struct list_head *p;
			struct dentry *cursor = file->private_data;
			loff_t n = file->f_pos - 2;

			spin_lock(&dcache_lock);
			list_del(&cursor->d_child);
			p = file->f_dentry->d_subdirs.next;
			while (n && p != &file->f_dentry->d_subdirs) {
				struct dentry *next;
				next = list_entry(p, struct dentry, d_child);
				if (!list_empty(&next->d_hash) && next->d_inode)
					n--;
				p = p->next;
			}
			list_add_tail(&cursor->d_child, p);
			spin_unlock(&dcache_lock);
		}
	}
	up(&file->f_dentry->d_inode->i_sem);
	return offset;
}

int dcache_dir_fsync(struct file * file, struct dentry *dentry, int datasync)
{
	return 0;
}

/*
 * Directory is locked and all positive dentries in it are safe, since
 * for ramfs-type trees they can't go away without unlink() or rmdir(),
 * both impossible due to the lock on directory.
 */

int dcache_readdir(struct file * filp, void * dirent, filldir_t filldir)
{
	struct dentry *dentry = filp->f_dentry;
	struct dentry *cursor = filp->private_data;
	struct list_head *p, *q = &cursor->d_child;
	ino_t ino;
	int i = (int) filp->f_pos;

	switch (i) {
		case 0:
			ino = dentry->d_inode->i_ino;
			if (filldir(dirent, ".", 1, i, ino, DT_DIR) < 0)
				break;
			filp->f_pos++;
			i++;
			/* fallthrough */
		case 1:
			spin_lock(&dcache_lock);
			ino = dentry->d_parent->d_inode->i_ino;
			spin_unlock(&dcache_lock);
			if (filldir(dirent, "..", 2, i, ino, DT_DIR) < 0)
				break;
			filp->f_pos++;
			i++;
			/* fallthrough */
		default:
			spin_lock(&dcache_lock);
			if (filp->f_pos == 2) {
				list_del(q);
				list_add(q, &dentry->d_subdirs);
			}
			for (p=q->next; p != &dentry->d_subdirs; p=p->next) {
				struct dentry *next;
				next = list_entry(p, struct dentry, d_child);
				if (list_empty(&next->d_hash) || !next->d_inode)
					continue;

				spin_unlock(&dcache_lock);
				if (filldir(dirent, next->d_name.name, next->d_name.len, filp->f_pos, next->d_inode->i_ino, DT_UNKNOWN) < 0)
					return 0;
				spin_lock(&dcache_lock);
				/* next is still alive */
				list_del(q);
				list_add(q, p);
				p = q;
				filp->f_pos++;
			}
			spin_unlock(&dcache_lock);
	}
	UPDATE_ATIME(dentry->d_inode);
	return 0;
}

/*struct file_operations dcache_dir_ops = {
	open:		dcache_dir_open,
	release:	dcache_dir_close,
	llseek:		dcache_dir_lseek,
	read:		generic_read_dir,
	readdir:	dcache_readdir,
	fsync:		dcache_dir_fsync,
};*/
// visual C++ sucks --sadyc
struct file_operations dchache_dir_ops = {
	NULL,
	dcache_dir_lseek,
	generic_read_dir,
	NULL,
	dcache_readdir,
	NULL,
	NULL,
	NULL,
	dcache_dir_open,
	NULL,
	dcache_dir_close,
	dcache_dir_fsync,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL
};

