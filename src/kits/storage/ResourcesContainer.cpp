//----------------------------------------------------------------------
//  This software is part of the OpenBeOS distribution and is covered 
//  by the OpenBeOS license.
//---------------------------------------------------------------------
/*!
	\file ResourcesContainer.cpp
	ResourcesContainer implementation.
*/



#include <ResourcesContainer.h>

#include <ResourceItem.h>

namespace BPrivate {
namespace Storage {

// constructor
ResourcesContainer::ResourcesContainer()
				  : fResources(),
					fIsModified(false)
{
}

// destructor
ResourcesContainer::~ResourcesContainer()
{
	MakeEmpty();
}

// AddResource
//
// Returns false, if item is NULL or memory is insufficient, true otherwise.
bool ResourcesContainer::AddResource(ResourceItem *item, int32_t index,
								bool replace)
{
	bool result = false;
	if (item) {
		// replace an item with the same type and id
		if (replace)
			delete RemoveResource(IndexOf(item->Type(), item->ID()));
		int32_t count = CountResources();
		if (index < 0 || index > count)
			index = count;
		result = fResources.AddItem(item, count);
		SetModified(true);
	}
	return result;
}

// RemoveResource
ResourceItem*
ResourcesContainer::RemoveResource(int32_t index)
{
	ResourceItem* item = (ResourceItem*)fResources.RemoveItem(index);
	if (item)
		SetModified(true);
	return item;
}

// RemoveResource
bool ResourcesContainer::RemoveResource(ResourceItem *item)
{
	return RemoveResource(IndexOf(item));
}

// MakeEmpty
void ResourcesContainer::MakeEmpty()
{
	for (int32_t i = 0; ResourceItem *item = ResourceAt(i); i++)
		delete item;
	fResources.MakeEmpty();
	SetModified(false);
}

// AssimilateResources
void ResourcesContainer::AssimilateResources(ResourcesContainer &container)
{
	// Resistance is futile! ;-)
	int32_t newCount = container.CountResources();
	for (int32_t i = 0; i < newCount; i++) {
		ResourceItem *item = container.ResourceAt(i);
		if (item->IsLoaded())
			AddResource(item);
		else {
			// That should not happen.
			// Delete the item to have a consistent behavior.
			delete item;
		}
	}
	container.fResources.MakeEmpty();
	container.SetModified(true);
	SetModified(true);
}

// IndexOf
int32_t ResourcesContainer::IndexOf(ResourceItem *item) const
{
	return fResources.IndexOf(item);
}

// IndexOf
int32_t ResourcesContainer::IndexOf(const void *data) const
{
	int32_t index = -1;
	if (data) {
		int32_t count = CountResources();
		for (int32_t i = 0; index == -1 && i < count; i++) {
			if (ResourceAt(i)->Data() == data)
				index = i;
		}
	}
	return index;
}

// IndexOf
int32_t ResourcesContainer::IndexOf(type_code type, int32_t id) const
{
	int32_t index = -1;
	int32_t count = CountResources();
	for (int32_t i = 0; index == -1 && i < count; i++) {
		ResourceItem *item = ResourceAt(i);
		if (item->Type() == type && item->ID() == id)
			index = i;
	}
	return index;
}

// IndexOf
int32_t ResourcesContainer::IndexOf(type_code type, const char *name) const
{
	int32_t index = -1;
	int32_t count = CountResources();
	for (int32_t i = 0; index == -1 && i < count; i++) {
		ResourceItem *item = ResourceAt(i);
		const char *itemName = item->Name();
		if (item->Type() == type && ((name == NULL && itemName == NULL)
									 || (name != NULL && itemName != NULL
										&& strcmp(name, itemName) == 0))) {
			index = i;
		}
	}
	return index;
}

// IndexOfType
int32_t ResourcesContainer::IndexOfType(type_code type, int32_t typeIndex) const
{
	int32_t index = -1;
	int32_t count = CountResources();
	for (int32_t i = 0; index == -1 && i < count; i++) {
		ResourceItem *item = ResourceAt(i);
		if (item->Type() == type) {
			if (typeIndex == 0)
				index = i;
			typeIndex--;
		}
	}
	return index;
}

// ResourceAt
ResourceItem*
ResourcesContainer::ResourceAt(int32_t index) const
{
	return (ResourceItem*)fResources.ItemAt(index);
}

// CountResources
int32_t ResourcesContainer::CountResources() const
{
	return fResources.CountItems();
}

// SetModified
void ResourcesContainer::SetModified(bool modified)
{
	fIsModified = modified;
	// If unmodified, set the resource item's modified flag as well.
	if (!modified) {
		int32_t count = CountResources();
		for (int32_t i = 0; i < count; i++)
			ResourceAt(i)->SetModified(false);
	}
}

// IsModified
bool ResourcesContainer::IsModified() const
{
	bool isModified = fIsModified;
	int32_t count = CountResources();
	for (int32_t i = 0; !isModified && i < count; i++)
		isModified |= ResourceAt(i)->IsModified();
	return isModified;
}


};	// namespace Storage
};	// namespace BPrivate




