/*
 * Copyright 2012, Rene Gollent, rene@gollent.com.
 * Distributed under the terms of the MIT License.
 */
#ifndef WATCHPOINT_H
#define WATCHPOINT_H


#include <ObjectList.h>
#include <Referenceable.h>

#include <types/Types.h>


class Watchpoint : public BReferenceable {
public:
								Watchpoint(target_addr_t address, uint32_t type,
									int32_t length);
								~Watchpoint();

			target_addr_t		Address() const		{ return fAddress; }
			uint32_t				Type() const		{ return fType; }
			int32_t				Length() const	{ return fLength; }

			bool				IsInstalled() const	{ return fInstalled; }
			void				SetInstalled(bool installed);

			bool				IsEnabled() const	{ return fEnabled; }
			void				SetEnabled(bool enabled);
									// WatchpointManager only

			bool				ShouldBeInstalled() const
									{ return fEnabled && !fInstalled; }

			bool				Contains(target_addr_t address) const;

	static	int					CompareWatchpoints(const Watchpoint* a,
									const Watchpoint* b);
	static	int					CompareAddressWatchpoint(
									const target_addr_t* address,
									const Watchpoint* watchpoint);

private:
			target_addr_t		fAddress;
			uint32_t				fType;
			int32_t				fLength;

			bool				fInstalled;
			bool				fEnabled;
};


typedef BObjectList<Watchpoint> WatchpointList;


#endif	// WATCHPOINT_H
