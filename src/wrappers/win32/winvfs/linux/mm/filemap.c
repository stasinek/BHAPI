/*
 *  Portions of this file are taken from linux kernel 2.4.25
 *	linux/mm/filemap.c
 *
 * Copyright (C) 1994-1999  Linus Torvalds
 */

/*
 * Copyright (C) 2004 Dan Mihailescu
 */

#include "todo.h"
#include <linux/pagemap.h>
#include "debug.h"

static __inline struct page *__read_cache_page(struct address_space *mapping, unsigned long index,
											   int (*filler)(void *,struct page*), void *data)
{
	struct page **hash = NULL;//page_hash(mapping, index);
	struct page *page, *cached_page = NULL;
	int err;

//	printk("__read_cache_page: called: inode blkbits= %d\n", mapping->host->i_blkbits);

	page = NULL;//__find_get_page(mapping, index, hash);
	if (!page) {
		if (!cached_page) {
			cached_page = page_cache_alloc(mapping);
			if (!cached_page)
				return ERR_PTR(-ENOMEM);
		}
		page = cached_page;
		//if (add_to_page_cache_unique(page, mapping, index, hash))
		//	goto repeat;
		// --danny-- undeva in functia pe care am sarit-o se apeleaza alta functie care apeleaza alta functie
		// care apeleaza add_page_to_inode_queue care face urmatoarele : (nu mai fac functia, scriu direct aici,
		// nu mai e nevoie de ea in alta parte, deocamdata :)
		// begin
		ClearPageUptodate(page);
		ClearPageError(page);
		ClearPageDirty(page);
		ClearPageReferenced(page);
		ClearPageArch1(page);
		ClearPageChecked(page);
		//LockPage(page);
		//page_cache_get(page);
		page->index = index;

//		printk("___ se mapeaza inodul la pagina___\n");
		// astea doua sunt din add_page_to_inode_queue
		mapping->nrpages++;
		page->mapping = mapping;

		// ar trebui sa mai pun pagina in lista de clean_pages, dar nu ne intereseaza
		// end
		cached_page = NULL;
//		printk("__read_cache_page: about to call filler i_blkbits= %d\n", page->mapping->host->i_blkbits);
		err = filler(data, page);
		if (err < 0) {
//			printk("___ fillerul a dat eroare___\n");
			page_cache_release(page);
			page = ERR_PTR(err);
		}
	}
	if (cached_page)
		page_cache_release(cached_page);
	return page;
}

void mark_page_accessed(struct page *page)
{
	if (!PageActive(page) && PageReferenced(page)) {
		//activate_page(page);
		ClearPageReferenced(page);
	} else
		SetPageReferenced(page);
}

struct page *read_cache_page(struct address_space *mapping, unsigned long index, 
							 int (*filler)(void *,struct page*), void *data)
{
	struct page *page;
	int err;

retry:
//	printk("read_cache_page: before __read_cache_page\n");
	page = __read_cache_page(mapping, index, filler, data);
	if (IS_ERR(page))
		goto out;
	mark_page_accessed(page);
	if (Page_Uptodate(page))
		goto out;

	lock_page(page);
	if (!page->mapping) {
		UnlockPage(page);
		page_cache_release(page);
		goto retry;
	}
	if (Page_Uptodate(page)) {
		UnlockPage(page);
		goto out;
	}
	err = filler(data, page);
	if (err < 0) {
		page_cache_release(page);
		page = ERR_PTR(err);
	}
out:
	return page;
}

static int do_flushpage(struct page *page, unsigned long offset)
{
	int (*flushpage) (struct page *, unsigned long);
	flushpage = page->mapping->a_ops->flushpage;
	if (flushpage)
		return (*flushpage)(page, offset);
	return block_flushpage(page, offset);
}

static inline void truncate_partial_page(struct page *page, unsigned partial)
{
//	memclear_highpage_flush(page, partial, PAGE_CACHE_SIZE-partial);
	if (page->buffers)
		do_flushpage(page, partial);
}

