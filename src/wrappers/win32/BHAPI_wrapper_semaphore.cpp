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
 * File: BHAPI_wrapper_semaphore.cpp
 *
 * --------------------------------------------------------------------------*/

#include <os/kernel.h>
#include <os/debug.h>
#include <kits/support/String.h>
#include <kits/debug/Errors.h>

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0500
#endif
#include <winsock2.h>
#include <windows.h>


#define SECS_BETWEEN_EPOCHS    B_INT64_CONSTANT(11644473600)
#define SECS_TO_100NS	    B_INT64_CONSTANT(10000000)

typedef struct bhapi::win32_sem_info {
	bhapi::win32_sem_info()
	{
		InitData();
	}

	void InitData()
	{
		bzero(name, B_OS_NAME_LENGTH + 1);
		latestHolderTeamId = B_INT64_CONSTANT(0);
		latestHolderThreadId = B_INT64_CONSTANT(0);
		count = B_INT64_CONSTANT(0);
		minAcquiringCount = B_INT64_CONSTANT(0);
		acquiringCount = B_INT64_CONSTANT(0);
		closed = false;
		refCount = 0;
	}

	void SetLatestHolderTeamId(int64 id)
	{
		latestHolderTeamId = id;
	}

	void SetLatestHolderThreadId(int64 id)
	{
		latestHolderThreadId = id;
	}

	bool LatestHolderTeamIsCurrent(void)
	{
		return(latestHolderTeamId == bhapi::get_current_team_id());
	}

	bool LatestHolderThreadIsCurrent(void)
	{
		return(latestHolderThreadId == bhapi::get_current_thread_id());
	}

	char			name[B_OS_NAME_LENGTH + 1];
	int64			latestHolderTeamId;
	int64			latestHolderThreadId;
	int64			count;
	int64			minAcquiringCount;
	int64			acquiringCount;
	bool			closed;

	uint32			refCount;
} bhapi::win32_sem_info;


typedef struct bhapi::win32_sem_t {
	bhapi::win32_sem_t()
		: mapping(NULL), semInfo(NULL),
		  Mutex(NULL), Event(NULL),
		  created(false), no_clone(false)
	{
	}

	~bhapi::win32_sem_t()
	{
		if(created)
		{
			created = false;
			bhapi::delete_sem((void*)this);
		}
	}

	void			*mapping;
	bhapi::win32_sem_info	*semInfo;

	HANDLE			Mutex;
	HANDLE			Event;

	bool			created;
	bool			no_clone;
} bhapi::win32_sem_t;


class bhapi::win32_sem_locker_t {
public:
	bhapi::win32_sem_locker_t()
	{
		const char *lockerName = "_bhapi_global_";
        if((iLocker = OpenMutexA(MUTEX_ALL_ACCESS, FALSE, lockerName)) == NULL)
            iLocker = CreateMutexA(NULL, FALSE, lockerName);
		if(iLocker == NULL) BHAPI_ERROR("[KERNEL]: Can't initialize global semaphore!");
	}

	~bhapi::win32_sem_locker_t()
	{
		if(iLocker) CloseHandle(iLocker);
	}

	void Lock() {WaitForSingleObject(iLocker, INFINITE);}
	void Unlock() {ReleaseMutex(iLocker);}

	HANDLE iLocker;
};

static bhapi::win32_sem_locker_t __bhapi_semaphore_locker__;

static void BHAPI_LOCK_SEMAPHORE()
{
	__bhapi_semaphore_locker__.Lock();
}

static void BHAPI_UNLOCK_SEMAPHORE()
{
	__bhapi_semaphore_locker__.Unlock();
}


// return value must be free by "free()"
static char* bhapi::sem_locker_ipc_name(const char *name)
{
	if(name == NULL || *name == 0 || strlen(name) > B_OS_NAME_LENGTH) return NULL;

	const char *prefix = "__bhapi_";

	return bhapi::strdup_printf("%s%s%s", prefix, "_sem_l_", name);
}


// return value must be free by "free()"
static char* bhapi::sem_event_ipc_name(const char *name)
{
	if(name == NULL || *name == 0 || strlen(name) > B_OS_NAME_LENGTH) return NULL;

	const char *prefix = "__bhapi_";

	return bhapi::strdup_printf("%s%s%s", prefix, "_sem_e_", name);
}


