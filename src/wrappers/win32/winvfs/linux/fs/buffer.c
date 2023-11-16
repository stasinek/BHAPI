/*
 *  Portions of this file are taken from linux kernel 2.4.25
 *  linux/fs/buffer.c
 *
 *  Copyright (C) 2004 Dan Mihailescu
 */
 
#include "todo.h"
#include <linux/sched.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/locks.h>
#include <linux/errno.h>
#include <linux/smp_lock.h>
#include <linux/blkdev.h>
#include <linux/file.h>
#include <linux/quotaops.h>
#include <linux/highmem.h>
#include "vfs.h"

#include "debug.h"


#define BH_ENTRY(list)					list_entry((list), struct buffer_head, b_inode_buffers)

extern PUCHAR VfsNtStatusToString(IN NTSTATUS Status);

struct buffer_head * bread(kdev_t dev, int block, int size)
{
	struct buffer_head *bh = NULL;
	NTSTATUS status = STATUS_SUCCESS;
	LARGE_INTEGER start_blk = {0,0};
	ULONG blk_count = 0;

	printk("bread(block: %d, size: %d)\n", block, size);
	bh = kmem_cache_alloc(bh_cachep, GFP_KERNEL);
	if (!bh)
		return NULL;
	RtlZeroMemory(bh, sizeof(struct buffer_head));
	bh->b_data = (char*)kmalloc(size, GFP_KERNEL);
	if (!bh->b_data) {
		printk("bread: FAILED to allocate buffer_head data memory\n");
		goto out;
	}

	bh->b_blocknr = block;
	bh->b_size = (unsigned short)size;
	bh->b_dev = dev;
	atomic_set(&bh->b_count, 1);
	bh->b_rsector = (unsigned long)((unsigned long)block*(size/get_hardsect_size(dev)));

	start_blk.QuadPart = bh->b_rsector;
	blk_count = (ULONG)(size/get_hardsect_size(dev));
	// here i should first check if the dev is mounted
	// and then do a cache read from the virtual volume file object
	status = VfsReadPhysicalBlocks(dev, (PVOID)bh->b_data, start_blk, blk_count);

//	dump_buffer(bh->b_data, bh->b_size, "bread");

	if (NT_SUCCESS(status)) {
		set_bit(BH_Uptodate, &bh->b_state);
		return bh;
	}
	printk("bread: VfsReadPhysicalBlocks failed. status(%d): %s\n", (int) status, 
		VfsNtStatusToString(status));
	BUG();

	kfree(bh->b_data);
out:
	kmem_cache_free(bh_cachep, bh);
	return NULL;
}

void __brelse(struct buffer_head *bh)
{
	if (bh && bh->b_data)
		kfree(bh->b_data);
	if (bh)
		kmem_cache_free(bh_cachep, bh);
	bh = NULL;
	return;
}

// aici pur si simplu scriu in cache sau direct pe disk
// desi nu trebuie sa se scrie pe disk daca volumul nu a fost montat
// iar daca a fost montat a fost initializat si cachingul pe fisierul volum
void mark_buffer_dirty(struct buffer_head *bh)
{
	NTSTATUS status;
	ULONG blk_count;
	LARGE_INTEGER start_blk;

	start_blk.QuadPart = bh->b_rsector;
	blk_count = (ULONG)(bh->b_size/bh->b_dev->SectorSize);
//	printk("mark_buffer_dirty: writing... (start_blk: %d, count: %d)\n", start_blk, blk_count);
	status = VfsWritePhysicalBlocks(bh->b_dev, (PVOID)bh->b_data, start_blk, blk_count);
}

void set_bh_page (struct buffer_head *bh, struct page *page, unsigned long offset)
{
	ASSERT(offset < PAGE_SIZE);
	bh->b_data = page_address(page) + offset;
	bh->b_page = page;
}

struct buffer_head * get_unused_buffer_head(int async)
{
	struct buffer_head * bh;
 
	if (bh = kmem_cache_alloc(bh_cachep, GFP_KERNEL)) {
		bh->b_blocknr = -1;
		bh->b_this_page = NULL;
		return bh;
	}
	return NULL;
}

void __put_unused_buffer_head(struct buffer_head *bh)
{
	kmem_cache_free(bh_cachep, bh);
}

