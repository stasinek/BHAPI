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
 * File: etk-semaphore.cpp
 *
 * --------------------------------------------------------------------------*/

#include <stdio.h"

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0500
#endif

#include <windows.h>

#include "./../kernel/Kernel.h"
#include "./../support/String.h"

#define SECS_BETWEEN_EPOCHS	E_INT64_CONSTANT(11644473600)
#define SECS_TO_100NS		E_INT64_CONSTANT(10000000)

typedef struct etk_win32_sem_info {
	etk_win32_sem_info()
	{
		InitData();
	}

	void InitData()
	{
		bzero(name, E_OS_NAME_LENGTH + 1);
		latestHolderTeamId = E_INT64_CONSTANT(0);
		latestHolderThreadId = E_INT64_CONSTANT(0);
		count = E_INT64_CONSTANT(0);
		minAcquiringCount = E_INT64_CONSTANT(0);
		acquiringCount = E_INT64_CONSTANT(0);
		closed = false;
		refCount = 0;
	}

	void SetLatestHolderTeamId(eint64 id)
	{
		latestHolderTeamId = id;
	}

	void SetLatestHolderThreadId(eint64 id)
	{
		latestHolderThreadId = id;
	}

	bool LatestHolderTeamIsCurrent(void)
	{
		return(latestHolderTeamId == etk_get_current_team_id());
	}

	bool LatestHolderThreadIsCurrent(void)
	{
		return(latestHolderThreadId == etk_get_current_thread_id());
	}

	char			name[E_OS_NAME_LENGTH + 1];
	eint64			latestHolderTeamId;
	eint64			latestHolderThreadId;
	eint64			count;
	eint64			minAcquiringCount;
	eint64			acquiringCount;
	bool			closed;

	euint32			refCount;
} etk_win32_sem_info;


typedef struct etk_win32_sem_t {
	etk_win32_sem_t()
		: mapping(NULL), semInfo(NULL),
		  Mutex(NULL), Event(NULL),
		  created(false), no_clone(false)
	{
	}

	~etk_win32_sem_t()
	{
		if(created)
		{
			created = false;
			etk_delete_sem((void*)this);
		}
	}

	void			*mapping;
	etk_win32_sem_info	*semInfo;

	HANDLE			Mutex;
	HANDLE			Event;

	bool			created;
	bool			no_clone;
} etk_win32_sem_t;


class etk_win32_sem_locker_t {
public:
	etk_win32_sem_locker_t()
	{
		const char *lockerName = "_etk_global_";
		if((iLocker = OpenMutex(MUTEX_ALL_ACCESS, FALSE, lockerName)) == NULL)
			iLocker = CreateMutex(NULL, FALSE, lockerName);
		if(iLocker == NULL) ETK_ERROR("[KERNEL]: Can't initialize global semaphore!");
	}

	~etk_win32_sem_locker_t()
	{
		if(iLocker) CloseHandle(iLocker);
	}

	void Lock() {WaitForSingleObject(iLocker, INFINITE);}
	void Unlock() {ReleaseMutex(iLocker);}

	HANDLE iLocker;
};

static etk_win32_sem_locker_t __etk_semaphore_locker__;

static void _ETK_LOCK_SEMAPHORE_()
{
	__etk_semaphore_locker__.Lock();
}

static void _ETK_UNLOCK_SEMAPHORE_()
{
	__etk_semaphore_locker__.Unlock();
}


// return value must be free by "free()"
static char* etk_sem_locker_ipc_name(const char *name)
{
	if(name == NULL || *name == 0 || strlen(name) > E_OS_NAME_LENGTH) return NULL;

	const char *prefix = "__etk_";

	return e_strdup_printf("%s%s%s", prefix, "_sem_l_", name);
}


// return value must be free by "free()"
static char* etk_sem_event_ipc_name(const char *name)
{
	if(name == NULL || *name == 0 || strlen(name) > E_OS_NAME_LENGTH) return NULL;

	const char *prefix = "__etk_";

	return e_strdup_printf("%s%s%s", prefix, "_sem_e_", name);
}


static bool etk_is_sem_for_IPC(const etk_win32_sem_t *sem)
{
	if(!sem) return false;
	return(sem->mapping != NULL);
}


