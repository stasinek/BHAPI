/* --------------------------------------------------------------------------
 *
 * BHAPI++ Copyright (C) 2017, Stanislaw Stasiak, based on Haiku & ETK++, The Easy Toolkit for C++ programing
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
 * File: BHAPI_wrapper_area.cpp
 *
 * --------------------------------------------------------------------------*/

#include "../../support/SupportDefs.h"
#include "../../kernel/Kernel.h"
#include "../../kernel/Debug.h"
#include "../../support/StringClass.h"
#include "../../support/Errors.h"

#include <winsock2.h>
#include <windows.h>
//#include <sddl.h"

#define WIN32_AREA_INFO_MAGIC		0xABFC

// return value must be free by "free()"
static char* b_area_ipc_name(const char *name, const char *domain)
{
	if(name == NULL || *name == 0 || strlen(name) > B_OS_NAME_LENGTH || !domain || strlen(domain) != 4) return NULL;

	const char *prefix = "__bhapi_";

	return bhapi::strdup_printf("%s%s%s%s", prefix, domain, "_area_", name);
}


class bhapi::win32_area_locker_t {
public:
	bhapi::win32_area_locker_t()
	{
		const char *lockerName = "_bhapi_area_global_";
        if((iLocker = OpenMutexA(MUTEX_ALL_ACCESS, FALSE, lockerName)) == NULL)
            iLocker = CreateMutexA(NULL, FALSE, lockerName);
		if(iLocker == NULL) BHAPI_ERROR("[KERNEL]: Can't initialize global area!");
	}

	~bhapi::win32_area_locker_t()
	{
		if(iLocker) CloseHandle(iLocker);
	}

	void Lock() {WaitForSingleObject(iLocker, INFINITE);}
	void Unlock() {ReleaseMutex(iLocker);}

	HANDLE iLocker;
};

static bhapi::win32_area_locker_t __bhapi_area_locker__;

static void BHAPI_LOCK_AREA()
{
	__bhapi_area_locker__.Lock();
}

static void BHAPI_UNLOCK_AREA()
{
	__bhapi_area_locker__.Unlock();
}


typedef struct bhapi::win32_area_info_t {
	__be_int32		magic;
	bool		closed;
	size_t		length;
} bhapi::win32_area_info_t;


typedef struct bhapi::win32_area_t {
	bhapi::win32_area_t()
		: name(NULL), domain(NULL), ipc_name(NULL), prot(0), length(0), addr(NULL), mapping(NULL), openedIPC(false), created(false)
	{
	}

	~bhapi::win32_area_t()
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
	__be_uint32		prot;
	size_t		length;
	void		*addr;
	HANDLE		mapping;
	bool		openedIPC;
	bool		created;
} bhapi::win32_area_t;


BHAPI_EXPORT void*
bhapi::create_area(const char *name, void **start_addr, size_t size,  __be_uint32 protection, const char *domain, bhapi::area_access area_access)
{
	if(size <= 0) return NULL;

	char *ipc_name = b_area_ipc_name(name, domain);
	if(!ipc_name) return NULL;

	bhapi::win32_area_t *area = new bhapi::win32_area_t();
	if(!area)
	{
		free(ipc_name);
		return NULL;
	}

	area->prot = protection;

#if 0
	SECURITY_ATTRIBUTES sa;
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.bInheritHandle = TRUE;
	sa.lpSecurityDescriptor = NULL;

	BString szStr;
	szStr << "D:";				// Discretionary ACL
	szStr << "(D;OICI;GA;;;BG)";		// Deny access to built-in guests
	szStr << "(D;OICI;GA;;;AN)";		// Deny access to anonymous logon
	szStr << "(A;OICI;GA;;;BA)";		// Allow full control to administrators
	szStr << "(A;OICI;GA;;;CO)";		// Allow full control to creator owner

	if((area_access & BHAPI_AREA_ACCESS_GROUP_READ) || (area_access & BHAPI_AREA_ACCESS_GROUP_WRITE))
	{
		if(area_access & BHAPI_AREA_ACCESS_GROUP_WRITE)
			szStr << "(A;OICI;GA;;;CG)";	// Allow full control to creator group
		else
			szStr << "(A;OICI;GR;;;CG)";	// Allow read control to creator group
	}

	if((area_access & BHAPI_AREA_ACCESS_OTHERS_READ) || (area_access & BHAPI_AREA_ACCESS_OTHERS_WRITE))
	{
		if(area_access & BHAPI_AREA_ACCESS_OTHERS_WRITE)
			szStr << "(A;OICI;GA;;;BU)";	// Allow full control to others
		else
			szStr << "(A;OICI;GR;;;BU)";	// Allow read control to others
	}

	if(!ConvertStringSecurityDescriptorToSecurityDescriptor(szStr.String(), SDDL_REVISION_1, (PSECURITY_DESCRIPTOR*)&(sa.lpSecurityDescriptor), NULL))
	{
		delete area;
		free(ipc_name);
		return NULL;
	}
#endif

	HANDLE handler;

	BHAPI_LOCK_AREA();
    if((handler = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE | SEC_COMMIT, 0, size + sizeof(bhapi::win32_area_info_t), ipc_name)) == NULL)
	{
		BHAPI_UNLOCK_AREA();
		BHAPI_DEBUG("[KERNEL]: %s --- Can't create area : CreateFileMapping failed.", __PRETTY_FUNCTION__);
		free(ipc_name);
		delete area;
		return NULL;
	}

