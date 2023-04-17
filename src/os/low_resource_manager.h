/*
 * Copyright 2008, Ingo Weinhold, ingo_weinhold@gmx.de.
 * Copyright 2005-2009, Axel Dörfler, axeld@pinc-software.de.
 * Distributed under the terms of the MIT License.
 */
#ifndef _KERNEL_LOW_RESOURCE_MANAGER_H
#define _KERNEL_LOW_RESOURCE_MANAGER_H


#include <Haiku.h>


/* warning levels for low resource handlers */
enum {
	B_NO_LOW_RESOURCE = 0,
	B_LOW_RESOURCE_NOTE,
	B_LOW_RESOURCE_WARNING,
	B_LOW_RESOURCE_CRITICAL,
};

enum {
	B_KERNEL_RESOURCE_PAGES			= 0x01,	/* physical pages */
	B_KERNEL_RESOURCE_MEMORY		= 0x02,	/* reservable memory */
	B_KERNEL_RESOURCE_SEMAPHORES	= 0x04,	/* semaphores */
	B_KERNEL_RESOURCE_ADDRESS_SPACE	= 0x08, /* address space */

	B_ALL_KERNEL_RESOURCES			= B_KERNEL_RESOURCE_PAGES
										| B_KERNEL_RESOURCE_MEMORY
										| B_KERNEL_RESOURCE_SEMAPHORES
										| B_KERNEL_RESOURCE_ADDRESS_SPACE
};

typedef void (*low_resource_func)(void *data, uint32_t resources, int32_t level);

#ifdef __cplusplus
extern "C" {
#endif

status_t low_resource_manager_init(void);
status_t low_resource_manager_init_post_thread(void);
int32_t low_resource_state(uint32_t resources);
void low_resource(uint32_t resource, uint64 requirements, uint32_t flags,
	uint32_t timeout);

// these calls might get public some day
status_t register_low_resource_handler(low_resource_func function, void *data,
			uint32_t resources, int32_t priority);
status_t unregister_low_resource_handler(low_resource_func function,
			void *data);

#ifdef __cplusplus
}
#endif

#endif	/* _KERNEL_LOW_RESOURCE_MANAGER_H */