static struct buffer_head * create_buffers(struct page * page, unsigned long size, int async)
{
	struct buffer_head *bh, *head;
	long offset;
//	printk("buffer.c: create_buffers block_size=%u, page_size=%u\n", size, (unsigned)PAGE_SIZE);

try_again:
	head = NULL;
	offset = PAGE_SIZE;
	while ((offset -= size) >= 0) {
//		printk("***** buffer.c: create_buffers: allocating 1 buffer\n");
		bh = get_unused_buffer_head(async);
		if (!bh)
			goto no_grow;
  
		bh->b_dev = NODEV;
		bh->b_this_page = head;
		head = bh;
  
		bh->b_state = 0;
		bh->b_next_free = NULL;
		bh->b_pprev = NULL;
		atomic_set(&bh->b_count, 0);
		bh->b_size = (unsigned short)size;
  
		set_bh_page(bh, page, offset);
  
		bh->b_list = BUF_CLEAN;
		bh->b_end_io = NULL;
	}
//	printk("buffer.c: create_buffers: done\n");
	return head;
 
no_grow:
	printk("buffer.c: create_buffers: no_grow\n");
	if (head) {
		do {
			bh = head;
			head = head->b_this_page;
			__put_unused_buffer_head(bh);
		} while (head);
	}
	if (!async)
		return NULL;
 
	goto try_again;
}

void create_empty_buffers(struct page *page, kdev_t dev, unsigned long blocksize)
{
	struct buffer_head *bh, *head, *tail;
 
	head = create_buffers(page, blocksize, 1);
	ASSERT(!page->buffers);
 
	bh = head;
	do {
		bh->b_dev = dev;
		bh->b_blocknr = 0;
		bh->b_end_io = NULL;
		tail = bh;
		bh = bh->b_this_page;
	} while (bh);
	tail->b_this_page = head;
	page->buffers = head;
	page_cache_get(page);
}


int block_read_full_page(struct page *page, get_block_t *get_block)
{
	struct inode *inode = page->mapping->host;
	unsigned long iblock, lblock;
	struct buffer_head *bh, *head, *arr[MAX_BUF_PER_PAGE];
	unsigned int blocksize, blocks;
	int nr, i;
 
	blocksize = 1 << inode->i_blkbits;

	printk("block_read_full_page: was called (\
		inode.i_no= %d, inode.i_size= %d inode.i_blkbits= %d)\n", 
		(int)inode->i_ino, (int)inode->i_size, (int)inode->i_blkbits);

	if (!page->buffers)
		create_empty_buffers(page, inode->i_dev, blocksize);
	head = page->buffers;
	blocks = PAGE_CACHE_SIZE >> inode->i_blkbits;

//	printk("block_read_full_page: blocks=%d, page.index=%u\n", blocks, page->index);

	iblock = page->index << (PAGE_CACHE_SHIFT - inode->i_blkbits);
	lblock = (unsigned long)((inode->i_size+blocksize-1) >> inode->i_blkbits);
	bh = head;
	nr = 0;
	i = 0;

//	printk("block_read_full_page: iblock = %u, lblock = %u\n", iblock, lblock);
 
	do {
		//memset((char*)kmap(page) + i*blocksize, 'a'+i, blocksize);
		memset((char*)kmap(page) + i*blocksize, 0, blocksize);
		if (iblock < lblock) {
			int ret = get_block(inode, iblock, bh, 0);
			ASSERT(!ret);

			if (buffer_mapped(bh)) {
				// de optimizat -- todo
				struct buffer_head *bbh = bread(bh->b_dev, bh->b_blocknr, blocksize);
				if (bbh==NULL) {
					// we might leak memory here, --sadyc
					return -ENOMEM;
				}
//				if (!bbh)
//					BUG("buffer.c: block_read_full_page: CANNOT allocate memory for buffer_head\n");
//				ASSERT(bbh);
				RtlCopyMemory(bh->b_data, bbh->b_data, bbh->b_size);
				brelse(bbh);

			}
		}
	} while (i++, iblock++, (bh = bh->b_this_page) != head);

	// --- erori --- todo
	SetPageUptodate(page);
 
 return 0;
}