static void etk_lock_sem_inter(etk_win32_sem_t *sem)
{
	WaitForSingleObject(sem->Mutex, INFINITE);
}


static void etk_unlock_sem_inter(etk_win32_sem_t *sem)
{
	ReleaseMutex(sem->Mutex);
}


static void* etk_create_sem_for_IPC(eint64 count, const char *name, etk_area_access area_access)
{
	if(count < E_INT64_CONSTANT(0) || name == NULL || *name == 0 || strlen(name) > E_OS_NAME_LENGTH) return NULL;

	etk_win32_sem_t *sem = new etk_win32_sem_t();
	if(!sem) return NULL;

	char *locker_ipc_name = etk_sem_locker_ipc_name(name);
	char *event_ipc_name = etk_sem_event_ipc_name(name);

	if(!locker_ipc_name || !event_ipc_name)
	{
		if(locker_ipc_name) free(locker_ipc_name);
		if(event_ipc_name) free(event_ipc_name);
		delete sem;
		return NULL;
	}

	_ETK_LOCK_SEMAPHORE_();

	if((sem->mapping = etk_create_area(name, (void**)&(sem->semInfo), sizeof(etk_win32_sem_info),
					   E_READ_AREA | E_WRITE_AREA, ETK_AREA_SYSTEM_SEMAPHORE_DOMAIN, area_access)) == NULL ||
	   sem->semInfo == NULL)
	{
//		ETK_DEBUG("[KERNEL]: %s --- Can't create sem : create area failed.", __PRETTY_FUNCTION__, name);
		if(sem->mapping) etk_delete_area(sem->mapping);
		_ETK_UNLOCK_SEMAPHORE_();
		free(locker_ipc_name);
		free(event_ipc_name);
		delete sem;
		return NULL;
	}

	etk_win32_sem_info *sem_info = sem->semInfo;
	sem_info->InitData();
	memcpy(sem_info->name, name, (size_t)strlen(name));

	if((sem->Mutex = CreateMutex(NULL, FALSE, locker_ipc_name)) == NULL)
	{
		etk_delete_area(sem->mapping);
		_ETK_UNLOCK_SEMAPHORE_();
		free(locker_ipc_name);
		free(event_ipc_name);
		delete sem;
		return NULL;
	}
	if((sem->Event = CreateEvent(NULL, FALSE, FALSE, event_ipc_name)) == NULL)
	{
		CloseHandle(sem->Mutex);
		etk_delete_area(sem->mapping);
		_ETK_UNLOCK_SEMAPHORE_();
		free(locker_ipc_name);
		free(event_ipc_name);
		delete sem;
		return NULL;
	}

	free(locker_ipc_name);
	free(event_ipc_name);

	sem->semInfo->count = count;
	sem->semInfo->refCount = 1;

	_ETK_UNLOCK_SEMAPHORE_();

	sem->created = true;

//	ETK_DEBUG("[KERNEL]: %s --- SEMAPHORE [%s] created.", __PRETTY_FUNCTION__, name);

	return (void*)sem;
}


_IMPEXP_ETK void* etk_clone_sem(const char *name)
{
	if(name == NULL || *name == 0 || strlen(name) > E_OS_NAME_LENGTH) return NULL;

	etk_win32_sem_t *sem = new etk_win32_sem_t();
	if(!sem) return NULL;

	char *locker_ipc_name = etk_sem_locker_ipc_name(name);
	char *event_ipc_name = etk_sem_event_ipc_name(name);

	if(!locker_ipc_name || !event_ipc_name)
	{
		if(locker_ipc_name) free(locker_ipc_name);
		if(event_ipc_name) free(event_ipc_name);
		delete sem;
		return NULL;
	}

	_ETK_LOCK_SEMAPHORE_();

	if((sem->mapping = etk_clone_area(name, (void**)&(sem->semInfo),
					  E_READ_AREA | E_WRITE_AREA, ETK_AREA_SYSTEM_SEMAPHORE_DOMAIN)) == NULL ||
	   sem->semInfo == NULL || sem->semInfo->refCount >= E_MAXUINT32 || sem->semInfo->refCount == 0)
	{
//		ETK_DEBUG("[KERNEL]: %s --- Can't clone semaphore : clone area failed --- \"%s\"", __PRETTY_FUNCTION__, name);
		if(sem->mapping) etk_delete_area(sem->mapping);
		_ETK_UNLOCK_SEMAPHORE_();
		free(locker_ipc_name);
		free(event_ipc_name);
		delete sem;
		return NULL;
	}

	if((sem->Mutex = OpenMutex(MUTEX_ALL_ACCESS, FALSE, locker_ipc_name)) == NULL)
	{
		etk_delete_area(sem->mapping);
		_ETK_UNLOCK_SEMAPHORE_();
		free(locker_ipc_name);
		free(event_ipc_name);
		delete sem;
		return NULL;
	}
	if((sem->Event = OpenEvent(EVENT_ALL_ACCESS, FALSE, event_ipc_name)) == NULL)
	{
		CloseHandle(sem->Mutex);
		etk_delete_area(sem->mapping);
		_ETK_UNLOCK_SEMAPHORE_();
		free(locker_ipc_name);
		free(event_ipc_name);
		delete sem;
		return NULL;
	}

	free(locker_ipc_name);
	free(event_ipc_name);

	sem->semInfo->refCount += 1;

	_ETK_UNLOCK_SEMAPHORE_();

	sem->created = true;

	return (void*)sem;
}