static bool bhapi::is_sem_for_IPC(const bhapi::win32_sem_t *sem)
{
	if(!sem) return false;
	return(sem->mapping != NULL);
}


static void bhapi::lock_sem_inter(bhapi::win32_sem_t *sem)
{
	WaitForSingleObject(sem->Mutex, INFINITE);
}


static void bhapi::unlock_sem_inter(bhapi::win32_sem_t *sem)
{
	ReleaseMutex(sem->Mutex);
}


static void* bhapi::create_sem_for_IPC(int64 count, const char *name, bhapi::area_access area_access)
{
	if(count < B_INT64_CONSTANT(0) || name == NULL || *name == 0 || strlen(name) > B_OS_NAME_LENGTH) return NULL;

	bhapi::win32_sem_t *sem = new bhapi::win32_sem_t();
	if(!sem) return NULL;

	char *locker_ipc_name = bhapi::sem_locker_ipc_name(name);
	char *event_ipc_name = bhapi::sem_event_ipc_name(name);

	if(!locker_ipc_name || !event_ipc_name)
	{
		if(locker_ipc_name) free(locker_ipc_name);
		if(event_ipc_name) free(event_ipc_name);
		delete sem;
		return NULL;
	}

	BHAPI_LOCK_SEMAPHORE();

	if((sem->mapping = bhapi::create_area(name, (void**)&(sem->semInfo), sizeof(bhapi::win32_sem_info),
					   B_READ_AREA | B_WRITE_AREA, BHAPI_AREA_SYSTEM_SEMAPHORE_DOMAIN, area_access)) == NULL ||
	   sem->semInfo == NULL)
	{
//		BHAPI_DEBUG("[KERNEL]: %s --- Can't create sem : create area failed.", __PRETTY_FUNCTION__, name);
		if(sem->mapping) bhapi::delete_area(sem->mapping);
		BHAPI_UNLOCK_SEMAPHORE();
		free(locker_ipc_name);
		free(event_ipc_name);
		delete sem;
		return NULL;
	}

	bhapi::win32_sem_info *sem_info = sem->semInfo;
	sem_info->InitData();
	memcpy(sem_info->name, name, (size_t)strlen(name));

    if((sem->Mutex = CreateMutexA(NULL, FALSE, locker_ipc_name)) == NULL)
	{
		bhapi::delete_area(sem->mapping);
		BHAPI_UNLOCK_SEMAPHORE();
		free(locker_ipc_name);
		free(event_ipc_name);
		delete sem;
		return NULL;
	}
    if((sem->Event = CreateEventA(NULL, FALSE, FALSE, event_ipc_name)) == NULL)
	{
		CloseHandle(sem->Mutex);
		bhapi::delete_area(sem->mapping);
		BHAPI_UNLOCK_SEMAPHORE();
		free(locker_ipc_name);
		free(event_ipc_name);
		delete sem;
		return NULL;
	}

	free(locker_ipc_name);
	free(event_ipc_name);

	sem->semInfo->count = count;
	sem->semInfo->refCount = 1;

	BHAPI_UNLOCK_SEMAPHORE();

	sem->created = true;

//	BHAPI_DEBUG("[KERNEL]: %s --- SEMAPHORE [%s] created.", __PRETTY_FUNCTION__, name);

	return (void*)sem;
}