static int __block_write_full_page(struct inode *inode, struct page *page, get_block_t *get_block)
{
	int err, i;
	unsigned long block;
	struct buffer_head *bh, *head;
	int need_unlock;

	ASSERT(!PageLocked(page));

	if (!page->buffers)
		create_empty_buffers(page, inode->i_dev, 1 << inode->i_blkbits);
	head = page->buffers;

	block = page->index << (PAGE_CACHE_SHIFT - inode->i_blkbits);
	bh = head;
	i = 0;

	/* Stage 1: make sure we have all the buffers mapped! */
	do {
		if (!buffer_mapped(bh)) {
			err = get_block(inode, block, bh, 1);
			if (err)
				goto out;
		}
		bh = bh->b_this_page;
		block++;
	} while (bh != head);

	/* Stage 2: lock the buffers, mark them clean */
	do {
		lock_buffer(bh);
		set_bit(BH_Uptodate, &bh->b_state);
		clear_bit(BH_Dirty, &bh->b_state);
		bh = bh->b_this_page;
	} while (bh != head);

	/* Stage 3: submit the IO */
	do {
		struct buffer_head *next = bh->b_this_page;
		mark_buffer_dirty(bh);	// submit_bh(WRITE, bh)	// asta o sa-l scrie direct pe disk
		bh = next;
	} while (bh != head);

	SetPageUptodate(page);
	return 0;

out:
	/*
	 * ENOSPC, or some other error.  We may already have added some
	 * blocks to the file, so we need to write these out to avoid
	 * exposing stale data.
	 */
	// nu ma intereseaza

	ClearPageUptodate(page);
	UnlockPage(page);
	return err;
}

// aici tebuie sa citesc blocurile asa cum sunt ele pe disk
// ca la commit write sa fac un memcpy peste zona care ma intereseaza
static int __block_prepare_write(struct inode *inode, struct page *page,
								 unsigned from, unsigned to, get_block_t *get_block)
{
	unsigned block_start, block_end;
	unsigned long block;
	int err = 0;
	unsigned blocksize, bbits;
	struct buffer_head *bh, *head, *wait[2], **wait_bh=wait;
	char *kaddr = kmap(page);
	NTSTATUS status = STATUS_SUCCESS;
	LARGE_INTEGER start_blk = {0,0};
	ULONG blk_count = 0;

	blocksize = 1 << inode->i_blkbits;
	if (!page->buffers)
		create_empty_buffers(page, inode->i_dev, blocksize);
	head = page->buffers;

	bbits = inode->i_blkbits;
	block = page->index << (PAGE_CACHE_SHIFT - bbits);
	
	for(bh = head, block_start = 0; bh != head || !block_start;
		block++, block_start=block_end, bh = bh->b_this_page) {
			ASSERT(bh);
			block_end = block_start+blocksize;
			if (block_end <= from)
				continue;
			if (block_start >= to)
				break;
			clear_bit(BH_New, &bh->b_state);
			if (!buffer_mapped(bh)) {
				err = get_block(inode, block, bh, 1);
				if (err)
					goto out;
				if (buffer_new(bh)) {
					if (Page_Uptodate(page)) {
						set_bit(BH_Uptodate, &bh->b_state);
						continue;
					}
					if (block_end > to)
						memset(kaddr+to, 0, block_end-to);
					if (block_start < from)
						memset(kaddr+block_start, 0, from-block_start);
					continue;
				}
			}
			if (Page_Uptodate(page)) {
				set_bit(BH_Uptodate, &bh->b_state);
				continue; 
			}
			if (!buffer_uptodate(bh) && !buffer_delay(bh) &&
				(block_start < from || block_end > to)) {
				bh->b_rsector = (unsigned long)(bh->b_blocknr*bh->b_size/bh->b_dev->SectorSize);
				start_blk.QuadPart = bh->b_rsector;
				blk_count = (ULONG)(bh->b_size/bh->b_dev->SectorSize);
				status = VfsReadPhysicalBlocks(bh->b_dev, (PVOID)bh->b_data, start_blk, blk_count);
				if (!NT_SUCCESS(status))
					return -EIO;
			}
		}
		return 0;
out:
		bh = head;
		to = block_start;
		block_start = 0;
		do {
			block_end = block_start+blocksize;
			if (block_end <= from)
				goto next_bh;
			if (block_start >= to)
				break;
			if (buffer_new(bh) && !buffer_uptodate(bh)) {
				memset(kaddr+block_start, 0, bh->b_size);
				set_bit(BH_Uptodate, &bh->b_state);
				mark_buffer_dirty(bh);
			}
next_bh:
			block_start = block_end;
			bh = bh->b_this_page;
		} while (bh != head);
		return err;
}

