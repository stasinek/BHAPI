/* --------------------------------------------------------------------------
 *
 * BHAPI++ previously named ETK++, The Easy Toolkit for C++ programing
 * Copyright (C) 2004-2006, Anthony Lee, All Rights Reserved
 *
 * BHAPI++ library is a freeware; it may be used and distributed according to
 * the terms of The MIT License.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
 * IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * File: etk-area.cpp
 *
 * --------------------------------------------------------------------------*/
#ifdef BHAPI_OS_BEOS

#include <be/kernel/OS.h>

#include "../kernel/Kernel.h"
#include "../support/String.h"

#define BEOS_AREA_INFO_MAGIC		0xABFC

typedef struct b_beos_area_t {
	b_beos_area_t()
		: name(NULL), domain(NULL), ipc_name(NULL), prot(0), length(0), addr(NULL), beArea(-1), openedIPC(false), created(false)
	{
	}

	~b_beos_area_t()
	{
		if(created)
		{
			created = false;
			bhapi::delete_area((void*)this);
		}
	}

	char		*name;
	char		*domain;
	char		*ipc_name;
	b_uint32		prot;
	size_t		length;
	void		*addr;
	area_id		beArea;
	bool		openedIPC;
	bool		created;
} b_beos_area_t;


// return value must be free by "free()"
static char* b_area_ipc_name(const char *name, const char *domain)
{
	if(name == NULL || *name == 0 || strlen(name) > B_OS_NAME_LENGTH || !domain || strlen(domain) != 4) return NULL;
	if(strlen(name) > B_OS_NAME_LENGTH - 4)
	{
		BHAPI_WARNING("\n==================================================================\n[KERNEL]: %s --- Length of area's name exceeds %d.\n==================================================================\n", __PRETTY_FUNCTION__, B_OS_NAME_LENGTH - 4);
		return NULL;
	}
	return bhapi::strdup_printf("%s%s", domain, name);
}

class b_beos_area_locker_t {
public:
	b_beos_area_locker_t()
	{
		const char *lockerName = "__bhapi_G_area_locker__";

		if((iLocker = find_port(lockerName)) < 0)
		{
			if((iLocker = create_port(1, lockerName)) >= 0)
			{
				char buf = 1;
				if(set_port_owner(iLocker, B_SYSTEM_TEAM) != B_OK || write_port(iLocker, 'b_', &buf, 1) != B_OK)
				{
					delete_port(iLocker);
					iLocker = -1;
				}
			}
			if(iLocker >= 0) BHAPI_DEBUG("[KERNEL]: port for global area locker created.");
		}
		else
		{
			port_info portInfo;
			if(get_port_info(iLocker, &portInfo) != B_OK || portInfo.capacity != 1) iLocker = -1;
			if(iLocker >= 0) BHAPI_DEBUG("[KERNEL]: port for global area locker found.");
		}
		if(iLocker < 0) BHAPI_ERROR("[KERNEL]: Can't initialize global area!");
	}

	void Lock()
	{
		while(true)
		{
//			BHAPI_DEBUG("[KERNEL]: try locking global area.");

			int32 msgCode = 0;
			char buf = 0;
			b_size_t readBytes = read_port(iLocker, &msgCode, &buf, 1);
			if(readBytes < 1) continue;
			if(readBytes != 1 || msgCode != 'b_' || buf != 1)
				BHAPI_ERROR("[KERNEL]: Unable to lock the locker for global area.");
//			BHAPI_DEBUG("[KERNEL]: global area locker locked.");
			break;
		}
	}

	void Unlock()
	{
		char buf = 1;
		if(write_port(iLocker, 'b_', &buf, 1) != B_OK) BHAPI_ERROR("[KERNEL]: Unable to unlock the locker for global area.");
//		BHAPI_DEBUG("[KERNEL]: global area locker unlocked.");
	}

	port_id iLocker;
};

static b_beos_area_locker_t __bhapi_area_locker__;

static void _BHAPI_LOCK_AREA_()
{
	__bhapi_area_locker__.Lock();
}

static void _BHAPI_UNLOCK_AREA_()
{
	__bhapi_area_locker__.Unlock();
}


typedef struct b_beos_area_info_t {
	b_int32		magic;
	bool		closed;
	size_t		length;
} b_beos_area_info_t;


