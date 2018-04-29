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
 * File: BHAPI_wrapper_locker.cpp
 *
 * --------------------------------------------------------------------------*/

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0500
#endif

#include <winsock2.h>
#include <windows.h>

#include <os/kernel.h>
#include <kits/debug/Errors.h>
#include <os/debug.h>

#define SECS_BETWEEN_EPOCHS    B_INT64_CONSTANT(11644473600)
#define SECS_TO_100NS	    B_INT64_CONSTANT(10000000)

typedef struct bhapi::win32_locker_t {
	bhapi::win32_locker_t()
		: holderThreadId(B_INT64_CONSTANT(0)), lockCount(B_INT64_CONSTANT(0)), closed(false), Locker(NULL), created(false), refCount(0)
	{
	}

	~bhapi::win32_locker_t()
	{
		if(created)
		{
			created = false;
			bhapi::delete_locker((void*)this);
		}
	}

	void SetHolderThreadId(int64 id)
	{
		holderThreadId = id;
	}

	bool HolderThreadIsCurrent(void)
	{
		return(holderThreadId == bhapi::get_current_thread_id());
	}

	int64			holderThreadId;
	int64			lockCount;
	bool			closed;

	CRITICAL_SECTION	iLocker;
	HANDLE			Locker;
	HANDLE			Cond;

	bool			created;

	uint32			refCount;
} bhapi::win32_locker_t;


static void bhapi::lock_locker_inter(bhapi::win32_locker_t *locker)
{
	EnterCriticalSection(&(locker->iLocker));
}


static void bhapi::unlock_locker_inter(bhapi::win32_locker_t *locker)
{
	LeaveCriticalSection(&(locker->iLocker));
}


void* bhapi::create_locker(void)
{
	bhapi::win32_locker_t *locker = new bhapi::win32_locker_t();

	if(!locker) return NULL;

    if((locker->Locker = CreateMutexA(NULL, FALSE, NULL)) == NULL)
	{
		delete locker;
		return NULL;
	}

	if((locker->Cond = CreateEvent(NULL, TRUE, FALSE, NULL)) == NULL)
	{
		CloseHandle(locker->Locker);
		delete locker;
		return NULL;
	}

	InitializeCriticalSection(&(locker->iLocker));

	locker->refCount = 1;
	locker->created = true;

	return (void*)locker;
}


BHAPI_EXPORT void* bhapi::clone_locker(void *data)
{
	bhapi::win32_locker_t *locker = (bhapi::win32_locker_t*)data;
	if(!locker) return NULL;

	bhapi::lock_locker_inter(locker);

	if(locker->closed || locker->refCount >= B_MAXUINT32 || locker->refCount == 0)
	{
		bhapi::unlock_locker_inter(locker);
		return NULL;
	}

	locker->refCount += 1;

	bhapi::unlock_locker_inter(locker);

	return data;
}


BHAPI_EXPORT status_t bhapi::delete_locker(void *data)
{
	bhapi::win32_locker_t *locker = (bhapi::win32_locker_t*)data;
	if(!locker) return B_BAD_VALUE;

	bhapi::lock_locker_inter(locker);
	if(locker->refCount == 0)
	{
		bhapi::unlock_locker_inter(locker);
		return B_ERROR;
	}
	uint32 count = --(locker->refCount);
#if 0
	bool locked = locker->HolderThreadIsCurrent();
#endif
	bhapi::unlock_locker_inter(locker);

#if 0
	if(locked && count > 0)
		BHAPI_DEBUG("\n\
**************************************************************************\n\
*                      [KERNEL]: bhapi::delete_locker                       *\n\
*                                                                        *\n\
*  Locker still locked by current thread, and some clone-copies existed  *\n\
*  It's recommended that unlock the locker before you delete it.         *\n\
*  Otherwise, the waitting thread will block!                            *\n\
**************************************************************************\n");
#endif

	if(count > 0) return B_OK;

	if(locker->HolderThreadIsCurrent()) ReleaseMutex(locker->Locker);
	CloseHandle(locker->Locker);
	CloseHandle(locker->Cond);
	DeleteCriticalSection(&(locker->iLocker));

	if(locker->created)
	{
		locker->created = false;
		delete locker;
	}

	return B_OK;
}


BHAPI_EXPORT status_t bhapi::close_locker(void *data)
{
	bhapi::win32_locker_t *locker = (bhapi::win32_locker_t*)data;
	if(!locker) return B_BAD_VALUE;

	bhapi::lock_locker_inter(locker);
	if(locker->closed)
	{
		bhapi::unlock_locker_inter(locker);
		return B_ERROR;
	}
	locker->closed = true;
	SetEvent(locker->Cond);
	bhapi::unlock_locker_inter(locker);

	return B_OK;
}


