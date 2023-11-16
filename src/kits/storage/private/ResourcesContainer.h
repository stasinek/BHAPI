//----------------------------------------------------------------------
//  This software is part of the OpenBeOS distribution and is covered 
//  by the OpenBeOS license.
//---------------------------------------------------------------------
/*!
	\file ResourcesContainer.h
	ResourcesContainer interface declaration.
*/

#ifndef _RESOURCES_CONTAINER_H
#define _RESOURCES_CONTAINER_H

#include <kits/support/List.h>

namespace BPrivate {
namespace Storage {

class ResourceItem;

/*!
	\class ResourcesContainer
	\brief Represents a collection of resources.
	
	This class can be used to manage a collection of resources represented
	by ResourceItem's. Usually it does never contain two resources with the
	same type \em and ID, since AddResource() replaces an old item with the
	new one, unless explicitly told not to do so. This should only be done
	by ResourceFile, when type and ID of the items are not yet known.
	Asside from the basic vector features like Add/RemoveResource()/
	ResourceAt() and MakeEmpty() a bunch of IndexOf() methods are provided
	and AssimilateResources() which incorporates all resources of another
	container into this one (replacing old resources, if necessary), emptying
	the other one.

	The ResourceItem's in a container are deleted on destruction, unless
	removed before via RemoveResource(). MakeEmpty() deletes the items as
	well.

	\author <a href='mailto:bonefish@users.sf.net'>Ingo Weinhold</a>
	
	\version 0.0.0
*/
class ResourcesContainer {
public:
	ResourcesContainer();
	virtual ~ResourcesContainer();

	bool AddResource(ResourceItem *item, int32_t index = -1,
					 bool replace = true);

	ResourceItem *RemoveResource(int32_t index);
	bool RemoveResource(ResourceItem *item);

	void MakeEmpty();

	void AssimilateResources(ResourcesContainer &container);

	int32_t IndexOf(ResourceItem *item) const;
	int32_t IndexOf(const void *data) const;
	int32_t IndexOf(type_code type, int32_t id) const;
	int32_t IndexOf(type_code type, const char *name) const;
	int32_t IndexOfType(type_code type, int32_t index) const;
	ResourceItem *ResourceAt(int32_t index) const;
	int32_t CountResources() const;

	void SetModified(bool modified);
	bool IsModified() const;

private:
	BList	fResources;
	bool	fIsModified;
};

};	// namespace Storage
};	// namespace BPrivate

#endif	// _RESOURCES_CONTAINER_H