//	if(sa.lpSecurityDescriptor != NULL) LocalFree(sa.lpSecurityDescriptor);

	DWORD prot = (protection & B_WRITE_AREA ? FILE_MAP_ALL_ACCESS : FILE_MAP_READ);

	if((area->addr = MapViewOfFile(handler, prot, 0, 0, 0)) == NULL)
	{
		BHAPI_DEBUG("[KERNEL]: %s --- Can't create area : MapViewOfFile failed.", __PRETTY_FUNCTION__);
		CloseHandle(handler);
		BHAPI_UNLOCK_AREA();
		free(ipc_name);
		delete area;
		return NULL;
	}

	bhapi::win32_area_info_t area_info;
	area_info.magic = WIN32_AREA_INFO_MAGIC;
	area_info.closed = false;
	area_info.length = size;
	memcpy(area->addr, &area_info, sizeof(bhapi::win32_area_info_t));

	area->length = size;
	area->mapping = handler;
	area->name = bhapi::strdup(name);
	area->domain = bhapi::strdup(domain);
	area->ipc_name = ipc_name;
	area->created = true;

	BHAPI_UNLOCK_AREA();

	if(start_addr) *start_addr = (void*)((char*)area->addr + sizeof(bhapi::win32_area_info_t));
	return area;
}


BHAPI_EXPORT void*
bhapi::clone_area(const char *name, void **dest_addr,  __be_uint32 protection, const char *domain)
{
	char *ipc_name = b_area_ipc_name(name, domain);
	if(!ipc_name) return NULL;

	bhapi::win32_area_t *area = new bhapi::win32_area_t();
	if(!area)
	{
		free(ipc_name);
		return NULL;
	}

	area->prot = protection;

	DWORD prot = (protection & B_WRITE_AREA ? FILE_MAP_ALL_ACCESS : FILE_MAP_READ);

	HANDLE handler;

	BHAPI_LOCK_AREA();
    if((handler = OpenFileMappingA(prot, FALSE, ipc_name)) == NULL)
	{
//		BHAPI_DEBUG("[KERNEL]: %s --- Can't clone area : open file mapping failed."", __PRETTY_FUNCTION__);
		BHAPI_UNLOCK_AREA();
		free(ipc_name);
		delete area;
		return NULL;
	}

	if((area->addr = MapViewOfFile(handler, prot, 0, 0, 0)) == NULL)
	{
		BHAPI_DEBUG("[KERNEL]: %s --- Can't clone area : MapViewOfFile failed.", __PRETTY_FUNCTION__);
		CloseHandle(handler);
		BHAPI_UNLOCK_AREA();
		free(ipc_name);
		delete area;
		return NULL;
	}

	bhapi::win32_area_info_t area_info;
	bzero(&area_info, sizeof(bhapi::win32_area_info_t));
	memcpy(&area_info, area->addr, sizeof(bhapi::win32_area_info_t));
	if(area_info.magic != WIN32_AREA_INFO_MAGIC || area_info.closed)
	{
		BHAPI_WARNING("[KERNEL]: %s --- FileMapping(%s) seem not created by BHAPI", __PRETTY_FUNCTION__, ipc_name);
		UnmapViewOfFile(area->addr);
		CloseHandle(handler);
		BHAPI_UNLOCK_AREA();
		free(ipc_name);
		delete area;
		return NULL;
	}

	area->length = area_info.length;
	area->mapping = handler;
	area->name = bhapi::strdup(name);
	area->domain = bhapi::strdup(domain);
	area->ipc_name = ipc_name;
	area->openedIPC = true;
	area->created = true;

	BHAPI_UNLOCK_AREA();

	if(dest_addr) *dest_addr = (void*)((char*)area->addr + sizeof(bhapi::win32_area_info_t));
	return area;
}