IMPEXP_BHAPI void*
bhapi::create_area(const char *name, void **start_addr, size_t size, b_uint32 protection, const char *domain, bhapi::area_access area_access)
{
	if(size <= 0) return NULL;

	char *ipc_name = b_area_ipc_name(name, domain);
	if(!ipc_name) return NULL;

	b_beos_area_t *area = new b_beos_area_t();
	if(!area)
	{
		free(ipc_name);
		return NULL;
	}

	area->prot = protection;
	uint32 prot = B_READ_AREA;
	if(protection & B_WRITE_AREA) prot |= B_WRITE_AREA;

//	if(area_access != BHAPI_AREA_ACCESS_OWNER) BHAPI_DEBUG("[KERNEL]: %s --- Access unsupport.", __PRETTY_FUNCTION__);

	_BHAPI_LOCK_AREA_();
	if(find_area(ipc_name) >= 0)
	{
		_BHAPI_UNLOCK_AREA_();
		BHAPI_WARNING("[KERNEL]: %s --- area already exists.", __PRETTY_FUNCTION__);
		free(ipc_name);
		delete area;
		return NULL;
	}

	uint32 requestSize = (size + sizeof(b_beos_area_info_t) + (B_PAGE_SIZE - 1)) & ~(B_PAGE_SIZE - 1);
	if((area->beArea = create_area(ipc_name, &(area->addr), B_ANY_ADDRESS,
				       requestSize, B_NO_LOCK, prot)) < 0 || area->addr == NULL)
	{
		if(area->beArea >= 0) delete_area(area->beArea);
		_BHAPI_UNLOCK_AREA_();
		BHAPI_DEBUG("[KERNEL]: %s --- Unable to create area \"%s\": error_no: %d",
				  __PRETTY_FUNCTION__, ipc_name, area->beArea);
		free(ipc_name);
		delete area;
		return NULL;
	}

	b_beos_area_info_t _area_info;
	_area_info.magic = BEOS_AREA_INFO_MAGIC;
	_area_info.closed = false;
	_area_info.length = size;
	memcpy(area->addr, &_area_info, sizeof(b_beos_area_info_t));

	area->length = size;
	area->name = bhapi::strdup(name);
	area->domain = bhapi::strdup(domain);
	area->ipc_name = ipc_name;
	area->created = true;

	_BHAPI_UNLOCK_AREA_();

	if(start_addr) *start_addr = (void*)((char*)area->addr + sizeof(b_beos_area_info_t));

//	BHAPI_DEBUG("[KERNEL]: Area created: \"%s\"-\"%s\" --- %lu", name, domain, size);

	return area;
}


IMPEXP_BHAPI void*
bhapi::clone_area(const char *name, void **dest_addr, b_uint32 protection, const char *domain)
{
	char *ipc_name = b_area_ipc_name(name, domain);
	if(!ipc_name) return NULL;

	b_beos_area_t *area = new b_beos_area_t();
	if(!area)
	{
		free(ipc_name);
		return NULL;
	}

	area->prot = protection;
	uint32 prot = B_READ_AREA;
	if(protection & B_WRITE_AREA) prot |= B_WRITE_AREA;

	_BHAPI_LOCK_AREA_();

	if((area->beArea = clone_area(ipc_name, &(area->addr), B_ANY_ADDRESS, prot, find_area(ipc_name))) < 0 || area->addr == NULL)
	{
		if(area->beArea >= 0) delete_area(area->beArea);
		_BHAPI_UNLOCK_AREA_();
//		BHAPI_DEBUG("[KERNEL]: %s --- Unable to clone area \"%s\": error_no: %d", __PRETTY_FUNCTION__, ipc_name, area->beArea);
		free(ipc_name);
		delete area;
		return NULL;
	}

	b_beos_area_info_t _area_info;
	bzero(&_area_info, sizeof(b_beos_area_info_t));
	memcpy(&_area_info, area->addr, sizeof(b_beos_area_info_t));
	if(_area_info.magic != BEOS_AREA_INFO_MAGIC || _area_info.closed)
	{
		delete_area(area->beArea);
		_BHAPI_UNLOCK_AREA_();
		BHAPI_WARNING("[KERNEL]: %s --- area(%s) %s.",
					__PRETTY_FUNCTION__, ipc_name,
					(_area_info.magic != BEOS_AREA_INFO_MAGIC ? "seems not created by BHAPI" : "already deleted and not allowed to clone"));
		free(ipc_name);
		delete area;
		return NULL;
	}

	area->length = _area_info.length;
	area->name = bhapi::strdup(name);
	area->domain = bhapi::strdup(domain);
	area->ipc_name = ipc_name;
	area->openedIPC = true;
	area->created = true;

	_BHAPI_UNLOCK_AREA_();

	if(dest_addr) *dest_addr = (void*)((char*)area->addr + sizeof(b_beos_area_info_t));
	return area;
}


IMPEXP_BHAPI void*
bhapi::clone_area_by_source(void *source_data, void **dest_addr, b_uint32 protection)
{
	b_beos_area_t *source_area = (b_beos_area_t*)source_data;
	if(!source_area) return NULL;

	return bhapi::clone_area(source_area->name, dest_addr, protection, source_area->domain);
}