BHAPI_EXPORT status_t bhapi::lock_locker(void *data)
{
	bhapi::win32_locker_t *locker = (bhapi::win32_locker_t*)data;
	if(!locker) return B_BAD_VALUE;

	bhapi::lock_locker_inter(locker);

	if(locker->closed)
	{
		bhapi::unlock_locker_inter(locker);
		return B_ERROR;
	}

	if(locker->HolderThreadIsCurrent() == false)
	{
		HANDLE handles[2];
		handles[0] = locker->Locker;
		handles[1] = locker->Cond;

		bhapi::unlock_locker_inter(locker);

		if(WaitForMultipleObjects(2, handles, FALSE, INFINITE) != WAIT_OBJECT_0) return B_ERROR;

		bhapi::lock_locker_inter(locker);

		if(locker->closed)
		{
			ReleaseMutex(handles[0]);
			bhapi::unlock_locker_inter(locker);
			return B_ERROR;
		}

		locker->SetHolderThreadId(bhapi::get_current_thread_id());
		locker->lockCount = B_INT64_CONSTANT(1);

		bhapi::unlock_locker_inter(locker);
	}
	else
	{
		if(B_MAXINT64 - locker->lockCount < B_INT64_CONSTANT(1))
		{
			bhapi::unlock_locker_inter(locker);
			return B_ERROR;
		}

		locker->lockCount++;
		bhapi::unlock_locker_inter(locker);
	}

	return B_OK;
}


BHAPI_EXPORT status_t bhapi::lock_locker_etc(void *data,  uint32 flags, bigtime_t microseconds_timeout)
{
	bhapi::win32_locker_t *locker = (bhapi::win32_locker_t*)data;
	if(!locker) return B_BAD_VALUE;

	if(microseconds_timeout < B_INT64_CONSTANT(0)) return B_BAD_VALUE;

	bigtime_t currentTime = b_real_time_clock_usecs();
	if(flags != B_ABSOLUTE_TIMEOUT)
	{
		if(microseconds_timeout == B_INFINITE_TIMEOUT || microseconds_timeout > B_MAXINT64 - currentTime)
			return bhapi::lock_locker(data);
		else
			microseconds_timeout += currentTime;
	}

	bhapi::lock_locker_inter(locker);

	if(locker->closed)
	{
		bhapi::unlock_locker_inter(locker);
		return B_ERROR;
	}

	if(locker->HolderThreadIsCurrent() == false)
	{
		HANDLE mutex = locker->Locker;

		if(microseconds_timeout == currentTime)
		{
			bhapi::unlock_locker_inter(locker);

			if(WaitForSingleObject(mutex, 0L) != WAIT_OBJECT_0) return B_WOULD_BLOCK;
		}
		else
		{
			HANDLE timer = NULL;
			LARGE_INTEGER due;
			due.QuadPart = microseconds_timeout * B_INT64_CONSTANT(10) + SECS_BETWEEN_EPOCHS * SECS_TO_100NS;
			timer = CreateWaitableTimer(NULL, TRUE, NULL);
			if(timer == NULL)
			{
				bhapi::unlock_locker_inter(locker);
				return B_ERROR;
			}
			if(SetWaitableTimer(timer, &due, 0, NULL, NULL, 0) == 0)
			{
				CloseHandle(timer);
				bhapi::unlock_locker_inter(locker);
				return B_ERROR;
			}

			HANDLE handles[3];
			handles[0] = mutex;
			handles[1] = timer;
			handles[2] = locker->Cond;

			bhapi::unlock_locker_inter(locker);

			DWORD status = WaitForMultipleObjects(3, handles, FALSE, INFINITE);

			CloseHandle(timer);

			if(status - WAIT_OBJECT_0 == 1 || status == WAIT_TIMEOUT)
				return B_TIMED_OUT;
			else if(status != WAIT_OBJECT_0)
				return B_ERROR;
		}

		bhapi::lock_locker_inter(locker);

		if(locker->closed)
		{
			ReleaseMutex(mutex);
			bhapi::unlock_locker_inter(locker);
			return B_ERROR;
		}

		locker->SetHolderThreadId(bhapi::get_current_thread_id());
		locker->lockCount = B_INT64_CONSTANT(1);

		bhapi::unlock_locker_inter(locker);
	}
	else
	{
		if(B_MAXINT64 - locker->lockCount < B_INT64_CONSTANT(1))
		{
			bhapi::unlock_locker_inter(locker);
			return B_ERROR;
		}

		locker->lockCount++;
		bhapi::unlock_locker_inter(locker);
	}

	return B_OK;
}


