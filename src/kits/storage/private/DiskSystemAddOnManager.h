/*
 * Copyright 2007, Ingo Weinhold, bonefish@users.sf.net.
 * Distributed under the terms of the MIT License.
 */
#ifndef _DISK_SYSTEM_ADD_ON_MANAGER_H
#define _DISK_SYSTEM_ADD_ON_MANAGER_H

#include <FindDirectory.h>
#include <kits/support/List.h>
#include <Locker.h>


class BDiskSystemAddOn;


namespace BPrivate {


class DiskSystemAddOnManager {
public:
	static	DiskSystemAddOnManager* Default();

			bool				Lock();
			void				Unlock();

			// load/unload all disk system add-ons
			status_t			LoadDiskSystems();
			void				UnloadDiskSystems();

			// manager must be locked
			int32_t				CountAddOns() const;
			BDiskSystemAddOn*	AddOnAt(int32_t index) const;

			// manager will be locked
			BDiskSystemAddOn*	GetAddOn(const char* name);
			void				PutAddOn(BDiskSystemAddOn* addOn);

private:
			struct AddOnImage;
			struct AddOn;
			struct StringSet;

								DiskSystemAddOnManager();

	static	void				_InitSingleton();

			AddOn*				_AddOnAt(int32_t index) const;
			void				_PutAddOn(int32_t index);

			status_t			_LoadAddOns(StringSet& alreadyLoaded,
									directory_which directory);

private:
			mutable BLocker		fLock;
			BList				fAddOns;
			BList				fAddOnsToBeUnloaded;
			int32_t				fLoadCount;

	static	DiskSystemAddOnManager* sManager;
};


}	// namespace BPrivate

using BPrivate::DiskSystemAddOnManager;

#endif	// _DISK_SYSTEM_ADD_ON_MANAGER_H
