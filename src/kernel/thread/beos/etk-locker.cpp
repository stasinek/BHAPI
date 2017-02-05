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

#include <be/kernel/OS.h"

#include "./../kernel/Kernel.h"

typedef struct etk_beos_locker_t {
	etk_beos_locker_t()
		: holderThreadId(E_INT64_CONSTANT(0)), lockCount(E_INT64_CONSTANT(0)), closed(false), created(false), refCount(0)
	{
	}

	~etk_beos_locker_t()
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
	sem_id			iLocker;
	sem_id			Locker;

	bool			created;

	euint32			refCount;
} etk_beos_locker_t;


static void etk_lock_locker_inter(etk_beos_locker_t *locker)
{
	acquire_sem(locker->iLocker);
}


static void etk_unlock_locker_inter(etk_beos_locker_t *locker)
{
	release_sem(locker->iLocker);
}


_IMPEXP_ETK void* etk_create_locker(void)
{
	etk_beos_locker_t *locker = new etk_beos_locker_t();
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


_IMPEXP_ETK void* etk_clone_locker(void *data)
{
	etk_beos_locker_t *locker = (etk_beos_locker_t*)data;
	if(!locker) return NULL;

	etk_lock_locker_inter(locker);

	if(locker->closed || locker->refCount >= E_MAXUINT32)
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
	etk_beos_locker_t *locker = (etk_beos_locker_t*)data;
	if(!locker) return E_BAD_VALUE;

	etk_lock_locker_inter(locker);
	euint32 count = --(locker->refCount);
#if 0
	bool showWarning = (locker->HolderThreadIsCurrent() && locker->closed == false && count > 0);
#endif
	etk_unlock_locker_inter(locker);

#if 0
	if(showWarning)
		ETK_OUTPUT("\n\
**************************************************************************\n\
*                      [KERNEL]: etk_delete_locker                       *\n\
*                                                                        *\n\
*  Locker still locked by current thread, and some clone-copies existed  *\n\
*  It's recommended that unlock or close the locker before delete it.    *\n\
*  Otherwise, the waitting thread will block!                            *\n\
**************************************************************************\n");
#endif

	if(count > 0) return E_OK;

	delete_sem(locker->iLocker);
	delete_sem(locker->Locker);

	if(locker->created)
	{
		locker->created = false;
		delete locker;
	}

	return E_OK;
}


/* after you call "etk_close_locker":
 * 	1. the next "etk_lock_locker..." function call will be failed
 * */
_IMPEXP_ETK e_status_t etk_close_locker(void *data)
{
	etk_beos_locker_t *locker = (etk_beos_locker_t*)data;
	if(!locker) return E_BAD_VALUE;

	etk_lock_locker_inter(locker);
	if(locker->closed)
	{
		etk_unlock_locker_inter(locker);
		return E_ERROR;
	}
	locker->closed = true;
	while(true)
	{
		int32 semCount = 0;
		if(get_sem_count(locker->Locker, &semCount) != B_OK) break;
		if(semCount > 0) break;
		release_sem(locker->Locker);
	}
	etk_unlock_locker_inter(locker);

	return E_OK;
}


_IMPEXP_ETK e_status_t etk_lock_locker(void *data)
{
	return etk_lock_locker_etc(data, E_TIMEOUT, E_INFINITE_TIMEOUT);
}


_IMPEXP_ETK e_status_t etk_lock_locker_etc(void *data, euint32 flags, e_bigtime_t microseconds_timeout)
{
	etk_beos_locker_t *locker = (etk_beos_locker_t*)data;
	if(!locker) return E_BAD_VALUE;

	if(microseconds_timeout < E_INT64_CONSTANT(0)) return E_BAD_VALUE;

	bool wait_forever = false;
	e_bigtime_t currentTime = etk_real_time_clock_usecs();
	if(flags != E_ABSOLUTE_TIMEOUT)
	{
		if(microseconds_timeout == E_INFINITE_TIMEOUT || microseconds_timeout > E_MAXINT64 - currentTime)
			wait_forever = true;
		else
			microseconds_timeout += currentTime;
	}

	etk_lock_locker_inter(locker);

	if(locker->closed)
	{
		etk_unlock_locker_inter(locker);
		return E_ERROR;
	}

	e_status_t retVal = E_ERROR;

	if(locker->HolderThreadIsCurrent() == false)
	{
		etk_unlock_locker_inter(locker);

		status_t status = (wait_forever ? acquire_sem(locker->Locker) :
						  acquire_sem_etc(locker->Locker, 1,
							  	  B_ABSOLUTE_TIMEOUT,
								  (bigtime_t)(microseconds_timeout - etk_system_boot_time())));

		etk_lock_locker_inter(locker);

		if(status != B_OK)
		{
			if(status == B_WOULD_BLOCK) retVal = E_WOULD_BLOCK;
			else if(status == B_TIMED_OUT) retVal = E_TIMED_OUT;
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
			locker->SetHolderThreadId(etk_get_current_thread_id());
			locker->lockCount = E_INT64_CONSTANT(1);
			retVal = E_OK;
		}

	}
	else
	{
		if(E_MAXINT64 - locker->lockCount >= E_INT64_CONSTANT(1))
		{
			locker->lockCount++;
			retVal = E_OK;
		}
	}

	etk_unlock_locker_inter(locker);

	return retVal;
}


_IMPEXP_ETK e_status_t etk_unlock_locker(void *data)
{
	etk_beos_locker_t *locker = (etk_beos_locker_t*)data;
	if(!locker) return E_BAD_VALUE;

	etk_lock_locker_inter(locker);

	if(locker->HolderThreadIsCurrent() == false)
	{
		etk_unlock_locker_inter(locker);
		ETK_WARNING("[KERNEL]: %s -- Can't unlock when didn't hold it in current thread!", __PRETTY_FUNCTION__);
		return E_ERROR;
	}
	else
	{
		if(locker->lockCount <= E_INT64_CONSTANT(1))
		{
			if(release_sem(locker->Locker) != B_OK)
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

	etk_beos_locker_t *locker = (etk_beos_locker_t*)data;

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
	sem_id *locker = (sem_id*)malloc(sizeof(sem_id));
	if(!locker) return NULL;

	if((*locker = create_sem(1, NULL)) < 0)
	{
		free(locker);
		return NULL;
	}

	return (void*)locker;
}


_IMPEXP_ETK e_status_t etk_delete_simple_locker(void* data)
{
	sem_id *locker = (sem_id*)data;
	if(!locker) return E_ERROR;
	delete_area(*locker);
	free(locker);
	return E_OK;
}


_IMPEXP_ETK bool etk_lock_simple_locker(void *data)
{
	sem_id *locker = (sem_id*)data;
	if(!locker) return false;
	return(acquire_sem(*locker) == B_OK);
}


_IMPEXP_ETK void etk_unlock_simple_locker(void *data)
{
	sem_id *locker = (sem_id*)data;
	if(!locker) return;
	release_sem(*locker);
}


#ifdef ETK_BUILD_WITH_MEMORY_TRACING
static sem_id __etk_win32_memory_tracing_locker = -1;
static vint32 __etk_beos_memory_tracing_locker_atom = 0;


_IMPEXP_ETK bool etk_memory_tracing_lock(void)
{
	while(atomic_or(&__etk_beos_memory_tracing_locker_atom, 0x1) & 0x1) {snooze(1);}
	if(__etk_win32_memory_tracing_locker < 0)
	{
		if((__etk_win32_memory_tracing_locker = create_sem(1, NULL)) < 0)
		{
			atomic_and(&__etk_beos_memory_tracing_locker_atom, 0xfffffffe);
			return false;
		}
	}
	atomic_and(&__etk_beos_memory_tracing_locker_atom, 0xfffffffe);

	return(acquire_sem(__etk_win32_memory_tracing_locker) == B_OK);
}


_IMPEXP_ETK void etk_memory_tracing_unlock(void)
{
	release_sem(__etk_win32_memory_tracing_locker);
}
#endif // ETK_BUILD_WITH_MEMORY_TRACING