static int __block_commit_write(struct inode *inode, struct page *page, 
								unsigned from, unsigned to)
{
	unsigned block_start, block_end;
	int partial = 0, need_balance_dirty = 0;
	unsigned blocksize;
	struct buffer_head *bh, *head;

	blocksize = 1 << inode->i_blkbits;
	for (bh = head = page->buffers, block_start = 0;
		bh != head || !block_start;
		block_start=block_end, bh = bh->b_this_page) {
			block_end = block_start + blocksize;
			if (block_end <= from || block_start >= to) {
				if (!buffer_uptodate(bh))
					partial = 1;
			} else {
				set_bit(BH_Uptodate, &bh->b_state);
				mark_buffer_dirty(bh);
			}
		}
	/*
	 * is this a partial write that happened to make all buffers
	 * uptodate then we can optimize away a bogus readpage() for
	 * the next read(). Here we 'discover' wether the page went
	 * uptodate as a result of this (potentially partial) write
	 */
	if (!partial)
		SetPageUptodate(page);
	return 0;
}

int block_write_full_page(struct page *page, get_block_t *get_block)
{
	struct inode *inode = page->mapping->host;
	unsigned long end_index = (unsigned long)(inode->i_size >> PAGE_CACHE_SHIFT);
	unsigned offset;
	int err;

	/* easy case */
	if (page->index < end_index)
		return __block_write_full_page(inode, page, get_block);

	/* things got complicated... */
	offset = (unsigned)(inode->i_size & (PAGE_CACHE_SIZE-1));
	/* OK, are we completely out? */
	if (page->index >= end_index+1 || !offset) {
		UnlockPage(page);
		return -EIO;
	}

	/* Sigh... will have to work, then... */
	err = __block_prepare_write(inode, page, 0, offset, get_block);
	if (!err) {
		memset(page_address(page) + offset, 0, PAGE_CACHE_SIZE - offset);
		__block_commit_write(inode, page, 0, offset);
done:
		kunmap(page);
		UnlockPage(page);
		return err;
	}
	ClearPageUptodate(page);
	goto done;
}

int block_prepare_write(struct page *page, unsigned from, unsigned to, get_block_t *get_block)
{
	struct inode *inode = page->mapping->host;
	int err = __block_prepare_write(inode, page, from, to, get_block);
	if (err) {
		ClearPageUptodate(page);
		kunmap(page);
	}
	return err;
}

int generic_block_bmap(struct address_space *mapping, long block, get_block_t *get_block)
{
	struct buffer_head tmp;
	struct inode *inode = mapping->host;
	tmp.b_state = 0;
	tmp.b_blocknr = 0;
	get_block(inode, block, &tmp, 0);
	return tmp.b_blocknr;
}

int block_sync_page(struct page *page)
{
	//run_task_queue(&tq_disk);
	return 0;
}

int generic_commit_write(struct file *file, struct page *page, unsigned from, unsigned to)
{
	struct inode *inode = page->mapping->host;
	loff_t pos = ((loff_t)page->index << PAGE_CACHE_SHIFT) + to;
	__block_commit_write(inode,page,from,to);
	kunmap(page);
	if (pos > inode->i_size) {
		inode->i_size = pos;
		mark_inode_dirty(inode);
	}
	return 0;
}

int writeout_one_page(struct page *page)
{
	struct buffer_head *bh, *head = page->buffers;

	if (!PageLocked(page))
		BUG();
	bh = head;
	do {
		if (buffer_locked(bh) || !buffer_dirty(bh) || !buffer_uptodate(bh))
			continue;

			bh->b_flushtime = CURRENT_TIME;
			mark_buffer_dirty(bh);
			//ll_rw_block(WRITE, 1, &bh);
	} while ((bh = bh->b_this_page) != head);
	return 0;
}

