#ifndef __LINUX_TODO_H
#define __LINUX_TODO_H

//#define __KERNEL__
#define inline
#define __init
#define __inline__ __inline
#define __asm__ __asm
#define __volatile__ volatile
#define asmlinkage
#define barrier()

// cache.h + arch/i386/config.in
#define CONFIG_X86_L1_CACHE_SHIFT 5 
#define L1_CACHE_SHIFT	(CONFIG_X86_L1_CACHE_SHIFT)
#define L1_CACHE_BYTES	(1 << L1_CACHE_SHIFT)

// from cache.c , needed by dcache.c
#define __cacheline_aligned_in_smp

// from pgtable.h, needed by dcache.c
#define kern_addr_valid(addr)	(1)
#define flush_dcache_page(page)                 do { } while (0)

#endif
