/*
 * Copyright 2009-2011, Ingo Weinhold, ingo_weinhold@gmx.de.
 * Distributed under the terms of the MIT License.
 */
#ifndef VM_KERNEL_ADDRESS_SPACE_H
#define VM_KERNEL_ADDRESS_SPACE_H


#include <vm/VMAddressSpace.h>

#include <VMKernelArea.h>


struct ObjectCache;


struct VMKernelAddressSpace : VMAddressSpace {
public:
								VMKernelAddressSpace(team_id id, addr_t base,
									size_t size);
	virtual						~VMKernelAddressSpace();

	virtual	status_t			InitObject();

	virtual	VMArea*				FirstArea() const;
	virtual	VMArea*				NextArea(VMArea* area) const;

	virtual	VMArea*				LookupArea(addr_t address) const;
	virtual	VMArea*				CreateArea(const char* name, uint32_t wiring,
									uint32_t protection, uint32_t allocationFlags);
	virtual	void				DeleteArea(VMArea* area,
									uint32_t allocationFlags);
	virtual	status_t			InsertArea(VMArea* area, size_t size,
									const virtual_address_restrictions*
										addressRestrictions,
									uint32_t allocationFlags, void** _address);
	virtual	void				RemoveArea(VMArea* area,
									uint32_t allocationFlags);

	virtual	bool				CanResizeArea(VMArea* area, size_t newSize);
	virtual	status_t			ResizeArea(VMArea* area, size_t newSize,
									uint32_t allocationFlags);
	virtual	status_t			ShrinkAreaHead(VMArea* area, size_t newSize,
									uint32_t allocationFlags);
	virtual	status_t			ShrinkAreaTail(VMArea* area, size_t newSize,
									uint32_t allocationFlags);

	virtual	status_t			ReserveAddressRange(size_t size,
									const virtual_address_restrictions*
										addressRestrictions,
									uint32_t flags, uint32_t allocationFlags,
									void** _address);
	virtual	status_t			UnreserveAddressRange(addr_t address,
									size_t size, uint32_t allocationFlags);
	virtual	void				UnreserveAllAddressRanges(
									uint32_t allocationFlags);

	virtual	void				Dump() const;

private:
			typedef VMKernelAddressRange Range;
			typedef VMKernelAddressRangeTree RangeTree;
			typedef DoublyLinkedList<Range,
				DoublyLinkedListMemberGetLink<Range, &Range::listLink> >
					RangeList;
			typedef DoublyLinkedList<Range, VMKernelAddressRangeGetFreeListLink>
				RangeFreeList;

private:
	inline	void				_FreeListInsertRange(Range* range, size_t size);
	inline	void				_FreeListRemoveRange(Range* range, size_t size);

			void				_InsertRange(Range* range);
			void				_RemoveRange(Range* range);

			status_t			_AllocateRange(
									const virtual_address_restrictions*
										addressRestrictions,
									size_t size, bool allowReservedRange,
									uint32_t allocationFlags, Range*& _range);
			Range*				_FindFreeRange(addr_t start, size_t size,
									size_t alignment, uint32_t addressSpec,
									bool allowReservedRange,
									addr_t& _foundAddress);
			void				_FreeRange(Range* range,
									uint32_t allocationFlags);

			void				_CheckStructures() const;

private:
			RangeTree			fRangeTree;
			RangeList			fRangeList;
			RangeFreeList*		fFreeLists;
			int					fFreeListCount;
			ObjectCache*		fAreaObjectCache;
			ObjectCache*		fRangesObjectCache;
};


#endif	/* VM_KERNEL_ADDRESS_SPACE_H */