int waitfor_one_page(struct page *page)
{
	int error = 0;
	struct buffer_head *bh, *head = page->buffers;

	bh = head;
	do {
		wait_on_buffer(bh);
		if (buffer_req(bh) && !buffer_uptodate(bh))
			error = -EIO;
	} while ((bh = bh->b_this_page) != head);
	return error;
}

void __wait_on_buffer(struct buffer_head * bh)
{
	;
}

int block_truncate_page(struct address_space *mapping, loff_t from, get_block_t *get_block)
{
	unsigned long index = (unsigned long)(from >> PAGE_CACHE_SHIFT);
	unsigned offset = (unsigned)(from & (PAGE_CACHE_SIZE-1));
	unsigned blocksize, iblock, length, pos;
	struct inode *inode = mapping->host;
	struct page *page;
	struct buffer_head *bh;
	int err;

	blocksize = 1 << inode->i_blkbits;
	length = offset & (blocksize - 1);

	/* Block boundary? Nothing to do */
	if (!length)
		return 0;
 
	length = blocksize - length;
	iblock = index << (PAGE_CACHE_SHIFT - inode->i_blkbits);

	page = grab_cache_page(mapping, index);
	err = -ENOMEM;
	if (!page)
		goto out;

	if (!page->buffers)
		create_empty_buffers(page, inode->i_dev, blocksize);

	/* Find the buffer that contains "offset" */
	bh = page->buffers;
	pos = blocksize;
	while (offset >= pos) {
		bh = bh->b_this_page;
		iblock++;
		pos += blocksize;
	}

	err = 0;
	if (!buffer_mapped(bh)) {
		/* Hole? Nothing to do */
		if (buffer_uptodate(bh))
			goto unlock;
			get_block(inode, iblock, bh, 0);
			/* Still unmapped? Nothing to do */
			if (!buffer_mapped(bh))
				goto unlock;
	}

	/* Ok, it's mapped. Make sure it's up-to-date */
	if (Page_Uptodate(page))
		set_bit(BH_Uptodate, &bh->b_state);

	if (!buffer_uptodate(bh) && !buffer_delay(bh)) {
		err = -EIO;
		ll_rw_block(READ, 1, &bh);
		wait_on_buffer(bh);
		/* Uhhuh. Read error. Complain and punt. */
		if (!buffer_uptodate(bh))
			goto unlock;
	}

	memset((char*)kmap(page) + offset, 0, length);
	//flush_dcache_page(page);
	kunmap(page);

	mark_buffer_dirty(bh);

	//if (!atomic_set_buffer_dirty(bh)) {
	//	__mark_dirty(bh);
		//buffer_insert_inode_data_queue(bh, inode);
		//balance_dirty();
	//}

	err = 0;

unlock:
	UnlockPage(page);
	page_cache_release(page);
out:
	return err;
}

void buffer_insert_list(struct buffer_head *bh, struct list_head *list)
{
	spin_lock(&lru_list_lock);
	if (buffer_attached(bh))
		list_del(&bh->b_inode_buffers);
	set_buffer_attached(bh);
	list_add_tail(&bh->b_inode_buffers, list);
	spin_unlock(&lru_list_lock);
} 

void unlock_buffer(struct buffer_head *bh)
{
	clear_bit(BH_Wait_IO, &bh->b_state);
	clear_bit(BH_Launder, &bh->b_state);
	/*
	 * When a locked buffer is visible to the I/O layer BH_Launder
	 * is set. This means before unlocking we must clear BH_Launder,
	 * mb() on alpha and then clear BH_Lock, so no reader can see
	 * BH_Launder set on an unlocked buffer and then risk to deadlock.
	 */
//	smp_mb__after_clear_bit();
	clear_bit(BH_Lock, &bh->b_state);
//	smp_mb__after_clear_bit();
//	if (waitqueue_active(&bh->b_wait))
//		wake_up(&bh->b_wait);
} 

