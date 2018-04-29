/*
 *  Portions of this file are taken from linux kernel 2.4.25
 *  linux/fs/super.c
 *
 *  Copyright (C) 1991, 1992  Linus Torvalds
 *
 *  super.c contains code to handle: - mount structures
 *                                   - super-block tables
 *                                   - filesystem drivers list
 *                                   - mount system call
 *                                   - umount system call
 *                                   - ustat system call
 *
 * GK 2/5/95  -  Changed to support mounting the root fs via NFS
 *
 *  Added kerneld support: Jacques Gelinas and Bjorn Ekwall
 *  Added change_root: Werner Almesberger & Hans Lermen, Feb '96
 *  Added options to /proc/mounts:
 *    Torbjörn Lindh (torbjorn.lindh@gopta.se), April 14, 1996.
 *  Added devfs support: Richard Gooch <rgooch@atnf.csiro.au>, 13-JAN-1998
 *  Heavily rewritten for 'one fs - one tree' dcache architecture. AV, Mar 2000
 */
/*  Copyright (C) 2004 Dan Mihailescu
 *  Copyright (C) 2004 Adrian Stanciu <adrian@sadyc.net>
 */

#include "todo.h"
#include <linux/slab.h>
#include <linux/locks.h>
#include <linux/smp_lock.h>
//#include <linux/devfs_fs_kernel.h>
//#include <linux/major.h>
//#include <linux/acct.h>
#include <linux/quotaops.h>

#include <asm/uaccess.h>

//#include <linux/kmod.h>
#define __NO_VERSION__
#include <linux/module.h>

LIST_HEAD(super_blocks);
struct file_system_type *file_systems;
static rwlock_t file_systems_lock;

void super_entry()
{
	ExInitializeResourceLite(&(file_systems_lock.rwlock));
}

void super_unload()
{
	ExDeleteResourceLite(&(file_systems_lock.rwlock));
}

/* WARNING: This can be used only if we _already_ own a reference */
static void get_filesystem(struct file_system_type *fs)
{
	if (fs->owner)
		__MOD_INC_USE_COUNT(fs->owner);
}

static void put_filesystem(struct file_system_type *fs)
{
	if (fs->owner)
		__MOD_DEC_USE_COUNT(fs->owner);
}

static struct file_system_type **find_filesystem(const char *name)
{
	struct file_system_type **p;
	for (p=&file_systems; *p; p=&(*p)->next)
		if (strcmp((*p)->name,name) == 0)
			break;
	return p;
} 

/**
 *	register_filesystem - register a new filesystem
 *	@fs: the file system structure
 *
 *	Adds the file system passed to the list of file systems the kernel
 *	is aware of for mount and other syscalls. Returns 0 on success,
 *	or a negative errno code on an error.
 *
 *	The &struct file_system_type that is passed is linked into the kernel 
 *	structures and must not be freed until the file system has been
 *	unregistered.
 */

int register_filesystem(struct file_system_type * fs)
{
	int res = 0;
	struct file_system_type ** p;

	if (!fs)
		return -EINVAL;
	if (fs->next)
		return -EBUSY;
	INIT_LIST_HEAD(&fs->fs_supers);
	write_lock(&file_systems_lock);
	p = find_filesystem(fs->name);
	if (*p)
		res = -EBUSY;
	else
		*p = fs;
	write_unlock(&file_systems_lock);
	return res;
}

/**
 *	unregister_filesystem - unregister a file system
 *	@fs: filesystem to unregister
 *
 *	Remove a file system that was previously successfully registered
 *	with the kernel. An error is returned if the file system is not found.
 *	Zero is returned on a success.
 *	
 *	Once this function has returned the &struct file_system_type structure
 *	may be freed or reused.
 */

int unregister_filesystem(struct file_system_type * fs)
{
	struct file_system_type ** tmp;

	write_lock(&file_systems_lock);
	tmp = &file_systems;
	while (*tmp) {
		if (fs == *tmp) {
			*tmp = fs->next;
			fs->next = NULL;
			write_unlock(&file_systems_lock);
			return 0;
		}
		tmp = &(*tmp)->next;
	}
	write_unlock(&file_systems_lock);
	return -EINVAL;
}


/**
 *	alloc_super	-	create new superblock
 *
 *	Allocates and initializes a new &struct super_block.  alloc_super()
 *	returns a pointer new superblock or %NULL if allocation had failed.
 */
struct super_block *alloc_super(void)
{
	static struct super_operations empty_sops = { NULL };
	struct super_block *s = kmalloc(sizeof(struct super_block),  GFP_USER);
	if (s) {
		memset(s, 0, sizeof(struct super_block));
		INIT_LIST_HEAD(&s->s_dirty);
		INIT_LIST_HEAD(&s->s_locked_inodes);
		INIT_LIST_HEAD(&s->s_files);
		INIT_LIST_HEAD(&s->s_instances);
		init_rwsem(&s->s_umount);
		sema_init(&s->s_lock, 1);
		down_write(&s->s_umount);
		s->s_count = S_BIAS;
		atomic_set(&s->s_active, 1);
		sema_init(&s->s_vfs_rename_sem,1);
		sema_init(&s->s_nfsd_free_path_sem,1);
//		sema_init(&s->s_dquot.dqio_sem, 1);
//		sema_init(&s->s_dquot.dqoff_sem, 1);
		s->s_maxbytes = MAX_NON_LFS;
		s->s_op = &empty_sops;
		s->dq_op = sb_dquot_ops;
		s->s_qcop = sb_quotactl_ops;
	}
	return s;
}