_IMPEXP_ETK void* etk_clone_sem_by_source(void *data)
{
	etk_win32_sem_t *sem = (etk_win32_sem_t*)data;
	if(!sem || !sem->semInfo) return NULL;

	_ETK_LOCK_SEMAPHORE_();

	if(etk_is_sem_for_IPC(sem))
	{
		_ETK_UNLOCK_SEMAPHORE_();
		return etk_clone_sem(sem->semInfo->name);
	}
	else if(sem->no_clone || sem->semInfo->refCount >= E_MAXUINT32 || sem->semInfo->refCount == 0)
	{
		_ETK_UNLOCK_SEMAPHORE_();
		return NULL;
	}

	sem->semInfo->refCount += 1;

	_ETK_UNLOCK_SEMAPHORE_();

	return data;
}


static void* etk_create_sem_for_local(eint64 count)
{
	if(count < E_INT64_CONSTANT(0)) return NULL;

	etk_win32_sem_t *sem = new etk_win32_sem_t();

	if(!sem) return NULL;

	if((sem->semInfo = new etk_win32_sem_info()) == NULL ||
	   (sem->Mutex = CreateMutex(NULL, FALSE, NULL)) == NULL ||
	   (sem->Event = CreateEvent(NULL, FALSE, FALSE, NULL)) == NULL)
	{
		if(sem->Event) CloseHandle(sem->Event);
		if(sem->Mutex) CloseHandle(sem->Mutex);
		if(sem->semInfo) delete sem->semInfo;
		delete sem;
		return NULL;
	}

	sem->semInfo->count = count;
	sem->semInfo->refCount = 1;
	sem->created = true;

	return (void*)sem;
}


_IMPEXP_ETK void* etk_create_sem(eint64 count, const char *name, etk_area_access area_access)
{
	return((name == NULL || *name == 0) ?
			etk_create_sem_for_local(count) :
			etk_create_sem_for_IPC(count, name, area_access));
}


_IMPEXP_ETK e_status_t etk_get_sem_info(void *data, etk_sem_info *info)
{
	etk_win32_sem_t *sem = (etk_win32_sem_t*)data;
	if(!sem || !info) return E_BAD_VALUE;

	bzero(info->name, E_OS_NAME_LENGTH + 1);

	etk_lock_sem_inter(sem);

	if(etk_is_sem_for_IPC(sem)) strcpy(info->name, sem->semInfo->name);
	info->latest_holder_team = sem->semInfo->latestHolderTeamId;
	info->latest_holder_thread = sem->semInfo->latestHolderThreadId;
	info->count = sem->semInfo->count - sem->semInfo->acquiringCount;
	info->closed = sem->semInfo->closed;

	etk_unlock_sem_inter(sem);

	return E_OK;
}


