#ifndef _LINUX_HIGHMEM_H
#define _LINUX_HIGHMEM_H

#include <linux/config.h>
//#include <asm/pgalloc.h>


#include <linux/mm.h>


//static inline unsigned int nr_free_highpages(void) { return 0; }

static inline void *kmap(struct page *page) { return page_address(page); }

#define kunmap(page) do { } while (0)

#define kmap_atomic(page,idx)		kmap(page)
#define kunmap_atomic(page,idx)		kunmap(page)

#define bh_kmap(bh)			((bh)->b_data)
#define bh_kunmap(bh)			do { } while (0)
#define kmap_nonblock(page)            kmap(page)
#define bh_kmap_irq(bh, flags)		((bh)->b_data)
#define bh_kunmap_irq(bh, flags)	do { *(flags) = 0; } while (0)

#if 0
/* when CONFIG_HIGHMEM is not set these will be plain clear/copy_page */
static inline void clear_user_highpage(struct page *page, unsigned long vaddr)
{
	void *addr = kmap_atomic(page, KM_USER0);
	clear_user_page(addr, vaddr);
	kunmap_atomic(addr, KM_USER0);
}

static inline void clear_highpage(struct page *page)
{
	clear_page(kmap(page));
	kunmap(page);
}

/*
 * Same but also flushes aliased cache contents to RAM.
 */
static inline void memclear_highpage_flush(struct page *page, unsigned int offset, unsigned int size)
{
	char *kaddr;

	if (offset + size > PAGE_SIZE)
		out_of_line_bug();
	kaddr = kmap(page);
	memset(kaddr + offset, 0, size);
	flush_dcache_page(page);
	flush_page_to_ram(page);
	kunmap(page);
}

static inline void copy_user_highpage(struct page *to, struct page *from, unsigned long vaddr)
{
	char *vfrom, *vto;

	vfrom = kmap_atomic(from, KM_USER0);
	vto = kmap_atomic(to, KM_USER1);
	copy_user_page(vto, vfrom, vaddr);
	kunmap_atomic(vfrom, KM_USER0);
	kunmap_atomic(vto, KM_USER1);
}

static inline void copy_highpage(struct page *to, struct page *from)
{
	char *vfrom, *vto;

	vfrom = kmap_atomic(from, KM_USER0);
	vto = kmap_atomic(to, KM_USER1);
	copy_page(vto, vfrom);
	kunmap_atomic(vfrom, KM_USER0);
	kunmap_atomic(vto, KM_USER1);
}
#endif

#endif /* _LINUX_HIGHMEM_H */
