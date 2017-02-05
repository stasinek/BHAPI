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

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include "./../config.h>
#include "./../kernel/Kernel.h>
#include "./../support/StringArray.h>
#include "./../support/SimpleLocker.h>


#ifdef HAVE_SHM_OPEN

typedef struct etk_posix_area_t {
	etk_posix_area_t()
		: name(NULL), domain(NULL), ipc_name(NULL), prot(0), length(0), addr(NULL), openedIPC(true), created(false)
	{
	}

	~etk_posix_area_t()
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
	bool		openedIPC;
	bool		created;
} etk_posix_area_t;

// return value must be free by "free()"
static char* etk_area_ipc_name(const char *name, const char *domain)
{
	if(name == NULL || *name == 0 || strlen(name) > E_OS_NAME_LENGTH || !domain || strlen(domain) != 4) return NULL;

	const char *prefix, *slash;

	if((prefix = getenv("POSIX_SHM_IPC_PREFIX")) == NULL)
	{
#ifdef POSIX_SHM_IPC_PREFIX
		prefix = POSIX_SHM_IPC_PREFIX;
#else
#if defined(ETK_OS_LINUX) || defined(ETK_OS_SOLARIS) || defined(ETK_OS_DARWIN)
		prefix = "/";
#else
		prefix = "/tmp";
#endif // ETK_OS_LINUX || ETK_OS_SOLARIS
#endif
	}

	slash = (prefix[strlen(prefix) - 1] == '/') ? "" : "/";

	return e_strdup_printf("%s%s%s%s%s%s", prefix, slash, "etk_", domain, "_area_", name);
}


