#ifndef _LINUX_MM_H
#define _LINUX_MM_H

#include <linux/sched.h>
#include <linux/errno.h>

#ifdef __KERNEL__

//#include <linux/config.h>
#include <linux/string.h>
#include <linux/list.h>

#include <asm/page.h>
//#include <asm/pgtable.h>
#include <asm/atomic.h>


// vom trata paginile in felul urmator:
// alocam la sfarsitul structurii inca PAGE_SIZE octeti cand alocam o structura
// de tip pagina - acolo vor veni toate buffer-headurile
// deci -> page_address(page) = page + sizeof(page)
#define page_address(_page) ((char*)_page + sizeof(struct page))

typedef struct page {
	struct list_head list;          /* ->mapping has some page lists. */
	struct address_space *mapping;  /* The inode (or ...) we belong to. */
	unsigned long index;            /* Our offset within mapping. */
	struct page *next_hash;         /* Next page sharing our hash bucket in
                                       the pagecache hash table. */
	atomic_t count;                 /* Usage count, see below. */
	unsigned long flags;            /* atomic flags, some possibly
                                       updated asynchronously */
	struct list_head lru;           /* Pageout list, eg. active_list;
                                       protected by pagemap_lru_lock !! */
	struct page **pprev_hash;       /* Complement to *next_hash. */
	struct buffer_head * buffers;   /* Buffer maps us to a disk block. */
} mem_map_t;

#define get_page(p) atomic_inc(&(p)->count)

#define PG_locked		 0	/* Page is locked. Don't touch. */
#define PG_error		 1
#define PG_referenced		 2
#define PG_uptodate		 3
#define PG_dirty		 4
#define PG_unused		 5
#define PG_lru			 6
#define PG_active		 7
#define PG_slab			 8
#define PG_skip			10
#define PG_highmem		11
#define PG_checked		12	/* kill me in 2.5.<early>. */
#define PG_arch_1		13
#define PG_reserved		14
#define PG_launder		15	/* written out by VM pressure.. */
#define PG_fs_1			16	/* Filesystem specific */

#ifndef arch_set_page_uptodate
#define arch_set_page_uptodate(page)
#endif

/* Make it prettier to test the above... */
#define UnlockPage(page)        unlock_page(page)
#define Page_Uptodate(page)     test_bit(PG_uptodate, &(page)->flags)
#define SetPageUptodate(page) \
	do {								\
		arch_set_page_uptodate(page);				\
		set_bit(PG_uptodate, &(page)->flags);			\
	} while (0)
#define ClearPageUptodate(page) clear_bit(PG_uptodate, &(page)->flags)
#define PageDirty(page)         test_bit(PG_dirty, &(page)->flags)
#define SetPageDirty(page)      set_bit(PG_dirty, &(page)->flags)
#define ClearPageDirty(page)    clear_bit(PG_dirty, &(page)->flags)
#define PageLocked(page)        test_bit(PG_locked, &(page)->flags)
#define LockPage(page)          set_bit(PG_locked, &(page)->flags)
#define TryLockPage(page)       test_and_set_bit(PG_locked, &(page)->flags)
#define PageChecked(page)       test_bit(PG_checked, &(page)->flags)
#define SetPageChecked(page)    set_bit(PG_checked, &(page)->flags)
#define ClearPageChecked(page)  clear_bit(PG_checked, &(page)->flags)
#define PageLaunder(page)       test_bit(PG_launder, &(page)->flags)
#define SetPageLaunder(page)    set_bit(PG_launder, &(page)->flags)
#define ClearPageLaunder(page)  clear_bit(PG_launder, &(page)->flags)
#define ClearPageArch1(page)    clear_bit(PG_arch_1, &(page)->flags)

#define PageError(page)		test_bit(PG_error, &(page)->flags)
#define SetPageError(page)	set_bit(PG_error, &(page)->flags)
#define ClearPageError(page)	clear_bit(PG_error, &(page)->flags)
#define PageReferenced(page)    test_bit(PG_referenced, &(page)->flags)
#define SetPageReferenced(page) set_bit(PG_referenced, &(page)->flags)
#define ClearPageReferenced(page)       clear_bit(PG_referenced, &(page)->flags)

#define PageActive(page)        test_bit(PG_active, &(page)->flags)
#define SetPageActive(page)     set_bit(PG_active, &(page)->flags)
#define ClearPageActive(page)   clear_bit(PG_active, &(page)->flags)

extern unsigned long __get_free_pages(unsigned int gfp_mask, unsigned int order);
#define __get_free_page(gfp_mask) \
		__get_free_pages((gfp_mask),0)

extern void __free_pages(struct page *page, unsigned int order);
extern void free_pages(unsigned long addr, unsigned int order);

#define __free_page(page) __free_pages((page), 0)
#define free_page(addr) free_pages((addr),0)

extern void truncate_inode_pages(struct address_space *, loff_t);

#define __GFP_HIGHMEM   0x02

#define __GFP_WAIT	0x10	/* Can wait and reschedule? */
#define __GFP_HIGH	0x20	/* Should access emergency pools? */
#define __GFP_IO	0x40	/* Can start low memory physical IO? */
#define __GFP_HIGHIO	0x80	/* Can start high mem physical IO? */
#define __GFP_FS	0x100	/* Can call down to low-level FS? */

#define GFP_ATOMIC	(__GFP_HIGH)
#define GFP_USER	(             __GFP_WAIT | __GFP_IO | __GFP_HIGHIO | __GFP_FS)
#define GFP_HIGHUSER    (             __GFP_WAIT | __GFP_IO | __GFP_HIGHIO | __GFP_FS | __GFP_HIGHMEM)
#define GFP_KERNEL	(__GFP_HIGH | __GFP_WAIT | __GFP_IO | __GFP_HIGHIO | __GFP_FS)

#endif /* __KERNEL__ */

#endif