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
#ifdef BHAPI_OS_BEOS

#include <be/kernel/OS.h>

#include "../kernel/Kernel.h"

typedef struct b_beos_locker_t {
	b_beos_locker_t()
		: holderThreadId(B_INT64_CONSTANT(0)), lockCount(B_INT64_CONSTANT(0)), closed(false), created(false), refCount(0)
	{
	}

	~b_beos_locker_t()
	{
		if(created)
		{
			created = false;
			bhapi::delete_locker((void*)this);
		}
	}

	void SetHolderThreadId(b_int64 id)
	{
		holderThreadId = id;
	}

	bool HolderThreadIsCurrent(void)
	{
		return(holderThreadId == bhapi::get_current_thread_id());
	}

	b_int64			holderThreadId;
	b_int64			lockCount;
	bool			closed;
	sem_id			iLocker;
	sem_id			Locker;

	bool			created;

	b_uint32			refCount;
} b_beos_locker_t;


static void bhapi::lock_locker_inter(b_beos_locker_t *locker)
{
	acquire_sem(locker->iLocker);
}


static void bhapi::unlock_locker_inter(b_beos_locker_t *locker)
{
	release_sem(locker->iLocker);
}


IMPEXPBHAPI void* bhapi::create_locker(void)
{
	b_beos_locker_t *locker = new b_beos_locker_t();
	if(!locker) return NULL;

	locker->iLocker = create_sem(1, NULL);
	locker->Locker = create_sem(1, NULL);

	if(locker->iLocker < 0 || locker->Locker < 0)
	{
		if(locker->iLocker >= 0) delete_sem(locker->iLocker);
		if(locker->Locker >= 0) delete_sem(locker->Locker);
		delete locker;
		return NULL;
	}

	locker->refCount = 1;
	locker->created = true;

	return (void*)locker;
}


IMPEXPBHAPI void* bhapi::clone_locker(void *data)
{
	b_beos_locker_t *locker = (b_beos_locker_t*)data;
	if(!locker) return NULL;

	bhapi::lock_locker_inter(locker);

	if(locker->closed || locker->refCount >= B_MAXUINT32)
	{
		bhapi::unlock_locker_inter(locker);
		return NULL;
	}

	locker->refCount += 1;

	bhapi::unlock_locker_inter(locker);

	return data;
}


IMPEXPBHAPI b_status_t bhapi::delete_locker(void *data)
{
	b_beos_locker_t *locker = (b_beos_locker_t*)data;
	if(!locker) return B_BAD_VALUE;

	bhapi::lock_locker_inter(locker);
	b_uint32 count = --(locker->refCount);
#if 0
	bool showWarning = (locker->HolderThreadIsCurrent() && locker->closed == false && count > 0);
#endif
	bhapi::unlock_locker_inter(locker);

#if 0
	if(showWarning)
		BHAPI_OUTPUT("\n\
**************************************************************************\n\
*                      [KERNEL]: bhapi::delete_locker                       *\n\
*                                                                        *\n\
*  Locker still locked by current thread, and some clone-copies existed  *\n\
*  It's recommended that unlock or close the locker before delete it.    *\n\
*  Otherwise, the waitting thread will block!                            *\n\
**************************************************************************\n");
#endif

	if(count > 0) return B_OK;

	delete_sem(locker->iLocker);
	delete_sem(locker->Locker);

	if(locker->created)
	{
		locker->created = false;
		delete locker;
	}

	return B_OK;
}


/* after you call "bhapi::close_locker":
 * 	1. the next "bhapi::lock_locker..." function call will be failed
 * */
IMPEXPBHAPI b_status_t bhapi::close_locker(void *data)
{
	b_beos_locker_t *locker = (b_beos_locker_t*)data;
	if(!locker) return B_BAD_VALUE;

	bhapi::lock_locker_inter(locker);
	if(locker->closed)
	{
		bhapi::unlock_locker_inter(locker);
		return B_ERROR;
	}
	locker->closed = true;
	while(true)
	{
		int32 semCount = 0;
		if(get_sem_count(locker->Locker, &semCount) != B_OK) break;
		if(semCount > 0) break;
		release_sem(locker->Locker);
	}
	bhapi::unlock_locker_inter(locker);

	return B_OK;
}


IMPEXPBHAPI b_status_t bhapi::lock_locker(void *data)
{
	return bhapi::lock_locker_etc(data, B_TIMEOUT, B_INFINITE_TIMEOUT);
}