/**
 *      destroy_super   -       frees a superblock
 *      @s: superblock to free
 *
 *      Frees a superblock.
 */
static inline void destroy_super(struct super_block *s)
{
	kfree(s);
}

/* Superblock refcounting  */

/**
 *	deactivate_super	-	turn an active reference into temporary
 *	@s: superblock to deactivate
 *
 *	Turns an active reference into temporary one.  Returns 0 if there are
 *	other active references, 1 if we had deactivated the last one.
 */
static inline int deactivate_super(struct super_block *s)
{
	if (!atomic_dec_and_lock(&s->s_active, &sb_lock))
		return 0;
	s->s_count -= S_BIAS-1;
	spin_unlock(&sb_lock);
	return 1;
}

/**
 *	put_super	-	drop a temporary reference to superblock
 *	@s: superblock in question
 *
 *	Drops a temporary reference, frees superblock if there's no
 *	references left.
 */
static inline void put_super(struct super_block *s)
{
	spin_lock(&sb_lock);
	if (!--s->s_count)
		destroy_super(s);
	spin_unlock(&sb_lock);
}

/**
 *	insert_super	-	put superblock on the lists
 *	@s:	superblock in question
 *	@type:	filesystem type it will belong to
 *
 *	Associates superblock with fs type and puts it on per-type and global
 *	superblocks' lists.  Should be called with sb_lock held; drops it.
 */
void insert_super(struct super_block *s, struct file_system_type *type)
{
	s->s_type = type;
	list_add(&s->s_list, super_blocks.prev);
	list_add(&s->s_instances, &type->fs_supers);
	spin_unlock(&sb_lock);
	get_filesystem(type);
}

/**
 *	remove_super	-	makes superblock unreachable
 *	@s:	superblock in question
 *
 *	Removes superblock from the lists, unlocks it, drop the reference
 *	and releases the hosting device.  @s should have no active
 *	references by that time and after remove_super() it's essentially
 *	in rundown mode - all remaining references are temporary, no new
 *	reference of any sort are going to appear and all holders of
 *	temporary ones will eventually drop them.  At that point superblock
 *	itself will be destroyed; all its contents is already gone.
 */
static void remove_super(struct super_block *s)
{
	kdev_t dev = s->s_dev;
	struct block_device *bdev = s->s_bdev;
	struct file_system_type *fs = s->s_type;

	spin_lock(&sb_lock);
	list_del(&s->s_list);
	list_del(&s->s_instances);
	spin_unlock(&sb_lock);
	up_write(&s->s_umount);
	put_super(s);
	put_filesystem(fs);
/*	if (bdev)
		blkdev_put(bdev, BDEV_FS);
	else
		put_anon_dev(dev);*/
}

static inline struct super_block * find_super(kdev_t dev)
{
	struct list_head *p;

	list_for_each(p, &super_blocks) {
		struct super_block * s = sb_entry(p);
			if (s->s_dev == dev) {
				s->s_count++;
				return s;
			}
	}
	return NULL;
}

void drop_super(struct super_block *sb)
{
	up_read(&sb->s_umount);
	put_super(sb);
}

/**
 *	get_super	-	get the superblock of a device
 *	@dev: device to get the superblock for
 *	
 *	Scans the superblock list and finds the superblock of the file system
 *	mounted on the device given. %NULL is returned if no match is found.
 */

struct super_block * get_super(kdev_t dev)
{
	struct super_block * s;

	if (!dev)
		return NULL;
	restart:
		spin_lock(&sb_lock);
		s = find_super(dev);
		if (s) {
			spin_unlock(&sb_lock);
			down_read(&s->s_umount);
			if (s->s_root)
				return s;
			drop_super(s);
			goto restart;
		}
		spin_unlock(&sb_lock);
		return NULL;
} 



void kill_super(struct super_block *sb)
{
	struct dentry *root = sb->s_root;
	struct file_system_type *fs = sb->s_type;
	struct super_operations *sop = sb->s_op;

	printk("kill_super: start\n");
	if (!deactivate_super(sb))
		return;

	down_write(&sb->s_umount);
	sb->s_root = NULL;
	/* Need to clean after the sucker */
	if (fs->fs_flags & FS_LITTER)
		d_genocide(root);
	printk("kill_super: calling shrink_dcache_parent\n");
	shrink_dcache_parent(root);
	dput(root);
	printk("kill_super: calling fsync_super\n");
	fsync_super(sb);
	lock_super(sb);
	lock_kernel();
	sb->s_flags &= ~MS_ACTIVE;
	printk("kill_super: calling invalidate_inodes\n");
	invalidate_inodes(sb);	/* bad name - it should be evict_inodes() */
	if (sop) {
		printk("kill_super: calling super operations\n");
		if (sop->write_super && sb->s_dirt)
			sop->write_super(sb);
		if (sop->put_super)
			sop->put_super(sb);
	}

	/* Forget any remaining inodes */
	if (invalidate_inodes(sb)) {
		printk(KERN_ERR "VFS: Busy inodes after unmount. "
			"Self-destruct in 5 seconds.  Have a nice day...\n");
	}

	unlock_kernel();
	unlock_super(sb);
	printk("kill_super: calling remove_super\n");
	remove_super(sb);
}
