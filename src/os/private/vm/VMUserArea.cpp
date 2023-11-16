/*
 * Copyright 2009-2010, Ingo Weinhold, ingo_weinhold@gmx.de.
 * Distributed under the terms of the NewOS License.
 */


#include <VMUserArea.h>

#include <heap.h>
#include <vm/vm_priv.h>


VMUserArea::VMUserArea(VMAddressSpace* addressSpace, uint32_t wiring,
	uint32_t protection)
	:
	VMArea(addressSpace, wiring, protection)
{
}


VMUserArea::~VMUserArea()
{
}


/*static*/ VMUserArea*
VMUserArea::Create(VMAddressSpace* addressSpace, const char* name,
	uint32_t wiring, uint32_t protection, uint32_t allocationFlags)
{
	VMUserArea* area = new(malloc_flags(allocationFlags)) VMUserArea(
		addressSpace, wiring, protection);
	if (area == NULL)
		return NULL;

	if (area->Init(name, allocationFlags) != B_OK) {
		area->~VMUserArea();
		free_etc(area, allocationFlags);
		return NULL;
	}

	return area;
}


/*static*/ VMUserArea*
VMUserArea::CreateReserved(VMAddressSpace* addressSpace, uint32_t flags,
	uint32_t allocationFlags)
{
	VMUserArea* area = new(malloc_flags(allocationFlags)) VMUserArea(
		addressSpace, 0, 0);
	if (area != NULL) {
		area->id = RESERVED_AREA_ID;
		area->protection = flags;
	}
	return area;
}