IMPEXP_BHAPI b_status_t
bhapi::get_area_info(void *data, b_area_info *info)
{
	b_beos_area_t *area = (b_beos_area_t*)data;
	if(!area || !info) return B_BAD_VALUE;
	if(!area->name || *(area->name) == 0 || strlen(area->name) > B_OS_NAME_LENGTH ||
	   !area->domain || strlen(area->domain) != 4 ||
	   area->addr == NULL) return B_ERROR;

	bzero(info->name, B_OS_NAME_LENGTH + 1);
	bzero(info->domain, 5);

	info->size = area->length;
	strcpy(info->name, area->name);
	strcpy(info->domain, area->domain);
	info->protection = area->prot;
	info->address = (void*)((char*)area->addr + sizeof(b_beos_area_info_t));

	return B_OK;
}


IMPEXP_BHAPI b_status_t
bhapi::delete_area(void *data)
{
	b_beos_area_t *area = (b_beos_area_t*)data;
	if(!area) return B_BAD_VALUE;

	if(!area->openedIPC)
	{
		if(!(area->prot & B_WRITE_AREA)) set_area_protection(area->beArea, B_READ_AREA | B_WRITE_AREA);

		_BHAPI_LOCK_AREA_();
		b_beos_area_info_t _area_info;
		bzero(&_area_info, sizeof(b_beos_area_info_t));
		memcpy(&_area_info, area->addr, sizeof(b_beos_area_info_t));
		if(_area_info.magic == BEOS_AREA_INFO_MAGIC && !_area_info.closed)
		{
			_area_info.closed = true;
			memcpy(area->addr, &_area_info, sizeof(b_beos_area_info_t));
		}
		_BHAPI_UNLOCK_AREA_();
	}

	delete_area(area->beArea);

	free(area->ipc_name);

	free(area->name);
	free(area->domain);

	if(area->created)
	{
		area->created = false;
		delete area;
	}

	return B_OK;
}


IMPEXP_BHAPI b_status_t
bhapi::delete_area_etc(void *data, bool no_clone)
{
	b_beos_area_t *area = (b_beos_area_t*)data;
	if(!area) return B_BAD_VALUE;

	if(no_clone && (area->openedIPC ? (area->prot & B_WRITE_AREA) : true))
	{
		if(!(area->prot & B_WRITE_AREA)) set_area_protection(area->beArea, B_READ_AREA | B_WRITE_AREA);

		_BHAPI_LOCK_AREA_();
		b_beos_area_info_t _area_info;
		bzero(&_area_info, sizeof(b_beos_area_info_t));
		memcpy(&_area_info, area->addr, sizeof(b_beos_area_info_t));
		if(_area_info.magic == BEOS_AREA_INFO_MAGIC && !_area_info.closed)
		{
			_area_info.closed = true;
			memcpy(area->addr, &_area_info, sizeof(b_beos_area_info_t));
		}
		_BHAPI_UNLOCK_AREA_();
	}

	delete_area(area->beArea);

	free(area->ipc_name);

	free(area->name);
	free(area->domain);

	if(area->created)
	{
		area->created = false;
		delete area;
	}

	return B_OK;
}


IMPEXP_BHAPI b_status_t
b_resize_area(void *data, void **start_addr, size_t new_size)
{
	b_beos_area_t *area = (b_beos_area_t*)data;
	if(!area || area->openedIPC || new_size <= 0) return B_BAD_VALUE;

	_BHAPI_LOCK_AREA_();

	b_beos_area_info_t _area_info;
	bzero(&_area_info, sizeof(b_beos_area_info_t));
	memcpy(&_area_info, area->addr, sizeof(b_beos_area_info_t));
	_area_info.length = new_size;

	uint32 requestSize = (new_size + sizeof(b_beos_area_info_t) + (B_PAGE_SIZE - 1)) & ~(B_PAGE_SIZE - 1);
	if(resize_area(area->beArea, requestSize) != B_OK)
	{
		_BHAPI_UNLOCK_AREA_();
		return B_ERROR;
	}

	area_info areaInfo;
	get_area_info(area->beArea, &areaInfo);
	area->addr = areaInfo.address;
	area->length = new_size;

	memcpy(area->addr, &_area_info, sizeof(b_beos_area_info_t));

	_BHAPI_UNLOCK_AREA_();

	if(start_addr) *start_addr = (void*)((char*)area->addr + sizeof(b_beos_area_info_t));

	return B_OK;
}


IMPEXP_BHAPI b_status_t
b_set_area_protection(void *data, b_uint32 new_protection)
{
	b_beos_area_t *area = (b_beos_area_t*)data;
	if(!area) return B_BAD_VALUE;

	uint32 prot = B_READ_AREA;
	if(new_protection & B_WRITE_AREA) prot |= B_WRITE_AREA;

	if(set_area_protection(area->beArea, prot) != B_OK) return B_ERROR;

	area->prot = new_protection;

	return B_OK;
}
#endif // BEOS
