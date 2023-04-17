/*
 * Copyright 2009, Ingo Weinhold, ingo_weinhold@gmx.de.
 * Distributed under the terms of the MIT License.
 */
#ifndef VM_USER_ADDRESS_SPACE_H
#define VM_USER_ADDRESS_SPACE_H


#include <vm/VMAddressSpace.h>

#include <VMUserArea.h>


struct VMUserAddressSpace : VMAddressSpace {
public:
								VMUserAddressSpace(team_id id, addr_t base,
									size_t size);
	virtual						~VMUserAddressSpace();

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
	inline	bool				_IsRandomized(uint32_t addressSpec) const;
	static	addr_t				_RandomizeAddress(addr_t start, addr_t end,
									size_t alignment, bool initial = false);

			status_t			_InsertAreaIntoReservedRegion(addr_t start,
									size_t size, VMUserArea* area,
									uint32_t allocationFlags);
			status_t			_InsertAreaSlot(addr_t start, addr_t size,
									addr_t end, uint32_t addressSpec,
									size_t alignment, VMUserArea* area,
									uint32_t allocationFlags);

private:
	static	const addr_t		kMaxRandomize;
	static	const addr_t		kMaxInitialRandomize;

			VMUserAreaList		fAreas;
	mutable	VMUserArea*			fAreaHint;
};


#endif	/* VM_USER_ADDRESS_SPACE_H */