static void truncate_complete_page(struct page *page)
{
	/* Leave it on the LRU if it gets converted into anonymous buffers */
//	if (!page->buffers || do_flushpage(page, 0))
//		lru_cache_del(page);

	/*
	 * We remove the page from the page cache _after_ we have
	 * destroyed all buffer-cache references to it. Otherwise some
	 * other process might think this inode page is not in the
	 * page cache and creates a buffer-cache alias to it causing
	 * all sorts of fun problems ...  
	 */
	ClearPageDirty(page);
	ClearPageUptodate(page);
//	remove_inode_page(page);
	page_cache_release(page);
}

static int truncate_list_pages(struct list_head *head, unsigned long start, unsigned *partial)
{
	struct list_head *curr;
	struct page * page;
	int unlocked = 0;

 restart:
	curr = head->prev;
	while (curr != head) {
		unsigned long offset;

		page = list_entry(curr, struct page, list);
		offset = page->index;

		/* Is one of the pages to truncate? */
		if ((offset >= start) || (*partial && (offset + 1) == start)) {
			int failed;

			page_cache_get(page);
			failed = TryLockPage(page);

			list_del(head);
			if (!failed)
				/* Restart after this page */
				list_add_tail(head, curr);
			else
				/* Restart on this page */
				list_add(head, curr);

			spin_unlock(&pagecache_lock);
			unlocked = 1;

 			if (!failed) {
				if (*partial && (offset + 1) == start) {
					truncate_partial_page(page, *partial);
					*partial = 0;
				} else 
					truncate_complete_page(page);

				UnlockPage(page);
			} else
 				wait_on_page(page);

			page_cache_release(page);

			spin_lock(&pagecache_lock);
			goto restart;
		}
		curr = curr->prev;
	}
	return unlocked;
}



void truncate_inode_pages(struct address_space * mapping, loff_t lstart) 
{
	unsigned long start = ((unsigned long) lstart + PAGE_CACHE_SIZE - 1) >> PAGE_CACHE_SHIFT;
	unsigned partial = (unsigned long) lstart & (PAGE_CACHE_SIZE - 1);
	int unlocked;

	spin_lock(&pagecache_lock);
	do {
		unlocked = truncate_list_pages(&mapping->clean_pages, start, &partial);
		unlocked |= truncate_list_pages(&mapping->dirty_pages, start, &partial);
		unlocked |= truncate_list_pages(&mapping->locked_pages, start, &partial);
	} while (unlocked);
	/* Traversed all three lists without dropping the lock */
	spin_unlock(&pagecache_lock);
}

struct page * find_or_create_page(struct address_space *mapping, unsigned long index, unsigned int gfp_mask)
{
	struct page *page;
	//struct page **hash = page_hash(mapping, index);

	//spin_lock(&pagecache_lock);
	page = NULL;//__find_lock_page_helper(mapping, index, *hash);
	//spin_unlock(&pagecache_lock);
	if (!page) {
		struct page *newpage = page_cache_alloc(mapping);//alloc_page(gfp_mask);
		if (newpage) {
			//spin_lock(&pagecache_lock);
			//page = __find_lock_page_helper(mapping, index, *hash);
			//if (likely(!page)) {
				page = newpage;
			//	__add_to_page_cache(page, mapping, index, hash);
			//	newpage = NULL;
			//}
			//spin_unlock(&pagecache_lock);
			//if (newpage == NULL)
			//	lru_cache_add(page);
			//else 
			//	page_cache_release(newpage);
		}
	}
	return page;
}

int file_read_actor(read_descriptor_t * desc, struct page *page, unsigned long offset, unsigned long size)
{
	unsigned long left, count = desc->count;

	if (size > count)
		size = count;

//	printk("file_read_actor: offset=%d, size=%d\n", offset, size);
//	dump_buffer((char*)page + sizeof(struct page), offset+size, "file_read_actor");

//	memcpy(desc->buf, (char*)page + offset, size);  // fixed  --sadyc
	memcpy(desc->buf, (char*)page + sizeof(struct page) + offset, size);
	desc->count = count - size;
	desc->written += size;
	desc->buf += size;
	return size;
}

