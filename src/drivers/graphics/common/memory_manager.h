/*
 * Copyright 2006, Haiku Inc.
 * Copyright 2002, Thomas Kurschel.
 *
 * Distributed under the terms of the MIT license.
 */
#ifndef _MEMORY_MANAGER_H
#define _MEMORY_MANAGER_H

/**	Memory manager used for graphics mem */

#include <kits/kernel/OS.h>


typedef struct mem_info mem_info;

#ifdef __cplusplus
extern "C" {
#endif

mem_info *mem_init(const char *name, uint32_t start, uint32_t length, uint32_t blockSize,
				uint32_t heapEntries);
void mem_destroy(mem_info *mem);
status_t mem_alloc(mem_info *mem, uint32_t size, void *tag, uint32_t *blockID, uint32_t *offset);
status_t mem_free(mem_info *mem, uint32_t blockID, void *tag);
status_t mem_freetag(mem_info *mem, void *tag);

#ifdef __cplusplus
}
#endif

#endif	/* _MEMORY_MANAGER_H */