BHAPI_EXPORT status_t bhapi::unlock_locker(void *data)
{
	bhapi::win32_locker_t *locker = (bhapi::win32_locker_t*)data;
	if(!locker) return B_BAD_VALUE;

	bhapi::lock_locker_inter(locker);

	if(locker->HolderThreadIsCurrent() == false)
	{
		bhapi::unlock_locker_inter(locker);
		BHAPI_ERROR("[KERNEL]: %s --- Can't unlock when didn't hold it in current thread!", __PRETTY_FUNCTION__);
		return B_ERROR;
	}
	else
	{
		if(locker->lockCount <= B_INT64_CONSTANT(1))
		{
			if(ReleaseMutex(locker->Locker) == 0)
			{
				bhapi::unlock_locker_inter(locker);
				return B_ERROR;
			}
		}

		locker->lockCount--;

		if(locker->lockCount <= B_INT64_CONSTANT(0))
		{
			locker->SetHolderThreadId(B_INT64_CONSTANT(0));
			locker->lockCount = B_INT64_CONSTANT(0);
		}

		bhapi::unlock_locker_inter(locker);
	}

	return B_OK;
}


BHAPI_EXPORT  int64 bhapi::count_locker_locks(void *data)
{
	int64 retVal = B_INT64_CONSTANT(0);

	bhapi::win32_locker_t *locker = (bhapi::win32_locker_t*)data;

	if(locker)
	{
		bhapi::lock_locker_inter(locker);
		if(locker->HolderThreadIsCurrent()) retVal = locker->lockCount;
		else if(locker->lockCount > B_INT64_CONSTANT(0)) retVal = -(locker->lockCount);
		bhapi::unlock_locker_inter(locker);
	}

	return retVal;
}


BHAPI_EXPORT void* bhapi::create_simple_locker(void)
{
	CRITICAL_SECTION *locker = (CRITICAL_SECTION*)malloc(sizeof(CRITICAL_SECTION));
	if(locker) InitializeCriticalSection(locker);
	return (void*)locker;
}


BHAPI_EXPORT status_t bhapi::delete_simple_locker(void* data)
{
	CRITICAL_SECTION *locker = (CRITICAL_SECTION*)data;
	if(!locker) return B_ERROR;
	DeleteCriticalSection(locker);
	free(locker);
	return B_OK;
}


BHAPI_EXPORT bool bhapi::lock_simple_locker(void *data)
{
	CRITICAL_SECTION *locker = (CRITICAL_SECTION*)data;
	if(!locker) return false;

	EnterCriticalSection(locker);

	return true;
}


BHAPI_EXPORT void bhapi::unlock_simple_locker(void *data)
{
	CRITICAL_SECTION *locker = (CRITICAL_SECTION*)data;
	if(!locker) return;

	LeaveCriticalSection(locker);
}


#ifdef BHAPI_BUILD_WITH_MEMORY_TRACING
static CRITICAL_SECTION *__bhapi_win32_memory_tracing_locker = NULL;
static LONG __bhapi_win32_memory_tracing_locker_inuse = 0;


void bhapi::win32_memory_tracing_locker_clean()
{
	while(InterlockedExchange(&__bhapi_win32_memory_tracing_locker_inuse, 1) == 1) Sleep(0);
	if(__bhapi_win32_memory_tracing_locker != NULL)
	{
		HeapFree(GetProcessHeap(), 0, __bhapi_win32_memory_tracing_locker);
		__bhapi_win32_memory_tracing_locker = NULL;
	}
	InterlockedExchange(&__bhapi_win32_memory_tracing_locker_inuse, 0);
}


BHAPI_EXPORT bool b_memory_tracing_lock(void)
{
	while(InterlockedExchange(&__bhapi_win32_memory_tracing_locker_inuse, 1) == 1) Sleep(0);
	if(__bhapi_win32_memory_tracing_locker == NULL)
	{
		__bhapi_win32_memory_tracing_locker = (CRITICAL_SECTION*)HeapAlloc(GetProcessHeap(), 0, sizeof(CRITICAL_SECTION));
		if(__bhapi_win32_memory_tracing_locker == NULL)
		{
			InterlockedExchange(&__bhapi_win32_memory_tracing_locker_inuse, 0);
			return false;
		}

		InitializeCriticalSection(__bhapi_win32_memory_tracing_locker);

		if(atexit(bhapi::win32_memory_tracing_locker_clean) != 0) BHAPI_ERROR("[KERNEL]: %s - atexit failed.", __PRETTY_FUNCTION__);
	}
	InterlockedExchange(&__bhapi_win32_memory_tracing_locker_inuse, 0);

	EnterCriticalSection(__bhapi_win32_memory_tracing_locker);

	return true;
}


BHAPI_EXPORT void b_memory_tracing_unlock(void)
{
	LeaveCriticalSection(__bhapi_win32_memory_tracing_locker);
}
#endif // BHAPI_BUILD_WITH_MEMORY_TRACING