BHAPI_EXPORT void*
bhapi::clone_area_by_source(void *source_data, void **dest_addr,  __be_uint32 protection)
{
	bhapi::win32_area_t *source_area = (bhapi::win32_area_t*)source_data;
	if(!source_area) return NULL;

	return bhapi::clone_area(source_area->name, dest_addr, protection, source_area->domain);
}


BHAPI_EXPORT status_t 
bhapi::get_area_info(void *data, b_area_info *info)
{
	bhapi::win32_area_t *area = (bhapi::win32_area_t*)data;
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
	info->address = (void*)((char*)area->addr + sizeof(bhapi::win32_area_info_t));

	return B_OK;
}


BHAPI_EXPORT status_t 
bhapi::delete_area(void *data)
{
	bhapi::win32_area_t *area = (bhapi::win32_area_t*)data;
	if(!area) return B_BAD_VALUE;

	if(!area->openedIPC && area->mapping && area->addr)
	{
		if(!(area->prot & B_WRITE_AREA))
		{
			void *new_addr = MapViewOfFile(area->mapping, FILE_MAP_ALL_ACCESS, 0, 0, 0);
			if(new_addr == NULL) BHAPI_ERROR("[KERNEL]: %s --- MapViewOfFile again failed.", __PRETTY_FUNCTION__);
			UnmapViewOfFile(area->addr);
			area->addr = new_addr;
		}

		BHAPI_LOCK_AREA();
		bhapi::win32_area_info_t area_info;
		bzero(&area_info, sizeof(bhapi::win32_area_info_t));
		memcpy(&area_info, area->addr, sizeof(bhapi::win32_area_info_t));
		if(area_info.magic == WIN32_AREA_INFO_MAGIC && !area_info.closed)
		{
			area_info.closed = true;
			memcpy(area->addr, &area_info, sizeof(bhapi::win32_area_info_t));
		}
		BHAPI_UNLOCK_AREA();
	}

	if(area->addr) UnmapViewOfFile(area->addr);
	if(area->mapping) CloseHandle(area->mapping);

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


BHAPI_EXPORT status_t 
bhapi::delete_area_etc(void *data, bool no_clone)
{
	bhapi::win32_area_t *area = (bhapi::win32_area_t*)data;
	if(!area) return B_BAD_VALUE;

	if(no_clone && (area->openedIPC ? (area->prot & B_WRITE_AREA) : true) && area->mapping && area->addr)
	{
		if(!(area->prot & B_WRITE_AREA))
		{
			void *new_addr = MapViewOfFile(area->mapping, FILE_MAP_ALL_ACCESS, 0, 0, 0);
			if(new_addr == NULL) BHAPI_ERROR("[KERNEL]: %s --- MapViewOfFile again failed.", __PRETTY_FUNCTION__);
			UnmapViewOfFile(area->addr);
			area->addr = new_addr;
		}

		BHAPI_LOCK_AREA();
		bhapi::win32_area_info_t area_info;
		bzero(&area_info, sizeof(bhapi::win32_area_info_t));
		memcpy(&area_info, area->addr, sizeof(bhapi::win32_area_info_t));
		if(area_info.magic == WIN32_AREA_INFO_MAGIC && !area_info.closed)
		{
			area_info.closed = true;
			memcpy(area->addr, &area_info, sizeof(bhapi::win32_area_info_t));
		}
		BHAPI_UNLOCK_AREA();
	}

	if(area->addr) UnmapViewOfFile(area->addr);
	if(area->mapping) CloseHandle(area->mapping);

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


BHAPI_EXPORT status_t 
b_resize_area(void *data, void **start_addr, size_t new_size)
{
	BHAPI_WARNING("%s: Not supported.", __PRETTY_FUNCTION__);
	return B_ERROR;
}


BHAPI_EXPORT status_t 
b_set_area_protection(void *data,  __be_uint32 new_protection)
{
	BHAPI_WARNING("%s: Not supported.", __PRETTY_FUNCTION__);
	return B_ERROR;
}

