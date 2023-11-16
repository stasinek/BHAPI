/*
 *  Portions of this file are taken from linux kernel 2.4.25
 *  linux/mm/page_alloc.c
 *
 */

#include "todo.h"
#include <linux/slab.h>


void __free_pages(struct page *page, unsigned int order)
{
//	printk("__free_pages called order %u\n", order);
	LockPage(page);
	block_invalidate_page(page);
	if (page != NULL) kfree(page);
	page = NULL;
}

unsigned long __get_free_pages(unsigned int gfp_mask, unsigned int order)
{
	printk("__get_free_pages called mask %x order %u\n", gfp_mask, order);
	return (unsigned long) kmalloc( (1UL << order) << PAGE_SHIFT, gfp_mask);
}

//void free_pages(struct page *page, unsigned int order)
void free_pages(unsigned long addr, unsigned int order)
{
	printk("free_pages called order %u\n", order);
	if (addr != 0) kfree((void *)addr);
}