_IMPEXP_ETK e_status_t etk_delete_sem(void *data)
{
	etk_win32_sem_t *sem = (etk_win32_sem_t*)data;
	if(!sem || !sem->semInfo) return E_BAD_VALUE;

	_ETK_LOCK_SEMAPHORE_();
	if(sem->semInfo->refCount == 0)
	{
		_ETK_UNLOCK_SEMAPHORE_();
		return E_ERROR;
	}
	euint32 count = --(sem->semInfo->refCount);
	_ETK_UNLOCK_SEMAPHORE_();

	if(etk_is_sem_for_IPC(sem))
	{
//		ETK_DEBUG("[KERNEL]: %s --- sem [%s] deleting...", __PRETTY_FUNCTION__, sem->semInfo->name);
		CloseHandle(sem->Mutex);
		CloseHandle(sem->Event);
		etk_delete_area(sem->mapping);
	}
	else
	{
		if(count > 0) return E_OK;

		CloseHandle(sem->Mutex);
		CloseHandle(sem->Event);
		delete sem->semInfo;
	}

	if(sem->created)
	{
		sem->created = false;
		delete sem;
	}

	return E_OK;
}


_IMPEXP_ETK e_status_t etk_delete_sem_etc(void *data, bool no_clone)
{
	etk_win32_sem_t *sem = (etk_win32_sem_t*)data;
	if(!sem || !sem->semInfo) return E_BAD_VALUE;

	_ETK_LOCK_SEMAPHORE_();
	if(sem->semInfo->refCount == 0)
	{
		_ETK_UNLOCK_SEMAPHORE_();
		return E_ERROR;
	}
	if(!etk_is_sem_for_IPC(sem) && no_clone) sem->no_clone = true;
	euint32 count = --(sem->semInfo->refCount);
	_ETK_UNLOCK_SEMAPHORE_();

	if(etk_is_sem_for_IPC(sem))
	{
		CloseHandle(sem->Mutex);
		CloseHandle(sem->Event);
		etk_delete_area_etc(sem->mapping, no_clone);
	}
	else
	{
		if(count > 0) return E_OK;

		CloseHandle(sem->Mutex);
		CloseHandle(sem->Event);
		delete sem->semInfo;
	}

	if(sem->created)
	{
		sem->created = false;
		delete sem;
	}

	return E_OK;
}


_IMPEXP_ETK e_status_t etk_close_sem(void *data)
{
	etk_win32_sem_t *sem = (etk_win32_sem_t*)data;
	if(!sem) return E_BAD_VALUE;

	etk_lock_sem_inter(sem);

	if(sem->semInfo->closed)
	{
		etk_unlock_sem_inter(sem);
		return E_ERROR;
	}
	sem->semInfo->closed = true;

	SetEvent(sem->Event);

	etk_unlock_sem_inter(sem);

	return E_OK;
}