/*
 * Ok, this is getblk, and it isn't very clear, again to hinder
 * race-conditions. Most of the code is seldom used, (ie repeating),
 * so it should be much more efficient than it looks.
 *
 * The algorithm is changed: hopefully better, and an elusive bug removed.
 *
 * 14.02.92: changed it to sync dirty buffers a bit: better performance
 * when the filesystem starts to get full of dirty blocks (I hope).
 */
struct buffer_head * getblk(kdev_t dev, int block, int size)
{
	return bread(dev, block, size);
/*	for (;;) {
		struct buffer_head * bh;

		bh = get_hash_table(dev, block, size);
		if (bh) {
			touch_buffer(bh);
			return bh;
		}

		if (!grow_buffers(dev, block, size))
			free_more_memory();        --tofix
	}*/
}

int block_symlink(struct inode *inode, const char *symname, int len)
{
	struct address_space *mapping = inode->i_mapping;
	struct page *page = grab_cache_page(mapping, 0);
	int err = -ENOMEM;
	char *kaddr;

	if (!page)
		goto fail;
	err = mapping->a_ops->prepare_write(NULL, page, 0, len-1);
	if (err)
		goto fail_map;
	kaddr = page_address(page);
	memcpy(kaddr, symname, len-1);
	mapping->a_ops->commit_write(NULL, page, 0, len-1);
	/*
	 * Notice that we are _not_ going to block here - end of page is
	 * unmapped, so this will only try to map the rest of page, see
	 * that it is unmapped (typically even will not look into inode -
	 * ->i_size will be enough for everything) and zero it out.
	 * OTOH it's obviously correct and should make the page up-to-date.
	 */
	err = mapping->a_ops->readpage(NULL, page);
	wait_on_page(page);
	page_cache_release(page);
	if (err < 0)
		goto fail;
	mark_inode_dirty(inode);
	return 0;
fail_map:
	UnlockPage(page);
	page_cache_release(page);
fail:
	return err;
} 

/*
 * bforget() is like brelse(), except it discards any
 * potentially dirty data.
 */
void __bforget(struct buffer_head * buf)
{
	mark_buffer_clean(buf);
	__brelse(buf);
} 

/*
 * A buffer may need to be moved from one buffer list to another
 * (e.g. in case it is not shared any more). Handle this.
 */
static void __refile_buffer(struct buffer_head *bh)
{
	/* --tofix
	int dispose = BUF_CLEAN;
	if (buffer_locked(bh))
		dispose = BUF_LOCKED;
	if (buffer_dirty(bh))
		dispose = BUF_DIRTY;
	if (dispose != bh->b_list) {
		__remove_from_lru_list(bh);
		bh->b_list = dispose;
		if (dispose == BUF_CLEAN)
			remove_inode_queue(bh);
		__insert_into_lru_list(bh, dispose);
	}
	*/
} 
void refile_buffer(struct buffer_head *bh)
{
	spin_lock(&lru_list_lock);
	__refile_buffer(bh);
	spin_unlock(&lru_list_lock);
}

int fsync_buffers_list(struct list_head *list)
{
	return 0;
}

/*
 *	filp may be NULL if called via the msync of a vma.
 */
 
int file_fsync(struct file *filp, struct dentry *dentry, int datasync)
{
	struct inode * inode = dentry->d_inode;
	struct super_block * sb;
	kdev_t dev;
	int ret;

	lock_kernel();
	/* sync the inode to buffers */
	write_inode_now(inode, 0);

	/* sync the superblock to buffers */
	sb = inode->i_sb;
	lock_super(sb);
	if (sb->s_op && sb->s_op->write_super)
		sb->s_op->write_super(sb);
	unlock_super(sb);

	/* .. finally sync the buffers to disk */
	dev = inode->i_dev;
	ret = sync_buffers(dev, 1);
	unlock_kernel();
	return ret;
} 

int sync_buffers(kdev_t dev, int wait)
{
	int err = 0;

	/* One pass for no-wait, three for wait:
	 * 0) write out all dirty, unlocked buffers;
	 * 1) wait for all dirty locked buffers;
	 * 2) write out all dirty, unlocked buffers;
	 * 2) wait for completion by waiting for all buffers to unlock.
	 */
/*	write_unlocked_buffers(dev);
	if (wait) {
		err = wait_for_locked_buffers(dev, BUF_DIRTY, 0);
		write_unlocked_buffers(dev);
		err |= wait_for_locked_buffers(dev, BUF_LOCKED, 1);
	}*/
	return err;
} 

