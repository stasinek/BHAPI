/*
 * rewritten
 * copyright (c) 2004 Adrian Stanciu
 * 
 * portions from linux/slab.h
 */

#if	!defined(_LINUX_SLAB_H)
#define	_LINUX_SLAB_H

#if	defined(__KERNEL__)

#include <ntifs.h>

#include <config.h>

#define CACHE_NAMELEN 16
#define	BYTES_PER_WORD		sizeof(void *)

/* flags passed to a constructor func */
#define	SLAB_CTOR_CONSTRUCTOR	0x001UL		/* if not set, then deconstructor */
#define SLAB_CTOR_ATOMIC	0x002UL		/* tell constructor it can't sleep */
#define	SLAB_CTOR_VERIFY	0x004UL		/* tell constructor it's a verify call */

typedef struct kmem_cache_s kmem_cache_t;

struct kmem_cache_s {
	unsigned int		objsize;
	char				name[CACHE_NAMELEN];
#ifdef USE_NONPAGED_MEMORY
	NPAGED_LOOKASIDE_LIST  lookaside;
#else
	PAGED_LOOKASIDE_LIST  lookaside;
#endif
	/* constructor func */
	void (*ctor)(void *, kmem_cache_t *, unsigned long);

	/* de-constructor func */
	void (*dtor)(void *, kmem_cache_t *, unsigned long);
#if DBG
	int objects_allocated;  // number of objects allocated, for debug purposes
#endif
};

#include	<linux/mm.h>
//#include	<linux/cache.h>


#define	SLAB_HWCACHE_ALIGN	0x00002000UL	/* align objs on a h/w cache lines */
/* prototypes */
extern kmem_cache_t *kmem_cache_create(const char *, size_t, size_t, unsigned long,
				       void (*)(void *, kmem_cache_t *, unsigned long),
				       void (*)(void *, kmem_cache_t *, unsigned long));
extern int kmem_cache_destroy(kmem_cache_t *);
extern int kmem_cache_shrink(kmem_cache_t *);
extern void *kmem_cache_alloc(kmem_cache_t *, int);
extern void kmem_cache_free(kmem_cache_t *, void *);
//extern unsigned int kmem_cache_size(kmem_cache_t *);

#define	SLAB_KERNEL		GFP_KERNEL

/* System wide caches */
//extern kmem_cache_t	*vm_area_cachep;
//extern kmem_cache_t	*mm_cachep;
extern kmem_cache_t	*names_cachep;
//extern kmem_cache_t	*files_cachep;
extern kmem_cache_t	*filp_cachep;
extern kmem_cache_t	*dquot_cachep;
extern kmem_cache_t	*bh_cachep;
//extern kmem_cache_t	*fs_cachep;
//extern kmem_cache_t	*sigact_cachep;

#define sadyc_tag(n) (((unsigned)((n)[3]) << 24) + ((unsigned)((n)[2]) << 16) + ((unsigned)((n)[1]) << 8) + (unsigned)((n)[0]))
#ifdef USE_NONPAGED_MEMORY
#define VFSPOOL NonPagedPool
#else
#define VFSPOOL PagedPool
#endif
#define kmalloc(size, gfp) ExAllocatePoolWithTag(VFSPOOL, size, sadyc_tag(__FUNCTION__))

#define kfree(p) ExFreePool(p)

#endif	/* __KERNEL__ */

#endif	/* _LINUX_SLAB_H */