IMPEXPBHAPI b_status_t bhapi::lock_locker_etc(void *data, b_uint32 flags, b_bigtime_t microseconds_timeout)
{
	b_beos_locker_t *locker = (b_beos_locker_t*)data;
	if(!locker) return B_BAD_VALUE;

	if(microseconds_timeout < B_INT64_CONSTANT(0)) return B_BAD_VALUE;

	bool wait_forever = false;
	b_bigtime_t currentTime = b_real_time_clock_usecs();
	if(flags != B_ABSOLUTE_TIMEOUT)
	{
		if(microseconds_timeout == B_INFINITE_TIMEOUT || microseconds_timeout > B_MAXINT64 - currentTime)
			wait_forever = true;
		else
			microseconds_timeout += currentTime;
	}

	bhapi::lock_locker_inter(locker);

	if(locker->closed)
	{
		bhapi::unlock_locker_inter(locker);
		return B_ERROR;
	}

	b_status_t retVal = B_ERROR;

	if(locker->HolderThreadIsCurrent() == false)
	{
		bhapi::unlock_locker_inter(locker);

		status_t status = (wait_forever ? acquire_sem(locker->Locker) :
						  acquire_sem_etc(locker->Locker, 1,
							  	  B_ABSOLUTE_TIMEOUT,
								  (bigtime_t)(microseconds_timeout - b_system_boot_time())));

		bhapi::lock_locker_inter(locker);

		if(status != B_OK)
		{
			if(status == B_WOULD_BLOCK) retVal = B_WOULD_BLOCK;
			else if(status == B_TIMED_OUT) retVal = B_TIMED_OUT;
		}
		else if(locker->closed)
		{
			while(true)
			{
				int32 semCount = 0;
				if(get_sem_count(locker->Locker, &semCount) != B_OK) break;
				if(semCount > 0) break;
				release_sem(locker->Locker);
			}
		}
		else
		{
			locker->SetHolderThreadId(bhapi::get_current_thread_id());
			locker->lockCount = B_INT64_CONSTANT(1);
			retVal = B_OK;
		}

	}
	else
	{
		if(B_MAXINT64 - locker->lockCount >= B_INT64_CONSTANT(1))
		{
			locker->lockCount++;
			retVal = B_OK;
		}
	}

	bhapi::unlock_locker_inter(locker);

	return retVal;
}


IMPEXPBHAPI b_status_t bhapi::unlock_locker(void *data)
{
	b_beos_locker_t *locker = (b_beos_locker_t*)data;
	if(!locker) return B_BAD_VALUE;

	bhapi::lock_locker_inter(locker);

	if(locker->HolderThreadIsCurrent() == false)
	{
		bhapi::unlock_locker_inter(locker);
		BHAPI_WARNING("[KERNEL]: %s -- Can't unlock when didn't hold it in current thread!", __PRETTY_FUNCTION__);
		return B_ERROR;
	}
	else
	{
		if(locker->lockCount <= B_INT64_CONSTANT(1))
		{
			if(release_sem(locker->Locker) != B_OK)
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


IMPEXPBHAPI b_int64 bhapi::count_locker_locks(void *data)
{
	b_int64 retVal = B_INT64_CONSTANT(0);

	b_beos_locker_t *locker = (b_beos_locker_t*)data;

	if(locker)
	{
		bhapi::lock_locker_inter(locker);
		if(locker->HolderThreadIsCurrent()) retVal = locker->lockCount;
		else if(locker->lockCount > B_INT64_CONSTANT(0)) retVal = -(locker->lockCount);
		bhapi::unlock_locker_inter(locker);
	}

	return retVal;
}


IMPEXPBHAPI void* bhapi::create_simple_locker(void)
{
	sem_id *locker = (sem_id*)malloc(sizeof(sem_id));
	if(!locker) return NULL;

	if((*locker = create_sem(1, NULL)) < 0)
	{
		free(locker);
		return NULL;
	}

	return (void*)locker;
}


IMPEXPBHAPI b_status_t bhapi::delete_simple_locker(void* data)
{
	sem_id *locker = (sem_id*)data;
	if(!locker) return B_ERROR;
	delete_area(*locker);
	free(locker);
	return B_OK;
}


IMPEXPBHAPI bool bhapi::lock_simple_locker(void *data)
{
	sem_id *locker = (sem_id*)data;
	if(!locker) return false;
	return(acquire_sem(*locker) == B_OK);
}


IMPEXPBHAPI void bhapi::unlock_simple_locker(void *data)
{
	sem_id *locker = (sem_id*)data;
	if(!locker) return;
	release_sem(*locker);
}


#ifdef BHAPI_BUILD_WITH_MEMORY_TRACING
static sem_id __bhapi_win32_memory_tracing_locker = -1;
static vint32 __bhapi_beos_memory_tracing_locker_atom = 0;


IMPEXPBHAPI bool b_memory_tracing_lock(void)
{
	while(atomic_or(&__bhapi_beos_memory_tracing_locker_atom, 0x1) & 0x1) {snooze(1);}
	if(__bhapi_win32_memory_tracing_locker < 0)
	{
		if((__bhapi_win32_memory_tracing_locker = create_sem(1, NULL)) < 0)
		{
			atomic_and(&__bhapi_beos_memory_tracing_locker_atom, 0xfffffffe);
			return false;
		}
	}
	atomic_and(&__bhapi_beos_memory_tracing_locker_atom, 0xfffffffe);

	return(acquire_sem(__bhapi_win32_memory_tracing_locker) == B_OK);
}


IMPEXPBHAPI void b_memory_tracing_unlock(void)
{
	release_sem(__bhapi_win32_memory_tracing_locker);
}
#endif // BHAPI_BUILD_WITH_MEMORY_TRACING
#endif // BHAPI_OS_BEOS
