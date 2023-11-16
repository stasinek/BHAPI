#ifndef _CONFIG_H
#define _CONFIG_H


// commet this line to disable IRP debugging in debug.c
#define IRP_DEBUG 1

/* comment to use paged memory for kmalloc and kmem_cache --sadyc */
#define USE_NONPAGED_MEMORY

// uncomment to shrink dcache after close, when fcb->count is 0
// basicall, with SHRINK_DCACHE_ON_CLOSE turned on,
// dcache and icache are almost empty all the time
#define SHRINK_DCACHE_ON_CLOSE

#endif