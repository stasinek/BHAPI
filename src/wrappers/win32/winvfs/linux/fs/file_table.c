/*
 *  Portions of this file are taken from linux kernel 2.4.25
 *  linux/fs/file_table.c
 *
 *  Copyright (C) 1991, 1992  Linus Torvalds
 *  Copyright (C) 1997 David S. Miller (davem@caip.rutgers.edu)
 *
 *  Changes Copyright (C) 2004 Adrian Stanciu (adrian@sadyc.net)
 */

#include "todo.h"
#include <linux/slab.h>
#include <linux/file.h>
//#include <linux/init.h>
//#include <linux/module.h>
//#include <linux/smp_lock.h>
//#include <linux/iobuf.h>

struct files_stat_struct files_stat = {0};

/* Here the new files go */
static LIST_HEAD(anon_list);
/* And here the free ones sit */
//static LIST_HEAD(free_list);
/* public *and* exported. Not pretty! */
spinlock_t files_lock = SPIN_LOCK_UNLOCKED;

/* Find an unused file structure and return a pointer to it.
 * Returns NULL, if there are no more free file structures or
 * we run out of memory.
 *
 * SMP-safe.
 */
struct file * get_empty_filp(void)
{
	static int old_max = 0;
	struct file * f;

	printk("get_empty_filp\n");

	f = kmem_cache_alloc(filp_cachep, SLAB_KERNEL);
	file_list_lock();
	if (f) {
		files_stat.nr_files++;
		printk("get_empty_filp: nr_files: %d\n", (int)files_stat.nr_files);
		memset(f, 0, sizeof(*f));
		atomic_set(&f->f_count,1);
		f->f_version = ++event;
		f->f_uid = current->fsuid;
		f->f_gid = current->fsgid;
		list_add(&f->f_list, &anon_list);
		file_list_unlock();
		return f;
	}
    /* Big problems... */
	file_list_unlock();
	printk(KERN_WARNING "VFS: filp allocation failed\n");
	return NULL;
}

void fput(struct file * file)
{
	struct dentry * dentry = file->f_dentry;
	struct vfsmount * mnt = file->f_vfsmnt;
	struct inode * inode = dentry->d_inode;

	if (atomic_dec_and_test(&file->f_count)) {
		//locks_remove_flock(file);

		//if (file->f_iobuf)
		//	free_kiovec(1, &file->f_iobuf);

		if (file->f_op && file->f_op->release)
			file->f_op->release(inode, file);
		fops_put(file->f_op);
		if (file->f_mode & FMODE_WRITE)
			put_write_access(inode);

		file_list_lock();
		file->f_dentry = NULL;
		file->f_vfsmnt = NULL;
		list_del(&file->f_list);
//		list_add(&file->f_list, &free_list);
		files_stat.nr_files--;
		file_list_unlock();
		printk("put_filp: nr_files: %d\n", (int)files_stat.nr_files);
		kmem_cache_free(filp_cachep, file);

		dput(dentry);
		mntput(mnt);
	}
}

void put_filp(struct file *file)
{
	printk("put_filp\n");
	if(atomic_dec_and_test(&file->f_count)) {
		printk("put_filp: switching list\n");
		file_list_lock();
		list_del(&file->f_list);
//		list_add(&file->f_list, &free_list);
		files_stat.nr_files--;
		file_list_unlock();
		printk("put_filp: nr_free_files: %d\n", (int)files_stat.nr_files);
		kmem_cache_free(filp_cachep, file);
	}
}

void file_move(struct file *file, struct list_head *list)
{
	if (!list)
		return;
	file_list_lock();
	list_del(&file->f_list);
	list_add(&file->f_list, list);
	file_list_unlock();
}
