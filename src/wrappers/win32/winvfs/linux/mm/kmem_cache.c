/*
 *  linux/mm/kmem_cache.c
 *  Linux kmem_cache emulation
 *
 *  Copyright (C) 2004 Adrian Stanciu (adrian@sadyc.net)
 */

#include "todo.h"
//#include "linux/fs.h"
#include "linux/slab.h"


kmem_cache_t *
kmem_cache_create (const char *name, size_t size, size_t offset,
	unsigned long flags, void (*ctor)(void*, kmem_cache_t *, unsigned long),
	void (*dtor)(void*, kmem_cache_t *, unsigned long))
{
	kmem_cache_t *cachep = NULL;
	ULONG tag = sadyc_tag(name);

	if ((!name) || ((strlen(name) >= CACHE_NAMELEN - 1)) ||
		(size < BYTES_PER_WORD) ||
		(offset > size))
			BUG();

	printk("kmem_cache: %s: created size=%u tag=%u\n", name, (unsigned)size, tag);

	cachep = kmalloc(sizeof(kmem_cache_t), GFP_KERNEL);
	memset(cachep, 0, sizeof(kmem_cache_t));

#ifdef USE_NONPAGED_MEMORY
	ExInitializeNPagedLookasideList(&cachep->lookaside, NULL, NULL, 0, size, tag, 0);
#else
	ExInitializePagedLookasideList(&cachep->lookaside, NULL, NULL, 0, size, tag, 0);
#endif

	cachep->objsize = size;
	strcpy(cachep->name, name);
	cachep->ctor = ctor;
	cachep->dtor = dtor;
#if DBG
	cachep->objects_allocated = 0;
#endif
	return cachep;
}

int kmem_cache_shrink(kmem_cache_t *cachep)
{
	return 0; // no way to shrink, yet --sadyc
}

void kmem_cache_free (kmem_cache_t *cachep, void *objp)
{
	if (cachep->dtor != NULL) cachep->dtor(objp, cachep, 0); // 0 might be wrong --sadyc

#ifdef USE_NONPAGED_MEMORY
	ExFreeToNPagedLookasideList(&cachep->lookaside, objp);
#else
	ExFreeToPagedLookasideList(&cachep->lookaside, objp);
#endif
#if DBG
	cachep->objects_allocated--;
	printk("kmem_cache: %s: objects: %d [-]\n", cachep->name, cachep->objects_allocated);
#endif
}

void * kmem_cache_alloc (kmem_cache_t *cachep, int flags)
{
	void *tmp;
#ifdef USE_NONPAGED_MEMORY
	tmp = ExAllocateFromNPagedLookasideList(&cachep->lookaside);
#else
	tmp = ExAllocateFromPagedLookasideList(&cachep->lookaside);
#endif

	if (tmp == NULL) {
		printk("kmem_cache: %s: ALLOCATION FAILED\n", cachep->name);
		return NULL;
	}

#if DBG
	cachep->objects_allocated++;
	printk("kmem_cache: %s: objects: %d [+]\n", cachep->name, cachep->objects_allocated);
#endif
	if (tmp && cachep->ctor != NULL) cachep->ctor(tmp, cachep, flags);
	return tmp;
}

int kmem_cache_destroy (kmem_cache_t * cachep)
{
	printk("kmem_cache: %s: destroy (objects not freed: %d)\n", cachep->name, cachep->objects_allocated);
#ifdef USE_NONPAGED_MEMORY
	ExDeleteNPagedLookasideList(&cachep->lookaside);
#else
	ExDeletePagedLookasideList(&cachep->lookaside);
#endif
	kfree(cachep);
	cachep=NULL;
	return 0;
}
