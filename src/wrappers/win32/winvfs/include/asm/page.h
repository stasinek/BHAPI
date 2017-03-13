#pragma once

//
//	/include/asm-i386/page.h
//

//#define PAGE_SHIFT 12
//#define PAGE_SIZE (1UL << PAGE_SHIFT)
#define PAGE_MASK (~(PAGE_SIZE-1))
#define __PAGE_OFFSET		(0xC0000000)
#define PAGE_OFFSET		((unsigned long)__PAGE_OFFSET)