/*
 * This is a generic file read routine, and uses the
 * inode->i_op->readpage() function for the actual low-level
 * stuff.
 *
 * This is really ugly. But the goto's actually try to clarify some
 * of the logic when it comes to error handling etc.
 */
void do_generic_file_read(struct file * filp, loff_t *ppos, read_descriptor_t * desc, read_actor_t actor)
{
	struct address_space *mapping = filp->f_dentry->d_inode->i_mapping;
	struct inode *inode = mapping->host;
	unsigned long index, offset;
	int error;

	index = (unsigned long)(*ppos >> PAGE_CACHE_SHIFT);
	offset = (unsigned long)(*ppos & ~PAGE_CACHE_MASK);
	for (;;) {
		struct page *page;
		unsigned long end_index, nr, ret;

		end_index = (unsigned long)(inode->i_size >> PAGE_CACHE_SHIFT);
		if (index > end_index)
			break;
		nr = PAGE_CACHE_SIZE;
		if (index == end_index) {
			nr = (unsigned long)(inode->i_size & ~PAGE_CACHE_MASK);
			if (nr <= offset)
				break;
		}
		nr = nr - offset;
		page = page_cache_alloc(mapping);
		if (page == NULL) {
			printk("do_generic_file_read: NOMEM at page_cache_alloc\n");
			desc->error = -ENOMEM;
			break;
		}
		page->mapping = mapping;
		page->index = index;
		error = mapping->a_ops->readpage(filp, page);
		if (error) {
			desc->error = error;
			page_cache_release(page);
			break;
		}
		ret = actor(desc, page, offset, nr);
		offset += ret;
		index += offset >> PAGE_CACHE_SHIFT;
		offset &= ~PAGE_CACHE_MASK;
		page_cache_release(page);
		if (ret == nr && desc->count)
			continue;
		break;
	}
	*ppos = ((loff_t) index << PAGE_CACHE_SHIFT) + offset;
	//if (page)
	//	kfree(page);
	UPDATE_ATIME(inode);
}

/*
 * This is the "read()" routine for all filesystems
 * that can use the page cache directly.
 */
ssize_t generic_file_read(struct file * filp, char * buf, size_t count, loff_t *ppos)
{
	ssize_t retval;
	
	if ((ssize_t) count < 0)
		return -EINVAL;

	retval = 0;
	if (count) {
		read_descriptor_t desc;
		desc.written = 0;
		desc.count = count;
		desc.buf = buf;
		desc.error = 0;
		do_generic_file_read(filp, ppos, &desc, file_read_actor);
		retval = desc.written;
		if (!retval)
			retval = desc.error;
	}
	return retval;
}

/*
 * Write to a file through the page cache. 
 *
 * We currently put everything into the page cache prior to writing it.
 * This is not a problem when writing full pages. With partial pages,
 * however, we first have to read the data into the cache, then
 * dirty the page, and finally schedule it for writing. Alternatively, we
 * could write-through just the portion of data that would go into that
 * page, but that would kill performance for applications that write data
 * line by line, and it's prone to race conditions.
 *
 * Note that this routine doesn't try to keep track of dirty pages. Each
 * file system has to do this all by itself, unfortunately.
 *                                                      okir@monad.swb.de
 */