int fsync_super(struct super_block *sb)
{
	kdev_t dev = sb->s_dev;
	sync_buffers(dev, 0);

	lock_kernel();
	sync_inodes_sb(sb);
	DQUOT_SYNC_SB(sb);
	lock_super(sb);
	if (sb->s_dirt && sb->s_op && sb->s_op->write_super)
		sb->s_op->write_super(sb);
	unlock_super(sb);
	if (sb->s_op && sb->s_op->sync_fs)
		sb->s_op->sync_fs(sb);
	unlock_kernel();

	return sync_buffers(dev, 1);
}

/*
 * For moronic filesystems that do not allow holes in file.
 * We may have to extend the file.
 */

int cont_prepare_write(struct page *page, unsigned offset, unsigned to, get_block_t *get_block, unsigned long *bytes)
{
	struct address_space *mapping = page->mapping;
	struct inode *inode = mapping->host;
	struct page *new_page;
	unsigned long pgpos;
	long status;
	unsigned zerofrom;
	unsigned blocksize = 1 << inode->i_blkbits;
	char *kaddr;

	while(page->index > (pgpos = *bytes>>PAGE_CACHE_SHIFT)) {
		status = -ENOMEM;
		new_page = grab_cache_page(mapping, pgpos);
		if (!new_page)
			goto out;
		/* we might sleep */
		if (*bytes>>PAGE_CACHE_SHIFT != pgpos) {
			UnlockPage(new_page);
			page_cache_release(new_page);
			continue;
		}
		zerofrom = *bytes & ~PAGE_CACHE_MASK;
		if (zerofrom & (blocksize-1)) {
			*bytes |= (blocksize-1);
			(*bytes)++;
		}
		status = __block_prepare_write(inode, new_page, zerofrom,
						PAGE_CACHE_SIZE, get_block);
		if (status)
			goto out_unmap;
		kaddr = page_address(new_page);
		memset(kaddr+zerofrom, 0, PAGE_CACHE_SIZE-zerofrom);
		flush_dcache_page(new_page);
		__block_commit_write(inode, new_page, zerofrom, PAGE_CACHE_SIZE);
		kunmap(new_page);
		UnlockPage(new_page);
		page_cache_release(new_page);
	}

	if (page->index < pgpos) {
		/* completely inside the area */
		zerofrom = offset;
	} else {
		/* page covers the boundary, find the boundary offset */
		zerofrom = *bytes & ~PAGE_CACHE_MASK;

		/* if we will expand the thing last block will be filled */
		if (to > zerofrom && (zerofrom & (blocksize-1))) {
			*bytes |= (blocksize-1);
			(*bytes)++;
		}

		/* starting below the boundary? Nothing to zero out */
		if (offset <= zerofrom)
			zerofrom = offset;
	}
	status = __block_prepare_write(inode, page, zerofrom, to, get_block);
	if (status)
		goto out1;
	kaddr = page_address(page);
	if (zerofrom < offset) {
		memset(kaddr+zerofrom, 0, offset-zerofrom);
		flush_dcache_page(page);
		__block_commit_write(inode, page, zerofrom, offset);
	}
	return 0;
out1:
	ClearPageUptodate(page);
	kunmap(page);
	return status;

out_unmap:
	ClearPageUptodate(new_page);
	kunmap(new_page);
	UnlockPage(new_page);
	page_cache_release(new_page);
out:
	return status;
}

int inode_has_buffers(struct inode *inode)
{
	int ret;

	ret = !list_empty(&inode->i_dirty_buffers) || !list_empty(&inode->i_dirty_data_buffers);
	return ret;
}

static void __remove_inode_queue(struct buffer_head *bh)
{
	list_del(&bh->b_inode_buffers);
	clear_buffer_attached(bh);
}

static inline void remove_inode_queue(struct buffer_head *bh)
{
	if (buffer_attached(bh))
		__remove_inode_queue(bh);
}

static inline void write_buffer(struct buffer_head *bh)
{
	if (buffer_delay(bh)) {
		struct page *page = bh->b_page;

		//lock_page(page);
		if (buffer_delay(bh)) {
			page->mapping->a_ops->writepage(page);
			return;
		}
		//unlock_page(page);
	}

	mark_buffer_dirty(bh);
}

