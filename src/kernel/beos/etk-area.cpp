/* --------------------------------------------------------------------------
 *
 * ETK++ --- The Easy Toolkit for C++ programing
 * Copyright (C) 2004-2006, Anthony Lee, All Rights Reserved
 *
 * ETK++ library is a freeware; it may be used and distributed according to
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

#include <be/kernel/OS.h"

#include "./../kernel/Kernel.h"
#include "./../support/String.h"

#define BEOS_AREA_INFO_MAGIC		0xABFC

typedef struct etk_beos_area_t {
	etk_beos_area_t()
		: name(NULL), domain(NULL), ipc_name(NULL), prot(0), length(0), addr(NULL), beArea(-1), openedIPC(false), created(false)
	{
	}

	~etk_beos_area_t()
	{
		if(created)
		{
			created = false;
			etk_delete_area((void*)this);
		}
	}

	char		*name;
	char		*domain;
	char		*ipc_name;
	euint32		prot;
	size_t		length;
	void		*addr;
	area_id		beArea;
	bool		openedIPC;
	bool		created;
} etk_beos_area_t;


// return value must be free by "free()"
static char* etk_area_ipc_name(const char *name, const char *domain)
{
	if(name == NULL || *name == 0 || strlen(name) > E_OS_NAME_LENGTH || !domain || strlen(domain) != 4) return NULL;
	if(strlen(name) > B_OS_NAME_LENGTH - 4)
	{
		ETK_WARNING("\n==================================================================\n[KERNEL]: %s --- Length of area's name exceeds %d.\n==================================================================\n", __PRETTY_FUNCTION__, B_OS_NAME_LENGTH - 4);
		return NULL;
	}
	return e_strdup_printf("%s%s", domain, name);
}

class etk_beos_area_locker_t {
public:
	etk_beos_area_locker_t()
	{
		const char *lockerName = "__etk_G_area_locker__";

		if((iLocker = find_port(lockerName)) < 0)
		{
			if((iLocker = create_port(1, lockerName)) >= 0)
			{
				char buf = 1;
				if(set_port_owner(iLocker, B_SYSTEM_TEAM) != B_OK || write_port(iLocker, 'etk_', &buf, 1) != B_OK)
				{
					delete_port(iLocker);
					iLocker = -1;
				}
			}
			if(iLocker >= 0) ETK_DEBUG("[KERNEL]: port for global area locker created.");
		}
		else
		{
			port_info portInfo;
			if(get_port_info(iLocker, &portInfo) != B_OK || portInfo.capacity != 1) iLocker = -1;
			if(iLocker >= 0) ETK_DEBUG("[KERNEL]: port for global area locker found.");
		}
		if(iLocker < 0) ETK_ERROR("[KERNEL]: Can't initialize global area!");
	}

	void Lock()
	{
		while(true)
		{
//			ETK_DEBUG("[KERNEL]: try locking global area.");

			int32 msgCode = 0;
			char buf = 0;
			ssize_t readBytes = read_port(iLocker, &msgCode, &buf, 1);
			if(readBytes < 1) continue;
			if(readBytes != 1 || msgCode != 'etk_' || buf != 1)
				ETK_ERROR("[KERNEL]: Unable to lock the locker for global area.");
//			ETK_DEBUG("[KERNEL]: global area locker locked.");
			break;
		}
	}

	void Unlock()
	{
		char buf = 1;
		if(write_port(iLocker, 'etk_', &buf, 1) != B_OK) ETK_ERROR("[KERNEL]: Unable to unlock the locker for global area.");
//		ETK_DEBUG("[KERNEL]: global area locker unlocked.");
	}

	port_id iLocker;
};

static etk_beos_area_locker_t __etk_area_locker__;

static void _ETK_LOCK_AREA_()
{
	__etk_area_locker__.Lock();
}

static void _ETK_UNLOCK_AREA_()
{
	__etk_area_locker__.Unlock();
}


typedef struct etk_beos_area_info_t {
	eint32		magic;
	bool		closed;
	size_t		length;
} etk_beos_area_info_t;


_IMPEXP_ETK void*
etk_create_area(const char *name, void **start_addr, size_t size, euint32 protection, const char *domain, etk_area_access area_access)
{
	if(size <= 0) return NULL;

	char *ipc_name = etk_area_ipc_name(name, domain);
	if(!ipc_name) return NULL;

	etk_beos_area_t *area = new etk_beos_area_t();
	if(!area)
	{
		free(ipc_name);
		return NULL;
	}

	area->prot = protection;
	uint32 prot = B_READ_AREA;
	if(protection & E_WRITE_AREA) prot |= B_WRITE_AREA;

//	if(area_access != ETK_AREA_ACCESS_OWNER) ETK_DEBUG("[KERNEL]: %s --- Access unsupport.", __PRETTY_FUNCTION__);

	_ETK_LOCK_AREA_();
	if(find_area(ipc_name) >= 0)
	{
		_ETK_UNLOCK_AREA_();
		ETK_WARNING("[KERNEL]: %s --- area already exists.", __PRETTY_FUNCTION__);
		free(ipc_name);
		delete area;
		return NULL;
	}

	uint32 requestSize = (size + sizeof(etk_beos_area_info_t) + (B_PAGE_SIZE - 1)) & ~(B_PAGE_SIZE - 1);
	if((area->beArea = create_area(ipc_name, &(area->addr), B_ANY_ADDRESS,
				       requestSize, B_NO_LOCK, prot)) < 0 || area->addr == NULL)
	{
		if(area->beArea >= 0) delete_area(area->beArea);
		_ETK_UNLOCK_AREA_();
		ETK_DEBUG("[KERNEL]: %s --- Unable to create area \"%s\": error_no: %d",
				  __PRETTY_FUNCTION__, ipc_name, area->beArea);
		free(ipc_name);
		delete area;
		return NULL;
	}

	etk_beos_area_info_t _area_info;
	_area_info.magic = BEOS_AREA_INFO_MAGIC;
	_area_info.closed = false;
	_area_info.length = size;
	memcpy(area->addr, &_area_info, sizeof(etk_beos_area_info_t));

	area->length = size;
	area->name = e_strdup(name);
	area->domain = e_strdup(domain);
	area->ipc_name = ipc_name;
	area->created = true;

	_ETK_UNLOCK_AREA_();

	if(start_addr) *start_addr = (void*)((char*)area->addr + sizeof(etk_beos_area_info_t));

//	ETK_DEBUG("[KERNEL]: Area created: \"%s\"-\"%s\" --- %lu", name, domain, size);

	return area;
}


_IMPEXP_ETK void*
etk_clone_area(const char *name, void **dest_addr, euint32 protection, const char *domain)
{
	char *ipc_name = etk_area_ipc_name(name, domain);
	if(!ipc_name) return NULL;

	etk_beos_area_t *area = new etk_beos_area_t();
	if(!area)
	{
		free(ipc_name);
		return NULL;
	}

	area->prot = protection;
	uint32 prot = B_READ_AREA;
	if(protection & E_WRITE_AREA) prot |= B_WRITE_AREA;

	_ETK_LOCK_AREA_();

	if((area->beArea = clone_area(ipc_name, &(area->addr), B_ANY_ADDRESS, prot, find_area(ipc_name))) < 0 || area->addr == NULL)
	{
		if(area->beArea >= 0) delete_area(area->beArea);
		_ETK_UNLOCK_AREA_();
//		ETK_DEBUG("[KERNEL]: %s --- Unable to clone area \"%s\": error_no: %d", __PRETTY_FUNCTION__, ipc_name, area->beArea);
		free(ipc_name);
		delete area;
		return NULL;
	}

	etk_beos_area_info_t _area_info;
	bzero(&_area_info, sizeof(etk_beos_area_info_t));
	memcpy(&_area_info, area->addr, sizeof(etk_beos_area_info_t));
	if(_area_info.magic != BEOS_AREA_INFO_MAGIC || _area_info.closed)
	{
		delete_area(area->beArea);
		_ETK_UNLOCK_AREA_();
		ETK_WARNING("[KERNEL]: %s --- area(%s) %s.",
					__PRETTY_FUNCTION__, ipc_name,
					(_area_info.magic != BEOS_AREA_INFO_MAGIC ? "seems not created by ETK" : "already deleted and not allowed to clone"));
		free(ipc_name);
		delete area;
		return NULL;
	}

	area->length = _area_info.length;
	area->name = e_strdup(name);
	area->domain = e_strdup(domain);
	area->ipc_name = ipc_name;
	area->openedIPC = true;
	area->created = true;

	_ETK_UNLOCK_AREA_();

	if(dest_addr) *dest_addr = (void*)((char*)area->addr + sizeof(etk_beos_area_info_t));
	return area;
}


_IMPEXP_ETK void*
etk_clone_area_by_source(void *source_data, void **dest_addr, euint32 protection)
{
	etk_beos_area_t *source_area = (etk_beos_area_t*)source_data;
	if(!source_area) return NULL;

	return etk_clone_area(source_area->name, dest_addr, protection, source_area->domain);
}


_IMPEXP_ETK e_status_t
etk_get_area_info(void *data, etk_area_info *info)
{
	etk_beos_area_t *area = (etk_beos_area_t*)data;
	if(!area || !info) return E_BAD_VALUE;
	if(!area->name || *(area->name) == 0 || strlen(area->name) > E_OS_NAME_LENGTH ||
	   !area->domain || strlen(area->domain) != 4 ||
	   area->addr == NULL) return E_ERROR;

	bzero(info->name, E_OS_NAME_LENGTH + 1);
	bzero(info->domain, 5);

	info->size = area->length;
	strcpy(info->name, area->name);
	strcpy(info->domain, area->domain);
	info->protection = area->prot;
	info->address = (void*)((char*)area->addr + sizeof(etk_beos_area_info_t));

	return E_OK;
}


_IMPEXP_ETK e_status_t
etk_delete_area(void *data)
{
	etk_beos_area_t *area = (etk_beos_area_t*)data;
	if(!area) return E_BAD_VALUE;

	if(!area->openedIPC)
	{
		if(!(area->prot & E_WRITE_AREA)) set_area_protection(area->beArea, B_READ_AREA | B_WRITE_AREA);

		_ETK_LOCK_AREA_();
		etk_beos_area_info_t _area_info;
		bzero(&_area_info, sizeof(etk_beos_area_info_t));
		memcpy(&_area_info, area->addr, sizeof(etk_beos_area_info_t));
		if(_area_info.magic == BEOS_AREA_INFO_MAGIC && !_area_info.closed)
		{
			_area_info.closed = true;
			memcpy(area->addr, &_area_info, sizeof(etk_beos_area_info_t));
		}
		_ETK_UNLOCK_AREA_();
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

	return E_OK;
}


_IMPEXP_ETK e_status_t
etk_delete_area_etc(void *data, bool no_clone)
{
	etk_beos_area_t *area = (etk_beos_area_t*)data;
	if(!area) return E_BAD_VALUE;

	if(no_clone && (area->openedIPC ? (area->prot & E_WRITE_AREA) : true))
	{
		if(!(area->prot & E_WRITE_AREA)) set_area_protection(area->beArea, B_READ_AREA | B_WRITE_AREA);

		_ETK_LOCK_AREA_();
		etk_beos_area_info_t _area_info;
		bzero(&_area_info, sizeof(etk_beos_area_info_t));
		memcpy(&_area_info, area->addr, sizeof(etk_beos_area_info_t));
		if(_area_info.magic == BEOS_AREA_INFO_MAGIC && !_area_info.closed)
		{
			_area_info.closed = true;
			memcpy(area->addr, &_area_info, sizeof(etk_beos_area_info_t));
		}
		_ETK_UNLOCK_AREA_();
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

	return E_OK;
}


_IMPEXP_ETK e_status_t
etk_resize_area(void *data, void **start_addr, size_t new_size)
{
	etk_beos_area_t *area = (etk_beos_area_t*)data;
	if(!area || area->openedIPC || new_size <= 0) return E_BAD_VALUE;

	_ETK_LOCK_AREA_();

	etk_beos_area_info_t _area_info;
	bzero(&_area_info, sizeof(etk_beos_area_info_t));
	memcpy(&_area_info, area->addr, sizeof(etk_beos_area_info_t));
	_area_info.length = new_size;

	uint32 requestSize = (new_size + sizeof(etk_beos_area_info_t) + (B_PAGE_SIZE - 1)) & ~(B_PAGE_SIZE - 1);
	if(resize_area(area->beArea, requestSize) != B_OK)
	{
		_ETK_UNLOCK_AREA_();
		return E_ERROR;
	}

	area_info areaInfo;
	get_area_info(area->beArea, &areaInfo);
	area->addr = areaInfo.address;
	area->length = new_size;

	memcpy(area->addr, &_area_info, sizeof(etk_beos_area_info_t));

	_ETK_UNLOCK_AREA_();

	if(start_addr) *start_addr = (void*)((char*)area->addr + sizeof(etk_beos_area_info_t));

	return E_OK;
}


_IMPEXP_ETK e_status_t
etk_set_area_protection(void *data, euint32 new_protection)
{
	etk_beos_area_t *area = (etk_beos_area_t*)data;
	if(!area) return E_BAD_VALUE;

	uint32 prot = B_READ_AREA;
	if(new_protection & E_WRITE_AREA) prot |= B_WRITE_AREA;

	if(set_area_protection(area->beArea, prot) != B_OK) return E_ERROR;

	area->prot = new_protection;

	return E_OK;
}
