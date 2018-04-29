/*
 *  Portions of this file are taken from linux kernel 2.4.25
 *  linux/fs/namespace.c
 *
 * (C) Copyright Al Viro 2000, 2001
 *	Released under GPL v2.
 *
 * Based on code from fs/super.c, copyright Linus Torvalds and others.
 * Heavily rewritten.
 */
/*  Copyright (C) 2004 Adrian Stanciu <adrian@sadyc.net>
 */

#include "todo.h"
#include <linux/slab.h>
#include <linux/smp_lock.h>
#include <linux/init.h>
#include <linux/quotaops.h>

void kill_super(struct super_block *sb);

static int do_umount(struct vfsmount *mnt, int flags)
{
	struct super_block * sb = mnt->mnt_sb;
	int retval = 0;

	printk("do_umount\n");

	/*
	 * If we may have to abort operations to get out of this
	 * mount, and they will themselves hold resources we must
	 * allow the fs to do things. In the Unix tradition of
	 * 'Gee thats tricky lets do it in userspace' the umount_begin
	 * might fail to complete on the first run through as other tasks
	 * must return, and the like. Thats for the mount program to worry
	 * about for the moment.
	 */

	lock_kernel();
	if( (flags&MNT_FORCE) && sb->s_op->umount_begin)
		sb->s_op->umount_begin(sb);
	unlock_kernel();

	spin_lock(&dcache_lock);

	if (atomic_read(&sb->s_active) == 1) {
		/* last instance - try to be smart */
		spin_unlock(&dcache_lock);
		lock_kernel();
		DQUOT_OFF(sb);
//		acct_auto_close(sb->s_dev);
		unlock_kernel();
		spin_lock(&dcache_lock);
	}
	retval = -EBUSY;
	if (atomic_read(&mnt->mnt_count) == 2 || flags & MNT_DETACH) {
//		if (!list_empty(&mnt->mnt_list))
//			umount_tree(mnt);
			__mntput(mnt);    // free it --sadyc
		retval = 0;
	}
	spin_unlock(&dcache_lock);
	return retval;
}

/*
 * sys_umount modified for WinVFS use --sadyc
 */

asmlinkage long sys_umount(struct super_block *mnt_sb, int flags)
{
	struct vfsmount mnt;
	struct nameidata nd;
	int retval;

	mnt.mnt_sb = mnt_sb;
	mnt.mnt_root = mnt_sb->s_root;

	retval = do_umount(&mnt, flags);
	return retval;
}

void __mntput(struct vfsmount *mnt)
{
	struct super_block *sb = mnt->mnt_sb;
	dput(mnt->mnt_root);
//	free_vfsmnt(mnt); -- no need we allocate vfsmount structure on stack. --sadyc
	kill_super(sb);
}
