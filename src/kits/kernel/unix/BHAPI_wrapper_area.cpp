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
#ifdef BHAPI_OS_LINUX

#include <stdlib.h>

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include "../config.h"
#include "../kernel/Kernel.h"
#include "../support/StringList.h"
#include "../support/SimpleLocker.h"


#ifdef HAVE_SHM_OPEN

typedef struct b_posix_area_t {
	b_posix_area_t()
		: name(NULL), domain(NULL), ipc_name(NULL), prot(0), length(0), addr(NULL), openedIPC(true), created(false)
	{
	}

	~b_posix_area_t()
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
	bool		openedIPC;
	bool		created;
} b_posix_area_t;

// return value must be free by "free()"
static char* b_area_ipc_name(const char *name, const char *domain)
{
	if(name == NULL || *name == 0 || strlen(name) > B_OS_NAME_LENGTH || !domain || strlen(domain) != 4) return NULL;

	const char *prefix, *slash;

	if((prefix = getenv("POSIX_SHM_IPC_PREFIX")) == NULL)
	{
#ifdef POSIX_SHM_IPC_PREFIX
		prefix = POSIX_SHM_IPC_PREFIX;
#else
#if defined(BHAPI_OS_LINUX) || defined(BHAPI_OS_SOLARIS) || defined(BHAPI_OS_DARWIN)
		prefix = "/";
#else
		prefix = "/tmp";
#endif // BHAPI_OS_LINUX || BHAPI_OS_SOLARIS
#endif
	}

	slash = (prefix[strlen(prefix) - 1] == '/') ? "" : "/";

	return bhapi::strdup_printf("%s%s%s%s%s%s", prefix, slash, "b_", domain, "_area_", name);
}


IMPEXPBHAPI void*
bhapi::create_area(const char *name, void **start_addr, size_t size, b_uint32 protection, const char *domain, bhapi::area_access area_access)
{
	if(size <= 0) return NULL;

	char *ipc_name = b_area_ipc_name(name, domain);
	if(!ipc_name) return NULL;

	b_posix_area_t *area = new b_posix_area_t();
	if(!area)
	{
		free(ipc_name);
		return NULL;
	}

	area->prot = protection;

	mode_t openMode = S_IRUSR | S_IWUSR;
	if(area_access & BHAPI_AREA_ACCESS_GROUP_READ) openMode |= S_IRGRP;
	if(area_access & BHAPI_AREA_ACCESS_GROUP_WRITE) openMode |= S_IWGRP;
	if(area_access & BHAPI_AREA_ACCESS_OTHERS_READ) openMode |= S_IROTH;
	if(area_access & BHAPI_AREA_ACCESS_OTHERS_WRITE) openMode |= S_IWOTH;

	int handler;

	if((handler = shm_open(ipc_name, O_CREAT | O_EXCL | O_RDWR, openMode)) == -1)
	{
		bool doFailed = true;

		BHAPI_DEBUG("[KERNEL]: %s --- Map \"%s\" existed, try again after unlink it.", __PRETTY_FUNCTION__, ipc_name);
		if(!(shm_unlink(ipc_name) != 0 ||
		     (handler = shm_open(ipc_name, O_CREAT | O_EXCL | O_RDWR, openMode)) == -1)) doFailed = false;

		if(doFailed)
		{
			BHAPI_DEBUG("[KERNEL]: %s --- CANNOT create map \"%s\": error_no: %d", __PRETTY_FUNCTION__, ipc_name, errno);
			free(ipc_name);
			delete area;
			return NULL;
		}
	}

	if(ftruncate(handler, size) != 0)
	{
		close(handler);
		shm_unlink(ipc_name);
		free(ipc_name);
		delete area;
		return NULL;
	}

	int prot = PROT_READ;
	if(protection & B_WRITE_AREA) prot |= PROT_WRITE;

	if((area->addr = mmap(NULL, size, prot, MAP_SHARED, handler, 0)) == MAP_FAILED)
	{
		close(handler);
		shm_unlink(ipc_name);
		free(ipc_name);
		delete area;
		return NULL;
	}

	close(handler);

	area->length = size;
	area->openedIPC = false;
	area->name = bhapi::strdup(name);
	area->domain = bhapi::strdup(domain);
	area->ipc_name = ipc_name;
	area->created = true;

	if(start_addr) *start_addr = area->addr;

//	BHAPI_DEBUG("[KERNEL]: Area created: \"%s\"-\"%s\" --- %lu", name, domain, size);

	return area;
}


IMPEXPBHAPI void*
bhapi::clone_area(const char *name, void **dest_addr, b_uint32 protection, const char *domain)
{
	char *ipc_name = b_area_ipc_name(name, domain);
	if(!ipc_name) return NULL;

	b_posix_area_t *area = new b_posix_area_t();
	if(!area)
	{
		free(ipc_name);
		return NULL;
	}

	area->prot = protection;

	int oflag;
	if(protection & B_WRITE_AREA)
		oflag = O_RDWR;
	else
		oflag = O_RDONLY;

	int handler;

	if((handler = shm_open(ipc_name, oflag, 0)) == -1)
	{
		free(ipc_name);
		delete area;
		return NULL;
	}

	struct stat stat;
	bzero(&stat, sizeof(stat));
	fstat(handler, &stat);
	size_t size = stat.st_size;
	if(size <= 0)
	{
		close(handler);
		free(ipc_name);
		delete area;
		return NULL;
	}

	int prot = PROT_READ;
	if(protection & B_WRITE_AREA) prot |= PROT_WRITE;

	if((area->addr = mmap(NULL, size, prot, MAP_SHARED, handler, 0)) == MAP_FAILED)
	{
		close(handler);
		free(ipc_name);
		delete area;
		return NULL;
	}

	close(handler);

	area->length = size;
	area->openedIPC = true;
	area->name = bhapi::strdup(name);
	area->domain = bhapi::strdup(domain);
	area->ipc_name = ipc_name;
	area->created = true;

	if(dest_addr) *dest_addr = area->addr;
	return area;
}


