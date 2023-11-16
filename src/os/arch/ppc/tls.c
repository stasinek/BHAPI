/* 
** Copyright 2003, Axel Dörfler, axeld@pinc-software.de. All rights reserved.
** Distributed under the terms of the OpenBeOS License.
*/

// ToDo: this is a dummy implementation - I've not yet gained enough knowledge
//	to decide how this should be done, so it's just broken now (okay for single
//	threaded apps, though).

// we don't want to have the inline assembly included here
#ifndef _NO_INLINE_ASM
#	define _NO_INLINE_ASM 1
#endif

#include <kits/support/TLS.h>
#include <tls.h>


static int32_t gNextSlot = TLS_FIRST_FREE_SLOT;
static void *gSlots[TLS_MAX_KEYS];


int32_t tls_allocate(void)
{
	int32_t next = atomic_add(&gNextSlot, 1);
	if (next >= TLS_MAX_KEYS)
		return B_NO_MEMORY;

	return next;
}


void *
tls_get(int32_t index)
{
	return gSlots[index];
}


void **
tls_address(int32_t index)
{
	return &gSlots[index];
}


void
tls_set(int32_t index, void *value)
{
	gSlots[index] = value;
}

