/*
 * Copyright 2010, Ingo Weinhold, ingo_weinhold@gmx.de.
 * Distributed under the terms of the MIT License.
 */
#ifndef KERNEL_ARCH_X86_PAGING_PAE_X86_VM_TRANSLATION_MAP_PAE_H
#define KERNEL_ARCH_X86_PAGING_PAE_X86_VM_TRANSLATION_MAP_PAE_H


#include <paging/X86VMTranslationMap.h>


#if B_HAIKU_PHYSICAL_BITS == 64


struct X86PagingStructuresPAE;


struct X86VMTranslationMapPAE : X86VMTranslationMap {
								X86VMTranslationMapPAE();
	virtual						~X86VMTranslationMapPAE();

			status_t			Init(bool kernel);

	virtual	size_t				MaxPagesNeededToMap(addr_t start,
									addr_t end) const;

	virtual	status_t			Map(addr_t virtualAddress,
									phys_addr_t physicalAddress,
									uint32_t attributes, uint32_t memoryType,
									vm_page_reservation* reservation);
	virtual	status_t			Unmap(addr_t start, addr_t end);

	virtual	status_t			DebugMarkRangePresent(addr_t start, addr_t end,
									bool markPresent);

	virtual	status_t			UnmapPage(VMArea* area, addr_t address,
									bool updatePageQueue);
	virtual	void				UnmapPages(VMArea* area, addr_t base,
									size_t size, bool updatePageQueue);
	virtual	void				UnmapArea(VMArea* area,
									bool deletingAddressSpace,
									bool ignoreTopCachePageFlags);

	virtual	status_t			Query(addr_t virtualAddress,
									phys_addr_t* _physicalAddress,
									uint32_t* _flags);
	virtual	status_t			QueryInterrupt(addr_t virtualAddress,
									phys_addr_t* _physicalAddress,
									uint32_t* _flags);

	virtual	status_t			Protect(addr_t base, addr_t top,
									uint32_t attributes, uint32_t memoryType);

	virtual	status_t			ClearFlags(addr_t virtualAddress,
									uint32_t flags);

	virtual	bool				ClearAccessedAndModified(
									VMArea* area, addr_t address,
									bool unmapIfUnaccessed,
									bool& _modified);

	virtual	void				DebugPrintMappingInfo(addr_t virtualAddress);
	virtual	bool				DebugGetReverseMappingInfo(
									phys_addr_t physicalAddress,
									ReverseMappingInfoCallback& callback);

	virtual	X86PagingStructures* PagingStructures() const;
	inline	X86PagingStructuresPAE* PagingStructuresPAE() const
									{ return fPagingStructures; }

private:
			X86PagingStructuresPAE* fPagingStructures;
};


#endif	// B_HAIKU_PHYSICAL_BITS == 64


#endif	// KERNEL_ARCH_X86_PAGING_PAE_X86_VM_TRANSLATION_MAP_PAE_H
