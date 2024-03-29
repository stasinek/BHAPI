/*
 * Copyright 2010, Ithamar R. Adema, ithamar.adema@team-embedded.nl
 * Copyright 2010, Ingo Weinhold, ingo_weinhold@gmx.de.
 * Distributed under the terms of the MIT License.
 */
#ifndef KERNEL_ARCH_ARM_PAGING_32_BIT_ARM_PAGING_METHOD_32_BIT_H
#define KERNEL_ARCH_ARM_PAGING_32_BIT_ARM_PAGING_METHOD_32_BIT_H


#include <paging/32bit/paging.h>
#include <paging/ARMPagingMethod.h>
#include <paging/ARMPagingStructures.h>


class TranslationMapPhysicalPageMapper;
class ARMPhysicalPageMapper;


class ARMPagingMethod32Bit : public ARMPagingMethod {
public:
								ARMPagingMethod32Bit();
	virtual						~ARMPagingMethod32Bit();

	virtual	status_t			Init(kernel_args* args,
									VMPhysicalPageMapper** _physicalPageMapper);
	virtual	status_t			InitPostArea(kernel_args* args);

	virtual	status_t			CreateTranslationMap(bool kernel,
									VMTranslationMap** _map);

	virtual	status_t			MapEarly(kernel_args* args,
									addr_t virtualAddress,
									phys_addr_t physicalAddress,
									uint8 attributes,
									page_num_t (*get_free_page)(kernel_args*));

	virtual	bool				IsKernelPageAccessible(addr_t virtualAddress,
									uint32_t protection);

	inline	uint32_t				KernelPhysicalPageDirectory() const
									{ return fKernelPhysicalPageDirectory; }
	inline	page_directory_entry* KernelVirtualPageDirectory() const
									{ return fKernelVirtualPageDirectory; }
	inline	ARMPhysicalPageMapper* PhysicalPageMapper() const
									{ return fPhysicalPageMapper; }
	inline	TranslationMapPhysicalPageMapper* KernelPhysicalPageMapper() const
									{ return fKernelPhysicalPageMapper; }

	static	ARMPagingMethod32Bit* Method();

	static	void				PutPageTableInPageDir(
									page_directory_entry* entry,
									phys_addr_t pgtablePhysical,
									uint32_t attributes);
	static	void				PutPageTableEntryInTable(
									page_table_entry* entry,
									phys_addr_t physicalAddress,
									uint32_t attributes, uint32_t memoryType,
									bool globalPage);
	static	page_table_entry	SetPageTableEntry(page_table_entry* entry,
									page_table_entry newEntry);
	static	page_table_entry	SetPageTableEntryFlags(page_table_entry* entry,
									uint32_t flags);
	static	page_table_entry	TestAndSetPageTableEntry(
									page_table_entry* entry,
									page_table_entry newEntry,
									page_table_entry oldEntry);
	static	page_table_entry	ClearPageTableEntry(page_table_entry* entry);
	static	page_table_entry	ClearPageTableEntryFlags(
									page_table_entry* entry, uint32_t flags);

	static	uint32_t				MemoryTypeToPageTableEntryFlags(
									uint32_t memoryType);

private:
			struct PhysicalPageSlotPool;
			friend struct PhysicalPageSlotPool;

private:
	inline	int32_t				_GetInitialPoolCount();

	static	void				_EarlyPreparePageTables(
									page_table_entry* pageTables,
									addr_t address, size_t size);
	static	status_t			_EarlyQuery(addr_t virtualAddress,
									phys_addr_t *_physicalAddress);

private:
			uint32_t				fKernelPhysicalPageDirectory;
			page_directory_entry* fKernelVirtualPageDirectory;

			ARMPhysicalPageMapper* fPhysicalPageMapper;
			TranslationMapPhysicalPageMapper* fKernelPhysicalPageMapper;
};


/*static*/ inline ARMPagingMethod32Bit*
ARMPagingMethod32Bit::Method()
{
	return static_cast<ARMPagingMethod32Bit*>(gARMPagingMethod);
}


/*static*/ inline page_table_entry
ARMPagingMethod32Bit::SetPageTableEntry(page_table_entry* entry,
	page_table_entry newEntry)
{
	return atomic_get_and_set((int32_t*)entry, newEntry);
}


/*static*/ inline page_table_entry
ARMPagingMethod32Bit::SetPageTableEntryFlags(page_table_entry* entry,
	uint32_t flags)
{
	return atomic_or((int32_t*)entry, flags);
}


/*static*/ inline page_table_entry
ARMPagingMethod32Bit::TestAndSetPageTableEntry(page_table_entry* entry,
	page_table_entry newEntry, page_table_entry oldEntry)
{
	return atomic_test_and_set((int32_t*)entry, newEntry, oldEntry);
}


/*static*/ inline page_table_entry
ARMPagingMethod32Bit::ClearPageTableEntry(page_table_entry* entry)
{
	return SetPageTableEntry(entry, 0);
}


/*static*/ inline page_table_entry
ARMPagingMethod32Bit::ClearPageTableEntryFlags(page_table_entry* entry, uint32_t flags)
{
	return atomic_and((int32_t*)entry, ~flags);
}


/*static*/ inline uint32_t
ARMPagingMethod32Bit::MemoryTypeToPageTableEntryFlags(uint32_t memoryType)
{
#if 0 //IRA
	// ATM we only handle the uncacheable and write-through type explicitly. For
	// all other types we rely on the MTRRs to be set up correctly. Since we set
	// the default memory type to write-back and since the uncacheable type in
	// the PTE overrides any MTRR attribute (though, as per the specs, that is
	// not recommended for performance reasons), this reduces the work we
	// actually *have* to do with the MTRRs to setting the remaining types
	// (usually only write-combining for the frame buffer).
	switch (memoryType) {
		case B_MTR_UC:
			return X86_PTE_CACHING_DISABLED | X86_PTE_WRITE_THROUGH;

		case B_MTR_WC:
			// ARM_PTE_WRITE_THROUGH would be closer, but the combination with
			// MTRR WC is "implementation defined" for Pentium Pro/II.
			return 0;

		case B_MTR_WT:
			return X86_PTE_WRITE_THROUGH;

		case B_MTR_WP:
		case B_MTR_WB:
		default:
			return 0;
	}
#else
	return 0;
#endif
}


#endif	// KERNEL_ARCH_ARM_PAGING_32_BIT_ARM_PAGING_METHOD_32_BIT_H