ssize_t generic_file_write(struct file *file,const char *buf,size_t count, loff_t *ppos)
{
	struct address_space *mapping = file->f_dentry->d_inode->i_mapping;
	struct inode    *inode = mapping->host;

	//dimensiunea maxima a unui fisier in cadrul procesului curent
	//unsigned long   limit = current->rlim[RLIMIT_FSIZE].rlim_cur;

	loff_t          pos;
	struct page     *page, *cached_page;
	ssize_t         written;
	long            status = 0;
	int             err;
	unsigned        bytes;

	down(&inode->i_sem);
	pos = *ppos;

	written = 0;

	if (!S_ISBLK(inode->i_mode)) {
		if (pos >= (long)(inode->i_sb->s_maxbytes))
		{
			if (count || pos > (long)(inode->i_sb->s_maxbytes)) {
				err = -EFBIG;
				goto out;
			}
		}
		if (pos + count > (long)(inode->i_sb->s_maxbytes))
			count = (size_t)(inode->i_sb->s_maxbytes - pos);
        }
        inode->i_ctime = inode->i_mtime = CURRENT_TIME;
        mark_inode_dirty(inode);
        do {
			unsigned long index, offset;
			offset = (unsigned long)(pos & (PAGE_CACHE_SIZE -1)); /* Within page */
			index = (unsigned long)(pos >> PAGE_CACHE_SHIFT);
			bytes = PAGE_CACHE_SIZE - offset;
			if (bytes > count)
				bytes = count;
			page = page_cache_alloc(mapping);
			if (!page)
				break;
			status = mapping->a_ops->prepare_write(file, page, offset, offset+bytes);
			status = mapping->a_ops->commit_write(file, page, offset, offset+bytes);
			if (!status)
				status = bytes;
			if (status >= 0) {
				written += status;
				count -= status;
				pos += status;
				buf += status;
			}
			if (status < 0)
				break;
		} while (count);
		*ppos = pos;
        if (page != NULL)
			page_cache_release(page);
        if (status >= 0) {
			mark_inode_dirty(inode);
        }
        err = written ? written : status;
out:
        up(&inode->i_sem);
        return err;
}

int generic_file_mmap(struct file * file, struct vm_area_struct * vma)
{
	//struct address_space *mapping = file->f_dentry->d_inode->i_mapping;
	//struct inode *inode = mapping->host;

	//if ((vma->vm_flags & VM_SHARED) && (vma->vm_flags & VM_MAYWRITE)) {
	//	if (!mapping->a_ops->writepage)
	//	return -EINVAL;
	//}
	//if (!mapping->a_ops->readpage)
	//	return -ENOEXEC;
	//UPDATE_ATIME(inode);
	//vma->vm_ops = &generic_file_vm_ops;
	return 0;
}

 /**
  *      filemap_fdatawait - walk the list of locked pages of the given address space
  *      and wait for all of them.
  * 
  *      @mapping: address space structure to wait for
  *
  */
int filemap_fdatawait(struct address_space * mapping)
{
	int ret = 0;

	//spin_lock(&pagecache_lock);

	while (!list_empty(&mapping->locked_pages)) {
		struct page *page = list_entry(mapping->locked_pages.next, struct page, list);

		list_del(&page->list);
		list_add(&page->list, &mapping->clean_pages);

		//if (!PageLocked(page))
	//		continue;

		page_cache_get(page);
		//spin_unlock(&pagecache_lock);

		//___wait_on_page(page);
		//if (PageError(page))
		//	ret = -EIO;

		page_cache_release(page);
		//spin_lock(&pagecache_lock);
	}
	//spin_unlock(&pagecache_lock);
	return ret;
} 

int filemap_fdatasync(struct address_space * mapping)
{
	int ret = 0;
	int (*writepage)(struct page *) = mapping->a_ops->writepage;

	//spin_lock(&pagecache_lock);

	while (!list_empty(&mapping->dirty_pages)) {
			struct page *page = list_entry(mapping->dirty_pages.prev, struct page, list);

			list_del(&page->list);
			list_add(&page->list, &mapping->locked_pages);

			if (!PageDirty(page))
			continue;

			page_cache_get(page);
			//spin_unlock(&pagecache_lock);

			//lock_page(page);

			if (PageDirty(page)) {
				int err;
				ClearPageDirty(page);
				err = writepage(page);
				if (err && !ret)
					ret = err;
			} else
				//UnlockPage(page);

			page_cache_release(page);
			//spin_lock(&pagecache_lock);
	}

	//spin_unlock(&pagecache_lock);
	return ret;
}