IMPEXPBHAPI void*
bhapi::clone_area_by_source(void *source_data, void **dest_addr, b_uint32 protection)
{
	b_posix_area_t *source_area = (b_posix_area_t*)source_data;
	if(!source_area) return NULL;

	return bhapi::clone_area(source_area->name, dest_addr, protection, source_area->domain);
}


IMPEXPBHAPI b_status_t
bhapi::get_area_info(void *data, b_area_info *info)
{
	b_posix_area_t *area = (b_posix_area_t*)data;
	if(!area || !info) return B_BAD_VALUE;
	if(!area->name || *(area->name) == 0 || strlen(area->name) > B_OS_NAME_LENGTH ||
	   !area->domain || strlen(area->domain) != 4 ||
	   area->addr == NULL || area->addr == MAP_FAILED) return B_ERROR;

	bzero(info->name, B_OS_NAME_LENGTH + 1);
	bzero(info->domain, 5);

	strcpy(info->name, area->name);
	strcpy(info->domain, area->domain);
	info->protection = area->prot;
	info->address = area->addr;
	info->size = area->length;

	return B_OK;
}


IMPEXPBHAPI b_status_t
bhapi::delete_area(void *data)
{
	b_posix_area_t *area = (b_posix_area_t*)data;
	if(!area) return B_BAD_VALUE;

	if(!(area->addr == NULL || area->addr == MAP_FAILED)) munmap(area->addr, area->length);

	if(area->openedIPC == false) shm_unlink(area->ipc_name);
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


IMPEXPBHAPI b_status_t
bhapi::delete_area_etc(void *data, bool no_clone)
{
	b_posix_area_t *area = (b_posix_area_t*)data;
	if(!area) return B_BAD_VALUE;

	if(!(area->addr == NULL || area->addr == MAP_FAILED))
		munmap(area->addr, area->length);

	if(no_clone && (area->openedIPC ? (area->prot & B_WRITE_AREA) : true)) shm_unlink(area->ipc_name);

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


IMPEXPBHAPI b_status_t
b_resize_area(void *data, void **start_addr, size_t new_size)
{
#ifdef HAVE_MREMAP
	b_posix_area_t *area = (b_posix_area_t*)data;
	if(!area || area->openedIPC || new_size <= 0) return B_BAD_VALUE;

	int handler;

	if((handler = shm_open(area->ipc_name, O_RDWR, 0)) == -1) return B_ERROR;

	if(ftruncate(handler, new_size) == 0)
	{
		void *addr;
		if((addr = mremap(area->addr, area->length, new_size, MREMAP_MAYMOVE)) == MAP_FAILED)
		{
			ftruncate(handler, area->length);
			close(handler);
			return(errno == ENOMEM ? B_NO_MEMORY : B_ERROR);
		}
		close(handler);

		area->length = new_size;
		area->addr = addr;

		if(start_addr) *start_addr = area->addr;
		return B_OK;
	}

	close(handler);
#endif

	return B_ERROR;
}


IMPEXPBHAPI b_status_t
b_set_area_protection(void *data, b_uint32 new_protection)
{
	b_posix_area_t *area = (b_posix_area_t*)data;
	if(!area) return B_BAD_VALUE;

	int prot = PROT_READ;
	if(new_protection & B_WRITE_AREA) prot |= PROT_WRITE;

	if(mprotect(area->addr, area->length, prot) != 0) return B_ERROR;

	area->prot = new_protection;

	return B_OK;
}

#else // !HAVE_SHM_OPEN

#warning "FIXME: Your system seems don't support POSIX share memory."

IMPEXPBHAPI void*
bhapi::create_area(const char *name, void **start_addr, size_t size, b_uint32 protection, const char *domain, bhapi::area_access area_access)
{
	BHAPI_WARNING("[KERNEL]: %s --- System don't support POSIX share memory!", __PRETTY_FUNCTION__);
	return NULL;
}


IMPEXPBHAPI void*
bhapi::clone_area(const char *name, void **dest_addr, b_uint32 protection, const char *domain)
{
	return NULL;
}


IMPEXPBHAPI void*
bhapi::clone_area_by_source(void *source_data, void **dest_addr, b_uint32 protection)
{
	return NULL;
}


IMPEXPBHAPI b_status_t
bhapi::get_area_info(void *data, b_area_info *info)
{
	return B_ERROR;
}


IMPEXPBHAPI b_status_t
bhapi::delete_area(void *data)
{
	return B_ERROR;
}


IMPEXPBHAPI b_status_t
bhapi::delete_area_etc(void *data, bool no_clone)
{
	return B_ERROR;
}


IMPEXPBHAPI b_status_t
b_resize_area(void *data, void **start_addr, size_t new_size)
{
	return B_ERROR;
}


IMPEXPBHAPI b_status_t
b_set_area_protection(void *data, b_uint32 new_protection)
{
	return B_ERROR;
}

#endif // HAVE_SHM_OPEN
#endif // BHAPI_OS_LINUX