_IMPEXP_ETK void*
etk_create_area(const char *name, void **start_addr, size_t size, euint32 protection, const char *domain, etk_area_access area_access)
{
	if(size <= 0) return NULL;

	char *ipc_name = etk_area_ipc_name(name, domain);
	if(!ipc_name) return NULL;

	etk_posix_area_t *area = new etk_posix_area_t();
	if(!area)
	{
		free(ipc_name);
		return NULL;
	}

	area->prot = protection;

	mode_t openMode = S_IRUSR | S_IWUSR;
	if(area_access & ETK_AREA_ACCESS_GROUP_READ) openMode |= S_IRGRP;
	if(area_access & ETK_AREA_ACCESS_GROUP_WRITE) openMode |= S_IWGRP;
	if(area_access & ETK_AREA_ACCESS_OTHERS_READ) openMode |= S_IROTH;
	if(area_access & ETK_AREA_ACCESS_OTHERS_WRITE) openMode |= S_IWOTH;

	int handler;

	if((handler = shm_open(ipc_name, O_CREAT | O_EXCL | O_RDWR, openMode)) == -1)
	{
		bool doFailed = true;

		ETK_DEBUG("[KERNEL]: %s --- Map \"%s\" existed, try again after unlink it.", __PRETTY_FUNCTION__, ipc_name);
		if(!(shm_unlink(ipc_name) != 0 ||
		     (handler = shm_open(ipc_name, O_CREAT | O_EXCL | O_RDWR, openMode)) == -1)) doFailed = false;

		if(doFailed)
		{
			ETK_DEBUG("[KERNEL]: %s --- CANNOT create map \"%s\": error_no: %d", __PRETTY_FUNCTION__, ipc_name, errno);
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
	if(protection & E_WRITE_AREA) prot |= PROT_WRITE;

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
	area->name = e_strdup(name);
	area->domain = e_strdup(domain);
	area->ipc_name = ipc_name;
	area->created = true;

	if(start_addr) *start_addr = area->addr;

//	ETK_DEBUG("[KERNEL]: Area created: \"%s\"-\"%s\" --- %lu", name, domain, size);

	return area;
}


_IMPEXP_ETK void*
etk_clone_area(const char *name, void **dest_addr, euint32 protection, const char *domain)
{
	char *ipc_name = etk_area_ipc_name(name, domain);
	if(!ipc_name) return NULL;

	etk_posix_area_t *area = new etk_posix_area_t();
	if(!area)
	{
		free(ipc_name);
		return NULL;
	}

	area->prot = protection;

	int oflag;
	if(protection & E_WRITE_AREA)
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
	if(protection & E_WRITE_AREA) prot |= PROT_WRITE;

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
	area->name = e_strdup(name);
	area->domain = e_strdup(domain);
	area->ipc_name = ipc_name;
	area->created = true;

	if(dest_addr) *dest_addr = area->addr;
	return area;
}


_IMPEXP_ETK void*
etk_clone_area_by_source(void *source_data, void **dest_addr, euint32 protection)
{
	etk_posix_area_t *source_area = (etk_posix_area_t*)source_data;
	if(!source_area) return NULL;

	return etk_clone_area(source_area->name, dest_addr, protection, source_area->domain);
}


_IMPEXP_ETK e_status_t
etk_get_area_info(void *data, etk_area_info *info)
{
	etk_posix_area_t *area = (etk_posix_area_t*)data;
	if(!area || !info) return E_BAD_VALUE;
	if(!area->name || *(area->name) == 0 || strlen(area->name) > E_OS_NAME_LENGTH ||
	   !area->domain || strlen(area->domain) != 4 ||
	   area->addr == NULL || area->addr == MAP_FAILED) return E_ERROR;

	bzero(info->name, E_OS_NAME_LENGTH + 1);
	bzero(info->domain, 5);

	strcpy(info->name, area->name);
	strcpy(info->domain, area->domain);
	info->protection = area->prot;
	info->address = area->addr;
	info->size = area->length;

	return E_OK;
}


_IMPEXP_ETK e_status_t
etk_delete_area(void *data)
{
	etk_posix_area_t *area = (etk_posix_area_t*)data;
	if(!area) return E_BAD_VALUE;

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

	return E_OK;
}


_IMPEXP_ETK e_status_t
etk_delete_area_etc(void *data, bool no_clone)
{
	etk_posix_area_t *area = (etk_posix_area_t*)data;
	if(!area) return E_BAD_VALUE;

	if(!(area->addr == NULL || area->addr == MAP_FAILED))
		munmap(area->addr, area->length);

	if(no_clone && (area->openedIPC ? (area->prot & E_WRITE_AREA) : true)) shm_unlink(area->ipc_name);

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
#ifdef HAVE_MREMAP
	etk_posix_area_t *area = (etk_posix_area_t*)data;
	if(!area || area->openedIPC || new_size <= 0) return E_BAD_VALUE;

	int handler;

	if((handler = shm_open(area->ipc_name, O_RDWR, 0)) == -1) return E_ERROR;

	if(ftruncate(handler, new_size) == 0)
	{
		void *addr;
		if((addr = mremap(area->addr, area->length, new_size, MREMAP_MAYMOVE)) == MAP_FAILED)
		{
			ftruncate(handler, area->length);
			close(handler);
			return(errno == ENOMEM ? E_NO_MEMORY : E_ERROR);
		}
		close(handler);

		area->length = new_size;
		area->addr = addr;

		if(start_addr) *start_addr = area->addr;
		return E_OK;
	}

	close(handler);
#endif

	return E_ERROR;
}


_IMPEXP_ETK e_status_t
etk_set_area_protection(void *data, euint32 new_protection)
{
	etk_posix_area_t *area = (etk_posix_area_t*)data;
	if(!area) return E_BAD_VALUE;

	int prot = PROT_READ;
	if(new_protection & E_WRITE_AREA) prot |= PROT_WRITE;

	if(mprotect(area->addr, area->length, prot) != 0) return E_ERROR;

	area->prot = new_protection;

	return E_OK;
}

#else // !HAVE_SHM_OPEN

#warning "FIXME: Your system seems don't support POSIX share memory."

_IMPEXP_ETK void*
etk_create_area(const char *name, void **start_addr, size_t size, euint32 protection, const char *domain, etk_area_access area_access)
{
	ETK_WARNING("[KERNEL]: %s --- System don't support POSIX share memory!", __PRETTY_FUNCTION__);
	return NULL;
}


_IMPEXP_ETK void*
etk_clone_area(const char *name, void **dest_addr, euint32 protection, const char *domain)
{
	return NULL;
}


_IMPEXP_ETK void*
etk_clone_area_by_source(void *source_data, void **dest_addr, euint32 protection)
{
	return NULL;
}


_IMPEXP_ETK e_status_t
etk_get_area_info(void *data, etk_area_info *info)
{
	return E_ERROR;
}


_IMPEXP_ETK e_status_t
etk_delete_area(void *data)
{
	return E_ERROR;
}


_IMPEXP_ETK e_status_t
etk_delete_area_etc(void *data, bool no_clone)
{
	return E_ERROR;
}


_IMPEXP_ETK e_status_t
etk_resize_area(void *data, void **start_addr, size_t new_size)
{
	return E_ERROR;
}


_IMPEXP_ETK e_status_t
etk_set_area_protection(void *data, euint32 new_protection)
{
	return E_ERROR;
}

#endif // HAVE_SHM_OPEN

