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
 * File: etk-locker.cpp
 *
 * --------------------------------------------------------------------------*/

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0500
#endif

#include <windows.h>

#include "./../kernel/Kernel.h"

#define SECS_BETWEEN_EPOCHS	E_INT64_CONSTANT(11644473600)
#define SECS_TO_100NS		E_INT64_CONSTANT(10000000)

typedef struct etk_win32_locker_t {
	etk_win32_locker_t()
		: holderThreadId(E_INT64_CONSTANT(0)), lockCount(E_INT64_CONSTANT(0)), closed(false), Locker(NULL), created(false), refCount(0)
	{
	}

	~etk_win32_locker_t()
	{
		if(created)
		{
			created = false;
			etk_delete_locker((void*)this);
		}
	}

	void SetHolderThreadId(eint64 id)
	{
		holderThreadId = id;
	}

	bool HolderThreadIsCurrent(void)
	{
		return(holderThreadId == etk_get_current_thread_id());
	}

	eint64			holderThreadId;
	eint64			lockCount;
	bool			closed;

	CRITICAL_SECTION	iLocker;
	HANDLE			Locker;
	HANDLE			Cond;

	bool			created;

	euint32			refCount;
} etk_win32_locker_t;


static void etk_lock_locker_inter(etk_win32_locker_t *locker)
{
	EnterCriticalSection(&(locker->iLocker));
}


static void etk_unlock_locker_inter(etk_win32_locker_t *locker)
{
	LeaveCriticalSection(&(locker->iLocker));
}


