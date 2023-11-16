/*
 * linux/lib/kernel.c
 *
 * Portions of this file are taken from various files in Linux Kernel 2.4.25
 *
 * misc kernel stuff
 *
 */


#include "todo.h"
#include "vfs.h"
#include "linux/kernel.h"
#include "linux/slab.h"
#include "linux/blkdev.h"
#include "linux/locks.h"


spinlock_t kernel_flag;
struct task_struct current_struct;

void kernel_entry()
{
	KeInitializeSpinLock(&(kernel_flag.lock));

	memset(&current_struct, 0, sizeof(current_struct));
	current_struct.fs = kmalloc(sizeof(struct fs_struct), GFP_KERNEL);
	memset(current_struct.fs, 0, sizeof(struct fs_struct));
//	current_struct.fs->count.counter = 1;
	ExInitializeResourceLite(&(current_struct.fs->lock.rwlock));
	current_struct.fs->umask = 0022;
}

void kernel_unload()
{
	ExDeleteResourceLite(&(current_struct.fs->lock.rwlock));
	kfree(current_struct.fs);
}

void ll_rw_block(int rw, int nr, struct buffer_head * bhs[])
{
	//unsigned int major;
	int correct_size;
	int i;
	NTSTATUS status;
	ULONG blk_count;
	LARGE_INTEGER start_blk;

	if (!nr)
		return;

	//major = MAJOR(bhs[0]->b_dev);

	/* Determine correct block size for this device. */
	correct_size = get_hardsect_size(bhs[0]->b_dev);

	/* Verify requested block sizes. */
	for (i = 0; i < nr; i++) {
		struct buffer_head *bh = bhs[i];
		if (bh->b_size % correct_size) {
			printk("ll_rw_block: device %s: "
				"only %d-char blocks implemented (%u)\n",
				kdevname(bhs[0]->b_dev),
				correct_size, bh->b_size);
			goto sorry;
		}
	}

	if ((rw & WRITE) && is_read_only(bhs[0]->b_dev)) {
		printk("Can't write to read-only device %s\n",
			kdevname(bhs[0]->b_dev));
		goto sorry;
	}

	for (i = 0; i < nr; i++) {
		struct buffer_head *bh = bhs[i];

		lock_buffer(bh);

		start_blk.QuadPart = bh->b_rsector;
		blk_count = (ULONG)(bh->b_size/get_hardsect_size(bh->b_dev));

		/* We have the buffer lock */
		atomic_inc(&bh->b_count);
		//bh->b_end_io = end_buffer_io_sync;

		switch(rw) {
		case WRITE:
			//if (!atomic_set_buffer_clean(bh))
			/* Hmmph! Nothing to write */
			//	goto end_io;
			//__mark_buffer_clean(bh);
			status = VfsWritePhysicalBlocks(bh->b_dev, (PVOID)bh->b_data, start_blk, blk_count);
			break;

		case READA:
		case READ:
			//if (buffer_uptodate(bh))
			/* Hmmph! Already have it */
			//	goto end_io;
			status = VfsReadPhysicalBlocks(bh->b_dev, (PVOID)bh->b_data, start_blk, blk_count);
			break;
		default:
			BUG();
//end_io:
		//bh->b_end_io(bh, test_bit(BH_Uptodate, &bh->b_state));
			continue;
		}

		//submit_bh(rw, bh);
	}
	return;

sorry:
	/* Make sure we don't get infinite dirty retries.. */
	//for (i = 0; i < nr; i++)
	//	mark_buffer_clean(bhs[i]);
	;
}

int is_read_only(kdev_t dev)
{
	return 0;
}

/*
 * Check whether we're fsgid/egid or in the supplemental group..
 */
int in_group_p(gid_t grp)
{
	int retval = 1;
//	if (grp != current->fsgid)
//		retval = supplemental_group_member(grp);
	return retval;
} 