BHAPI_EXPORT void* bhapi::clone_sem(const char *name)
{
	if(name == NULL || *name == 0 || strlen(name) > B_OS_NAME_LENGTH) return NULL;

	bhapi::win32_sem_t *sem = new bhapi::win32_sem_t();
	if(!sem) return NULL;

	char *locker_ipc_name = bhapi::sem_locker_ipc_name(name);
	char *event_ipc_name = bhapi::sem_event_ipc_name(name);

	if(!locker_ipc_name || !event_ipc_name)
	{
		if(locker_ipc_name) free(locker_ipc_name);
		if(event_ipc_name) free(event_ipc_name);
		delete sem;
		return NULL;
	}

	BHAPI_LOCK_SEMAPHORE();

	if((sem->mapping = bhapi::clone_area(name, (void**)&(sem->semInfo),
					  B_READ_AREA | B_WRITE_AREA, BHAPI_AREA_SYSTEM_SEMAPHORE_DOMAIN)) == NULL ||
	   sem->semInfo == NULL || sem->semInfo->refCount >= B_MAXUINT32 || sem->semInfo->refCount == 0)
	{
//		BHAPI_DEBUG("[KERNEL]: %s --- Can't clone semaphore : clone area failed --- \"%s\"", __PRETTY_FUNCTION__, name);
		if(sem->mapping) bhapi::delete_area(sem->mapping);
		BHAPI_UNLOCK_SEMAPHORE();
		free(locker_ipc_name);
		free(event_ipc_name);
		delete sem;
		return NULL;
	}

    if((sem->Mutex = OpenMutexA(MUTEX_ALL_ACCESS, FALSE, locker_ipc_name)) == NULL)
	{
		bhapi::delete_area(sem->mapping);
		BHAPI_UNLOCK_SEMAPHORE();
		free(locker_ipc_name);
		free(event_ipc_name);
		delete sem;
		return NULL;
	}
    if((sem->Event = OpenEventA(EVENT_ALL_ACCESS, FALSE, event_ipc_name)) == NULL)
	{
		CloseHandle(sem->Mutex);
		bhapi::delete_area(sem->mapping);
		BHAPI_UNLOCK_SEMAPHORE();
		free(locker_ipc_name);
		free(event_ipc_name);
		delete sem;
		return NULL;
	}

	free(locker_ipc_name);
	free(event_ipc_name);

	sem->semInfo->refCount += 1;

	BHAPI_UNLOCK_SEMAPHORE();

	sem->created = true;

	return (void*)sem;
}


BHAPI_EXPORT void* bhapi::clone_sem_by_source(void *data)
{
	bhapi::win32_sem_t *sem = (bhapi::win32_sem_t*)data;
	if(!sem || !sem->semInfo) return NULL;

	BHAPI_LOCK_SEMAPHORE();

	if(bhapi::is_sem_for_IPC(sem))
	{
		BHAPI_UNLOCK_SEMAPHORE();
		return bhapi::clone_sem(sem->semInfo->name);
	}
	else if(sem->no_clone || sem->semInfo->refCount >= B_MAXUINT32 || sem->semInfo->refCount == 0)
	{
		BHAPI_UNLOCK_SEMAPHORE();
		return NULL;
	}

	sem->semInfo->refCount += 1;

	BHAPI_UNLOCK_SEMAPHORE();

	return data;
}