void* etk_create_locker(void)
{
	etk_win32_locker_t *locker = new etk_win32_locker_t();

	if(!locker) return NULL;

	if((locker->Locker = CreateMutex(NULL, FALSE, NULL)) == NULL)
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


_IMPEXP_ETK void* etk_clone_locker(void *data)
{
	etk_win32_locker_t *locker = (etk_win32_locker_t*)data;
	if(!locker) return NULL;

	etk_lock_locker_inter(locker);

	if(locker->closed || locker->refCount >= E_MAXUINT32 || locker->refCount == 0)
	{
		etk_unlock_locker_inter(locker);
		return NULL;
	}

	locker->refCount += 1;

	etk_unlock_locker_inter(locker);

	return data;
}


_IMPEXP_ETK e_status_t etk_delete_locker(void *data)
{
	etk_win32_locker_t *locker = (etk_win32_locker_t*)data;
	if(!locker) return E_BAD_VALUE;

	etk_lock_locker_inter(locker);
	if(locker->refCount == 0)
	{
		etk_unlock_locker_inter(locker);
		return E_ERROR;
	}
	euint32 count = --(locker->refCount);
#if 0
	bool locked = locker->HolderThreadIsCurrent();
#endif
	etk_unlock_locker_inter(locker);

#if 0
	if(locked && count > 0)
		ETK_DEBUG("\n\
**************************************************************************\n\
*                      [KERNEL]: etk_delete_locker                       *\n\
*                                                                        *\n\
*  Locker still locked by current thread, and some clone-copies existed  *\n\
*  It's recommended that unlock the locker before you delete it.         *\n\
*  Otherwise, the waitting thread will block!                            *\n\
**************************************************************************\n");
#endif

	if(count > 0) return E_OK;

	if(locker->HolderThreadIsCurrent()) ReleaseMutex(locker->Locker);
	CloseHandle(locker->Locker);
	CloseHandle(locker->Cond);
	DeleteCriticalSection(&(locker->iLocker));

	if(locker->created)
	{
		locker->created = false;
		delete locker;
	}

	return E_OK;
}


_IMPEXP_ETK e_status_t etk_close_locker(void *data)
{
	etk_win32_locker_t *locker = (etk_win32_locker_t*)data;
	if(!locker) return E_BAD_VALUE;

	etk_lock_locker_inter(locker);
	if(locker->closed)
	{
		etk_unlock_locker_inter(locker);
		return E_ERROR;
	}
	locker->closed = true;
	SetEvent(locker->Cond);
	etk_unlock_locker_inter(locker);

	return E_OK;
}


_IMPEXP_ETK e_status_t etk_lock_locker(void *data)
{
	etk_win32_locker_t *locker = (etk_win32_locker_t*)data;
	if(!locker) return E_BAD_VALUE;

	etk_lock_locker_inter(locker);

	if(locker->closed)
	{
		etk_unlock_locker_inter(locker);
		return E_ERROR;
	}

	if(locker->HolderThreadIsCurrent() == false)
	{
		HANDLE handles[2];
		handles[0] = locker->Locker;
		handles[1] = locker->Cond;

		etk_unlock_locker_inter(locker);

		if(WaitForMultipleObjects(2, handles, FALSE, INFINITE) != WAIT_OBJECT_0) return E_ERROR;

		etk_lock_locker_inter(locker);

		if(locker->closed)
		{
			ReleaseMutex(handles[0]);
			etk_unlock_locker_inter(locker);
			return E_ERROR;
		}

		locker->SetHolderThreadId(etk_get_current_thread_id());
		locker->lockCount = E_INT64_CONSTANT(1);

		etk_unlock_locker_inter(locker);
	}
	else
	{
		if(E_MAXINT64 - locker->lockCount < E_INT64_CONSTANT(1))
		{
			etk_unlock_locker_inter(locker);
			return E_ERROR;
		}

		locker->lockCount++;
		etk_unlock_locker_inter(locker);
	}

	return E_OK;
}


_IMPEXP_ETK e_status_t etk_lock_locker_etc(void *data, euint32 flags, e_bigtime_t microseconds_timeout)
{
	etk_win32_locker_t *locker = (etk_win32_locker_t*)data;
	if(!locker) return E_BAD_VALUE;

	if(microseconds_timeout < E_INT64_CONSTANT(0)) return E_BAD_VALUE;

	e_bigtime_t currentTime = etk_real_time_clock_usecs();
	if(flags != E_ABSOLUTE_TIMEOUT)
	{
		if(microseconds_timeout == E_INFINITE_TIMEOUT || microseconds_timeout > E_MAXINT64 - currentTime)
			return etk_lock_locker(data);
		else
			microseconds_timeout += currentTime;
	}

	etk_lock_locker_inter(locker);

	if(locker->closed)
	{
		etk_unlock_locker_inter(locker);
		return E_ERROR;
	}

	if(locker->HolderThreadIsCurrent() == false)
	{
		HANDLE mutex = locker->Locker;

		if(microseconds_timeout == currentTime)
		{
			etk_unlock_locker_inter(locker);

			if(WaitForSingleObject(mutex, 0L) != WAIT_OBJECT_0) return E_WOULD_BLOCK;
		}
		else
		{
			HANDLE timer = NULL;
			LARGE_INTEGER due;
			due.QuadPart = microseconds_timeout * E_INT64_CONSTANT(10) + SECS_BETWEEN_EPOCHS * SECS_TO_100NS;
			timer = CreateWaitableTimer(NULL, TRUE, NULL);
			if(timer == NULL)
			{
				etk_unlock_locker_inter(locker);
				return E_ERROR;
			}
			if(SetWaitableTimer(timer, &due, 0, NULL, NULL, 0) == 0)
			{
				CloseHandle(timer);
				etk_unlock_locker_inter(locker);
				return E_ERROR;
			}

			HANDLE handles[3];
			handles[0] = mutex;
			handles[1] = timer;
			handles[2] = locker->Cond;

			etk_unlock_locker_inter(locker);

			DWORD status = WaitForMultipleObjects(3, handles, FALSE, INFINITE);

			CloseHandle(timer);

			if(status - WAIT_OBJECT_0 == 1 || status == WAIT_TIMEOUT)
				return E_TIMED_OUT;
			else if(status != WAIT_OBJECT_0)
				return E_ERROR;
		}

		etk_lock_locker_inter(locker);

		if(locker->closed)
		{
			ReleaseMutex(mutex);
			etk_unlock_locker_inter(locker);
			return E_ERROR;
		}

		locker->SetHolderThreadId(etk_get_current_thread_id());
		locker->lockCount = E_INT64_CONSTANT(1);

		etk_unlock_locker_inter(locker);
	}
	else
	{
		if(E_MAXINT64 - locker->lockCount < E_INT64_CONSTANT(1))
		{
			etk_unlock_locker_inter(locker);
			return E_ERROR;
		}

		locker->lockCount++;
		etk_unlock_locker_inter(locker);
	}

	return E_OK;
}


_IMPEXP_ETK e_status_t etk_unlock_locker(void *data)
{
	etk_win32_locker_t *locker = (etk_win32_locker_t*)data;
	if(!locker) return E_BAD_VALUE;

	etk_lock_locker_inter(locker);

	if(locker->HolderThreadIsCurrent() == false)
	{
		etk_unlock_locker_inter(locker);
		ETK_ERROR("[KERNEL]: %s --- Can't unlock when didn't hold it in current thread!", __PRETTY_FUNCTION__);
		return E_ERROR;
	}
	else
	{
		if(locker->lockCount <= E_INT64_CONSTANT(1))
		{
			if(ReleaseMutex(locker->Locker) == 0)
			{
				etk_unlock_locker_inter(locker);
				return E_ERROR;
			}
		}

		locker->lockCount--;

		if(locker->lockCount <= E_INT64_CONSTANT(0))
		{
			locker->SetHolderThreadId(E_INT64_CONSTANT(0));
			locker->lockCount = E_INT64_CONSTANT(0);
		}

		etk_unlock_locker_inter(locker);
	}

	return E_OK;
}


_IMPEXP_ETK eint64 etk_count_locker_locks(void *data)
{
	eint64 retVal = E_INT64_CONSTANT(0);

	etk_win32_locker_t *locker = (etk_win32_locker_t*)data;

	if(locker)
	{
		etk_lock_locker_inter(locker);
		if(locker->HolderThreadIsCurrent()) retVal = locker->lockCount;
		else if(locker->lockCount > E_INT64_CONSTANT(0)) retVal = -(locker->lockCount);
		etk_unlock_locker_inter(locker);
	}

	return retVal;
}


_IMPEXP_ETK void* etk_create_simple_locker(void)
{
	CRITICAL_SECTION *locker = (CRITICAL_SECTION*)malloc(sizeof(CRITICAL_SECTION));
	if(locker) InitializeCriticalSection(locker);
	return (void*)locker;
}


_IMPEXP_ETK e_status_t etk_delete_simple_locker(void* data)
{
	CRITICAL_SECTION *locker = (CRITICAL_SECTION*)data;
	if(!locker) return E_ERROR;
	DeleteCriticalSection(locker);
	free(locker);
	return E_OK;
}


_IMPEXP_ETK bool etk_lock_simple_locker(void *data)
{
	CRITICAL_SECTION *locker = (CRITICAL_SECTION*)data;
	if(!locker) return false;

	EnterCriticalSection(locker);

	return true;
}


_IMPEXP_ETK void etk_unlock_simple_locker(void *data)
{
	CRITICAL_SECTION *locker = (CRITICAL_SECTION*)data;
	if(!locker) return;

	LeaveCriticalSection(locker);
}


#ifdef ETK_BUILD_WITH_MEMORY_TRACING
static CRITICAL_SECTION *__etk_win32_memory_tracing_locker = NULL;
static LONG __etk_win32_memory_tracing_locker_inuse = 0;


void etk_win32_memory_tracing_locker_clean()
{
	while(InterlockedExchange(&__etk_win32_memory_tracing_locker_inuse, 1) == 1) Sleep(0);
	if(__etk_win32_memory_tracing_locker != NULL)
	{
		HeapFree(GetProcessHeap(), 0, __etk_win32_memory_tracing_locker);
		__etk_win32_memory_tracing_locker = NULL;
	}
	InterlockedExchange(&__etk_win32_memory_tracing_locker_inuse, 0);
}


_IMPEXP_ETK bool etk_memory_tracing_lock(void)
{
	while(InterlockedExchange(&__etk_win32_memory_tracing_locker_inuse, 1) == 1) Sleep(0);
	if(__etk_win32_memory_tracing_locker == NULL)
	{
		__etk_win32_memory_tracing_locker = (CRITICAL_SECTION*)HeapAlloc(GetProcessHeap(), 0, sizeof(CRITICAL_SECTION));
		if(__etk_win32_memory_tracing_locker == NULL)
		{
			InterlockedExchange(&__etk_win32_memory_tracing_locker_inuse, 0);
			return false;
		}

		InitializeCriticalSection(__etk_win32_memory_tracing_locker);

		if(atexit(etk_win32_memory_tracing_locker_clean) != 0) ETK_ERROR("[KERNEL]: %s - atexit failed.", __PRETTY_FUNCTION__);
	}
	InterlockedExchange(&__etk_win32_memory_tracing_locker_inuse, 0);

	EnterCriticalSection(__etk_win32_memory_tracing_locker);

	return true;
}


_IMPEXP_ETK void etk_memory_tracing_unlock(void)
{
	LeaveCriticalSection(__etk_win32_memory_tracing_locker);
}
#endif // ETK_BUILD_WITH_MEMORY_TRACING