_IMPEXP_ETK e_status_t etk_acquire_sem_etc(void *data, eint64 count, euint32 flags, e_bigtime_t microseconds_timeout)
{
	etk_win32_sem_t *sem = (etk_win32_sem_t*)data;
	if(!sem) return E_BAD_VALUE;

	if(microseconds_timeout < E_INT64_CONSTANT(0) || count < E_INT64_CONSTANT(1)) return E_BAD_VALUE;

	e_bigtime_t currentTime = etk_real_time_clock_usecs();
	bool wait_forever = false;

	if(flags != E_ABSOLUTE_TIMEOUT)
	{
		if(microseconds_timeout == E_INFINITE_TIMEOUT || microseconds_timeout > E_MAXINT64 - currentTime)
			wait_forever = true;
		else
			microseconds_timeout += currentTime;
	}

	etk_lock_sem_inter(sem);

	if(sem->semInfo->count - count >= E_INT64_CONSTANT(0))
	{
		sem->semInfo->count -= count;
		sem->semInfo->SetLatestHolderTeamId(etk_get_current_team_id());
		sem->semInfo->SetLatestHolderThreadId(etk_get_current_thread_id());
		etk_unlock_sem_inter(sem);
		return E_OK;
	}
	else if(sem->semInfo->closed)
	{
		etk_unlock_sem_inter(sem);
		return E_ERROR;
	}
	else if(microseconds_timeout == currentTime && !wait_forever)
	{
		etk_unlock_sem_inter(sem);
		return E_WOULD_BLOCK;
	}
	if(count > E_MAXINT64 - sem->semInfo->acquiringCount)
	{
		etk_unlock_sem_inter(sem);
		return E_ERROR;
	}

	HANDLE handles[2] = {NULL, NULL};
	handles[0] = sem->Event;
	if(!wait_forever)
	{
		HANDLE timer = NULL;
		LARGE_INTEGER due;
		due.QuadPart = microseconds_timeout * E_INT64_CONSTANT(10) + SECS_BETWEEN_EPOCHS * SECS_TO_100NS;
		timer = CreateWaitableTimer(NULL, TRUE, NULL);

		if(!timer || SetWaitableTimer(timer, &due, 0, NULL, NULL, 0) == 0)
		{
			if(timer) CloseHandle(timer);
			etk_unlock_sem_inter(sem);
			return E_ERROR;
		}

		handles[1] = timer;
	}

	sem->semInfo->acquiringCount += count;
	if(sem->semInfo->minAcquiringCount == E_INT64_CONSTANT(0) ||
	   sem->semInfo->minAcquiringCount > count) sem->semInfo->minAcquiringCount = count;

	e_status_t retval = E_ERROR;

	while(true)
	{
		etk_unlock_sem_inter(sem);
		DWORD status = (handles[1] == NULL ?
					WaitForSingleObject(handles[0], INFINITE) :
					WaitForMultipleObjects(2, handles, FALSE, INFINITE));
		etk_lock_sem_inter(sem);

		if(status - WAIT_OBJECT_0 == 1 || status == WAIT_TIMEOUT)
		{
			retval = E_TIMED_OUT;
			break;
		}
		else if(status != WAIT_OBJECT_0)
		{
			break;
		}

		if(sem->semInfo->count - count >= E_INT64_CONSTANT(0))
		{
			sem->semInfo->count -= count;
			sem->semInfo->SetLatestHolderTeamId(etk_get_current_team_id());
			sem->semInfo->SetLatestHolderThreadId(etk_get_current_thread_id());
			retval = E_OK;
			break;
		}
		else if(sem->semInfo->closed)
		{
			break;
		}

		if(sem->semInfo->minAcquiringCount > sem->semInfo->count) continue;
		if(sem->semInfo->minAcquiringCount == E_INT64_CONSTANT(0) ||
		   sem->semInfo->minAcquiringCount > count) sem->semInfo->minAcquiringCount = count;

		SetEvent(sem->Event);
	}

	sem->semInfo->acquiringCount -= count;

	if(sem->semInfo->minAcquiringCount == count) sem->semInfo->minAcquiringCount = E_INT64_CONSTANT(0);
	SetEvent(sem->Event);

	etk_unlock_sem_inter(sem);

	if(handles[1] != NULL) CloseHandle(handles[1]);

	return retval;
}


_IMPEXP_ETK e_status_t etk_acquire_sem(void *data)
{
	return etk_acquire_sem_etc(data, E_INT64_CONSTANT(1), E_TIMEOUT, E_INFINITE_TIMEOUT);
}


_IMPEXP_ETK e_status_t etk_release_sem_etc(void *data, eint64 count, euint32 flags)
{
	etk_win32_sem_t *sem = (etk_win32_sem_t*)data;
	if(!sem || count < E_INT64_CONSTANT(0)) return E_BAD_VALUE;

	etk_lock_sem_inter(sem);

	e_status_t retval = E_ERROR;

	if(sem->semInfo->closed == false && (E_MAXINT64 - sem->semInfo->count >= count))
	{
		sem->semInfo->count += count;
		if(flags != E_DO_NOT_RESCHEDULE && sem->semInfo->acquiringCount > E_INT64_CONSTANT(0)) SetEvent(sem->Event);
		retval = E_OK;
	}

	etk_unlock_sem_inter(sem);

	return retval;
}


_IMPEXP_ETK e_status_t etk_release_sem(void *data)
{
	return etk_release_sem_etc(data, E_INT64_CONSTANT(1), 0);
}


_IMPEXP_ETK e_status_t etk_get_sem_count(void *data, eint64 *count)
{
	etk_win32_sem_t *sem = (etk_win32_sem_t*)data;
	if(!sem || !count) return E_BAD_VALUE;

	etk_lock_sem_inter(sem);
	*count = (sem->semInfo->acquiringCount <= E_INT64_CONSTANT(0) ?
			sem->semInfo->count : E_INT64_CONSTANT(-1) * (sem->semInfo->acquiringCount));
	etk_unlock_sem_inter(sem);

	return E_OK;
}