static void* bhapi::create_sem_for_local(int64 count)
{
	if(count < B_INT64_CONSTANT(0)) return NULL;

	bhapi::win32_sem_t *sem = new bhapi::win32_sem_t();

	if(!sem) return NULL;

	if((sem->semInfo = new bhapi::win32_sem_info()) == NULL ||
       (sem->Mutex = CreateMutexA(NULL, FALSE, NULL)) == NULL ||
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


BHAPI_EXPORT void* bhapi::create_sem(int64 count, const char *name, bhapi::area_access area_access)
{
	return((name == NULL || *name == 0) ?
			bhapi::create_sem_for_local(count) :
			bhapi::create_sem_for_IPC(count, name, area_access));
}


BHAPI_EXPORT status_t bhapi::get_sem_info(void *data, bhapi::sem_info *info)
{
	bhapi::win32_sem_t *sem = (bhapi::win32_sem_t*)data;
	if(!sem || !info) return B_BAD_VALUE;

	bzero(info->name, B_OS_NAME_LENGTH + 1);

	bhapi::lock_sem_inter(sem);

	if(bhapi::is_sem_for_IPC(sem)) strcpy(info->name, sem->semInfo->name);
	info->latest_holder_team = sem->semInfo->latestHolderTeamId;
	info->latest_holder_thread = sem->semInfo->latestHolderThreadId;
	info->count = sem->semInfo->count - sem->semInfo->acquiringCount;
	info->closed = sem->semInfo->closed;

	bhapi::unlock_sem_inter(sem);

	return B_OK;
}


BHAPI_EXPORT status_t bhapi::delete_sem(void *data)
{
	bhapi::win32_sem_t *sem = (bhapi::win32_sem_t*)data;
	if(!sem || !sem->semInfo) return B_BAD_VALUE;

	BHAPI_LOCK_SEMAPHORE();
	if(sem->semInfo->refCount == 0)
	{
		BHAPI_UNLOCK_SEMAPHORE();
		return B_ERROR;
	}
	uint32 count = --(sem->semInfo->refCount);
	BHAPI_UNLOCK_SEMAPHORE();

	if(bhapi::is_sem_for_IPC(sem))
	{
//		BHAPI_DEBUG("[KERNEL]: %s --- sem [%s] deleting...", __PRETTY_FUNCTION__, sem->semInfo->name);
		CloseHandle(sem->Mutex);
		CloseHandle(sem->Event);
		bhapi::delete_area(sem->mapping);
	}
	else
	{
		if(count > 0) return B_OK;

		CloseHandle(sem->Mutex);
		CloseHandle(sem->Event);
		delete sem->semInfo;
	}

	if(sem->created)
	{
		sem->created = false;
		delete sem;
	}

	return B_OK;
}


BHAPI_EXPORT status_t bhapi::delete_sem_etc(void *data, bool no_clone)
{
	bhapi::win32_sem_t *sem = (bhapi::win32_sem_t*)data;
	if(!sem || !sem->semInfo) return B_BAD_VALUE;

	BHAPI_LOCK_SEMAPHORE();
	if(sem->semInfo->refCount == 0)
	{
		BHAPI_UNLOCK_SEMAPHORE();
		return B_ERROR;
	}
	if(!bhapi::is_sem_for_IPC(sem) && no_clone) sem->no_clone = true;
	uint32 count = --(sem->semInfo->refCount);
	BHAPI_UNLOCK_SEMAPHORE();

	if(bhapi::is_sem_for_IPC(sem))
	{
		CloseHandle(sem->Mutex);
		CloseHandle(sem->Event);
		bhapi::delete_area_etc(sem->mapping, no_clone);
	}
	else
	{
		if(count > 0) return B_OK;

		CloseHandle(sem->Mutex);
		CloseHandle(sem->Event);
		delete sem->semInfo;
	}

	if(sem->created)
	{
		sem->created = false;
		delete sem;
	}

	return B_OK;
}


BHAPI_EXPORT status_t bhapi::close_sem(void *data)
{
	bhapi::win32_sem_t *sem = (bhapi::win32_sem_t*)data;
	if(!sem) return B_BAD_VALUE;

	bhapi::lock_sem_inter(sem);

	if(sem->semInfo->closed)
	{
		bhapi::unlock_sem_inter(sem);
		return B_ERROR;
	}
	sem->semInfo->closed = true;

	SetEvent(sem->Event);

	bhapi::unlock_sem_inter(sem);

	return B_OK;
}


BHAPI_EXPORT status_t bhapi::acquire_sem_etc(void *data,  int64 count,  uint32 flags, bigtime_t microseconds_timeout)
{
	bhapi::win32_sem_t *sem = (bhapi::win32_sem_t*)data;
	if(!sem) return B_BAD_VALUE;

	if(microseconds_timeout < B_INT64_CONSTANT(0) || count < B_INT64_CONSTANT(1)) return B_BAD_VALUE;

	bigtime_t currentTime = b_real_time_clock_usecs();
	bool wait_forever = false;

	if(flags != B_ABSOLUTE_TIMEOUT)
	{
		if(microseconds_timeout == B_INFINITE_TIMEOUT || microseconds_timeout > B_MAXINT64 - currentTime)
			wait_forever = true;
		else
			microseconds_timeout += currentTime;
	}

	bhapi::lock_sem_inter(sem);

	if(sem->semInfo->count - count >= B_INT64_CONSTANT(0))
	{
		sem->semInfo->count -= count;
		sem->semInfo->SetLatestHolderTeamId(bhapi::get_current_team_id());
		sem->semInfo->SetLatestHolderThreadId(bhapi::get_current_thread_id());
		bhapi::unlock_sem_inter(sem);
		return B_OK;
	}
	else if(sem->semInfo->closed)
	{
		bhapi::unlock_sem_inter(sem);
		return B_ERROR;
	}
	else if(microseconds_timeout == currentTime && !wait_forever)
	{
		bhapi::unlock_sem_inter(sem);
		return B_WOULD_BLOCK;
	}
	if(count > B_MAXINT64 - sem->semInfo->acquiringCount)
	{
		bhapi::unlock_sem_inter(sem);
		return B_ERROR;
	}

	HANDLE handles[2] = {NULL, NULL};
	handles[0] = sem->Event;
	if(!wait_forever)
	{
		HANDLE timer = NULL;
		LARGE_INTEGER due;
		due.QuadPart = microseconds_timeout * B_INT64_CONSTANT(10) + SECS_BETWEEN_EPOCHS * SECS_TO_100NS;
		timer = CreateWaitableTimer(NULL, TRUE, NULL);

		if(!timer || SetWaitableTimer(timer, &due, 0, NULL, NULL, 0) == 0)
		{
			if(timer) CloseHandle(timer);
			bhapi::unlock_sem_inter(sem);
			return B_ERROR;
		}

		handles[1] = timer;
	}

	sem->semInfo->acquiringCount += count;
	if(sem->semInfo->minAcquiringCount == B_INT64_CONSTANT(0) ||
	   sem->semInfo->minAcquiringCount > count) sem->semInfo->minAcquiringCount = count;

	status_t retval = B_ERROR;

	while(true)
	{
		bhapi::unlock_sem_inter(sem);
		DWORD status = (handles[1] == NULL ?
					WaitForSingleObject(handles[0], INFINITE) :
					WaitForMultipleObjects(2, handles, FALSE, INFINITE));
		bhapi::lock_sem_inter(sem);

		if(status - WAIT_OBJECT_0 == 1 || status == WAIT_TIMEOUT)
		{
			retval = B_TIMED_OUT;
			break;
		}
		else if(status != WAIT_OBJECT_0)
		{
			break;
		}

		if(sem->semInfo->count - count >= B_INT64_CONSTANT(0))
		{
			sem->semInfo->count -= count;
			sem->semInfo->SetLatestHolderTeamId(bhapi::get_current_team_id());
			sem->semInfo->SetLatestHolderThreadId(bhapi::get_current_thread_id());
			retval = B_OK;
			break;
		}
		else if(sem->semInfo->closed)
		{
			break;
		}

		if(sem->semInfo->minAcquiringCount > sem->semInfo->count) continue;
		if(sem->semInfo->minAcquiringCount == B_INT64_CONSTANT(0) ||
		   sem->semInfo->minAcquiringCount > count) sem->semInfo->minAcquiringCount = count;

		SetEvent(sem->Event);
	}

	sem->semInfo->acquiringCount -= count;

	if(sem->semInfo->minAcquiringCount == count) sem->semInfo->minAcquiringCount = B_INT64_CONSTANT(0);
	SetEvent(sem->Event);

	bhapi::unlock_sem_inter(sem);

	if(handles[1] != NULL) CloseHandle(handles[1]);

	return retval;
}


BHAPI_EXPORT status_t bhapi::acquire_sem(void *data)
{
	return bhapi::acquire_sem_etc(data, B_INT64_CONSTANT(1), B_TIMEOUT, B_INFINITE_TIMEOUT);
}


BHAPI_EXPORT status_t bhapi::release_sem_etc(void *data,  int64 count,  uint32 flags)
{
	bhapi::win32_sem_t *sem = (bhapi::win32_sem_t*)data;
	if(!sem || count < B_INT64_CONSTANT(0)) return B_BAD_VALUE;

	bhapi::lock_sem_inter(sem);

	status_t retval = B_ERROR;

	if(sem->semInfo->closed == false && (B_MAXINT64 - sem->semInfo->count >= count))
	{
		sem->semInfo->count += count;
		if(flags != B_DO_NOT_RESCHEDULE && sem->semInfo->acquiringCount > B_INT64_CONSTANT(0)) SetEvent(sem->Event);
		retval = B_OK;
	}

	bhapi::unlock_sem_inter(sem);

	return retval;
}


BHAPI_EXPORT status_t bhapi::release_sem(void *data)
{
	return bhapi::release_sem_etc(data, B_INT64_CONSTANT(1), 0);
}


BHAPI_EXPORT status_t bhapi::get_sem_count(void *data,  int64 *count)
{
	bhapi::win32_sem_t *sem = (bhapi::win32_sem_t*)data;
	if(!sem || !count) return B_BAD_VALUE;

	bhapi::lock_sem_inter(sem);
	*count = (sem->semInfo->acquiringCount <= B_INT64_CONSTANT(0) ?
			sem->semInfo->count : B_INT64_CONSTANT(-1) * (sem->semInfo->acquiringCount));
	bhapi::unlock_sem_inter(sem);

	return B_OK;
}