/*
 * Invalidate any and all dirty buffers on a given inode.  We are
 * probably unmounting the fs, but that doesn't mean we have already
 * done a sync().  Just drop the buffers from the inode list.
 */
void invalidate_inode_buffers(struct inode *inode)
{
	struct list_head * entry;

	while ((entry = inode->i_dirty_buffers.next) != &inode->i_dirty_buffers)
		remove_inode_queue(BH_ENTRY(entry));
	while ((entry = inode->i_dirty_data_buffers.next) != &inode->i_dirty_data_buffers)
		remove_inode_queue(BH_ENTRY(entry));
}



// we free pages _without_ freeind buffer heads attached
// code from here on is trying to solve that --sadyc


/*
 * Called when truncating a buffer on a page completely.
 */
static void discard_buffer(struct buffer_head * bh)
{
	if (buffer_mapped(bh) || buffer_delay(bh)) {
		mark_buffer_clean(bh);
		lock_buffer(bh);
		clear_bit(BH_Uptodate, &bh->b_state);
		clear_bit(BH_Mapped, &bh->b_state);
		clear_bit(BH_Req, &bh->b_state);
		clear_bit(BH_New, &bh->b_state);
		clear_bit(BH_Delay, &bh->b_state);
		//remove_from_queues(bh);
		unlock_buffer(bh);
	}
}


/*
 * We don't have to release all buffers here, but
 * we have to be sure that no dirty buffer is left
 * and no IO is going on (no buffer is locked), because
 * we have truncated the file and are going to free the
 * blocks on-disk..
 */
int discard_bh_page(struct page *page, unsigned long offset, int drop_pagecache)
{
	struct buffer_head *head, *bh, *next;
	unsigned int curr_off = 0;

//	printk("buffer.c: discard_bh_page\n");
	if (!PageLocked(page))
		BUG();
	if (!page->buffers) {
		printk("buffer.c: discard_bh_page: no buffers attached\n");
		return 1;
	}

	head = page->buffers;
	bh = head;
	do {
		unsigned int next_off = curr_off + bh->b_size;
		next = bh->b_this_page;

		/*
		 * is this block fully flushed?
		 */
		if (offset <= curr_off)
			discard_buffer(bh);
		curr_off = next_off;
		bh = next;
	} while (bh != head);

	/*
	 * subtle. We release buffer-heads only if this is
	 * the 'final' flushpage. We have invalidated the get_block
	 * cached value unconditionally, so real IO is not
	 * possible anymore.
	 *
	 * If the free doesn't work out, the buffers can be
	 * left around - they just turn into anonymous buffers
	 * instead.
	 */
	if (!offset) {
		if (!try_to_release_page(page, 0))
			return 0;
	}

	return 1;
}


/**
 * try_to_release_page - release old fs-specific metadata on a page
 *
 */

int try_to_release_page(struct page * page, int gfp_mask)
{
//	printk("buffer.c: try_to_release_page\n");
	if (!PageLocked(page))
		BUG();
	
	if (!page->mapping)
		goto try_to_free;
	if (!page->mapping->a_ops->releasepage)
		goto try_to_free;
	if (page->mapping->a_ops->releasepage(page, gfp_mask))
		goto try_to_free;
	/*
	 * We couldn't release buffer metadata; don't even bother trying
	 * to release buffers.
	 */
	return 0;
try_to_free:	
	return try_to_free_buffers(page, gfp_mask);
}

/*
 * just free the buffer heads, no checks! --sadyc
 */
int try_to_free_buffers(struct page * page, unsigned int gfp_mask)
{
	struct buffer_head * tmp, * to_free, * bh = page->buffers;

//	printk("buffer.c: try_to_free_buffers\n");
	tmp = bh;
	to_free = NULL;
	do {
		to_free = tmp;
		tmp = tmp->b_this_page;
		if (to_free) {
//			printk("***** buffer.c: try_to_free_buffers: freeing 1 buffer\n");
			kmem_cache_free(bh_cachep, to_free);
		}
	} while (tmp != bh);

	return 0;
}
