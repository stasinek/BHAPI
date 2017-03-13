#pragma once

#include "linux/mm.h"

//
//	/include/asm-i386/highmem.h
//

#define kmap(page) __kmap(page, 0)

static __inline void *__kmap(struct page *page, int nonblocking)
{
	return page_address(page);
}

static __inline void kunmap(struct page *page)
{
	;
}
