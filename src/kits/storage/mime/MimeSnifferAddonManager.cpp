/*
 * Copyright 2006-2013, Ingo Weinhold, ingo_weinhold@gmx.de.
 * Distributed under the terms of the MIT License.
 */


#include <mime/MimeSnifferAddonManager.h>

#include <new>

#include <Autolock.h>
#include <MimeType.h>

#include <MimeSnifferAddon.h>


namespace BPrivate {
namespace Storage {
namespace Mime {


// singleton instance
MimeSnifferAddonManager* MimeSnifferAddonManager::sManager = NULL;


// AddonReference
struct MimeSnifferAddonManager::AddonReference {
	AddonReference(BMimeSnifferAddon* addon)
		: fAddon(addon),
		  fReferenceCount(1)
	{
	}

	~AddonReference()
	{
		delete fAddon;
	}

	BMimeSnifferAddon* Addon() const
	{
		return fAddon;
	}

	void GetReference()
	{
		atomic_add(&fReferenceCount, 1);
	}

	void PutReference()
	{
		if (atomic_add(&fReferenceCount, -1) == 1)
			delete this;
	}

private:
	BMimeSnifferAddon*	fAddon;
	int32_t				fReferenceCount;
};


// constructor
MimeSnifferAddonManager::MimeSnifferAddonManager()
	: fLock("mime sniffer manager"),
	  fAddons(20),
	  fMinimalBufferSize(0)
{
}

// destructor
MimeSnifferAddonManager::~MimeSnifferAddonManager()
{
}

// Default
MimeSnifferAddonManager*
MimeSnifferAddonManager::Default()
{
	return sManager;
}

// CreateDefault
status_t MimeSnifferAddonManager::CreateDefault()
{
	MimeSnifferAddonManager* manager
		= new(std::nothrow) MimeSnifferAddonManager;
	if (!manager)
		return B_NO_MEMORY;

	sManager = manager;

	return B_OK;
}

// DeleteDefault
void MimeSnifferAddonManager::DeleteDefault()
{
	MimeSnifferAddonManager* manager = sManager;
	sManager = NULL;

	delete manager;
}

// AddMimeSnifferAddon
status_t MimeSnifferAddonManager::AddMimeSnifferAddon(BMimeSnifferAddon* addon)
{
	if (!addon)
		return B_BAD_VALUE;

	BAutolock locker(fLock);
	if (!locker.IsLocked())
		return B_ERROR;

	// create a reference for the addon
	AddonReference* reference = new(std::nothrow) AddonReference(addon);
	if (!reference)
		return B_NO_MEMORY;

	// add the reference
	if (!fAddons.AddItem(reference)) {
		delete reference;
		return B_NO_MEMORY;
	}

	// update minimal buffer size
	size_t minBufferSize = addon->MinimalBufferSize();
	if (minBufferSize > fMinimalBufferSize)
		fMinimalBufferSize = minBufferSize;

	return B_OK;
}

// MinimalBufferSize
size_t
MimeSnifferAddonManager::MinimalBufferSize()
{
	return fMinimalBufferSize;
}

// GuessMimeType
float
MimeSnifferAddonManager::GuessMimeType(const char* fileName, BMimeType* type)
{
	// get addons
	AddonReference** addons = NULL;
	int32_t count = 0;
	status_t error = _GetAddons(addons, count);
	if (error != B_OK)
		return -1;

	// iterate over the addons and find the most fitting type
	float bestPriority = -1;
	for (int32_t i = 0; i < count; i++) {
		BMimeType currentType;
		float priority = addons[i]->Addon()->GuessMimeType(fileName,
			&currentType);
		if (priority > bestPriority) {
			type->SetTo(currentType.Type());
			bestPriority = priority;
		}
	}

	// release addons
	_PutAddons(addons, count);

	return bestPriority;
}

// GuessMimeType
float
MimeSnifferAddonManager::GuessMimeType(BFile* file, const void* buffer,
	int32_t length, BMimeType* type)
{
	// get addons
	AddonReference** addons = NULL;
	int32_t count = 0;
	status_t error = _GetAddons(addons, count);
	if (error != B_OK)
		return -1;

	// iterate over the addons and find the most fitting type
	float bestPriority = -1;
	for (int32_t i = 0; i < count; i++) {
		BMimeType currentType;
		float priority = addons[i]->Addon()->GuessMimeType(file, buffer,
			length, &currentType);
		if (priority > bestPriority) {
			type->SetTo(currentType.Type());
			bestPriority = priority;
		}
	}

	// release addons
	_PutAddons(addons, count);

	return bestPriority;
}

// _GetAddons
status_t MimeSnifferAddonManager::_GetAddons(AddonReference**& references, int32_t& count)
{
	BAutolock locker(fLock);
	if (!locker.IsLocked())
		return B_ERROR;

	count = fAddons.CountItems();
	references = new(std::nothrow) AddonReference*[count];
	if (!references)
		return B_NO_MEMORY;

	for (int32_t i = 0; i < count; i++) {
		references[i] = (AddonReference*)fAddons.ItemAt(i);
		references[i]->GetReference();
	}

	return B_OK;
}

// _PutAddons
void MimeSnifferAddonManager::_PutAddons(AddonReference** references, int32_t count)
{
	for (int32_t i = 0; i < count; i++)
		references[i]->PutReference();

	delete[] references;
}


}	// namespace Mime
}	// namespace Storage
}	// namespace BPrivate
