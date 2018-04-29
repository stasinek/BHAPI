#ifndef _LINUX_PAGEMAP_H
#define _LINUX_PAGEMAP_H

/*
 * Page-mapping primitive inline functions
 *
 * Copyright 1995 Linus Torvalds
 */

#include <linux/mm.h>
#include <linux/fs.h>
#include <linux/list.h>

//#include <asm/system.h>
//#include <asm/pgtable.h>
#include <linux/highmem.h>

#include <linux/slab.h> // for page_ache_alloc
 
#define PAGE_CACHE_SHIFT  PAGE_SHIFT
#define PAGE_CACHE_SIZE   PAGE_SIZE
#define PAGE_CACHE_MASK   PAGE_MASK
#define PAGE_CACHE_ALIGN(addr) (((addr)+PAGE_CACHE_SIZE-1)&PAGE_CACHE_MASK)

#define page_cache_get(x)       get_page(x)
#define page_cache_release(x)   __free_page(x)

static inline struct page *page_cache_alloc(struct address_space *x)
{
	struct page* page;

	page = kmalloc(sizeof(struct page) + PAGE_SIZE, x->gfp_mask);
	page->buffers = NULL;

	return page;
}

extern struct page * find_or_create_page(struct address_space *mapping,
				unsigned long index, unsigned int gfp_mask);

#define unlock_page(page)			test_and_clear_bit(PG_locked, &(page)->flags)
#define lock_page(page)				TryLockPage(page)

static inline void wait_on_page(struct page * page)
{
//	if (PageLocked(page))
//		___wait_on_page(page);
}

/*
 * Returns locked page at given index in given cache, creating it if needed.
 */
static inline struct page *grab_cache_page(struct address_space *mapping, unsigned long index)
{
	return find_or_create_page(mapping, index, mapping->gfp_mask);
}

typedef int filler_t(void *, struct page*);

extern struct page *read_cache_page(struct address_space *, unsigned long,
				filler_t *, void *);
#endif