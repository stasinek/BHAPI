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

#include <pthread.h"
#include <errno.h"

#include "./../kernel/Kernel.h"
#include "./../support/String.h"

typedef struct etk_posix_locker_t {
	etk_posix_locker_t()
		: holderThreadId(E_INT64_CONSTANT(0)), lockCount(E_INT64_CONSTANT(0)), closed(false), created(false), refCount(0)
	{
	}

	~etk_posix_locker_t()
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
	pthread_mutex_t		iLocker;
	pthread_mutex_t		Locker;
	pthread_cond_t		Cond;

	bool			created;

	euint32			refCount;
} etk_posix_locker_t;


static void etk_lock_locker_inter(etk_posix_locker_t *locker)
{
	pthread_mutex_lock(&(locker->iLocker));
}


static void etk_unlock_locker_inter(etk_posix_locker_t *locker)
{
	pthread_mutex_unlock(&(locker->iLocker));
}


_IMPEXP_ETK void* etk_create_locker(void)
{
	etk_posix_locker_t *locker = new etk_posix_locker_t();
	if(!locker) return NULL;

	euint32 successFlags = 0;

	if(pthread_mutex_init(&(locker->iLocker), NULL) != 0) successFlags |= (1 << 1);
	if(pthread_mutex_init(&(locker->Locker), NULL) != 0) successFlags |= (1 << 2);
	if(pthread_cond_init(&(locker->Cond), NULL) != 0) successFlags |= (1 << 3);

	if(successFlags != 0)
	{
		if(!(successFlags & (1 << 1))) pthread_mutex_destroy(&(locker->iLocker));
		if(!(successFlags & (1 << 2))) pthread_mutex_destroy(&(locker->Locker));
		if(!(successFlags & (1 << 3))) pthread_cond_destroy(&(locker->Cond));
		delete locker;
		return NULL;
	}

	locker->refCount = 1;
	locker->created = true;

	return (void*)locker;
}


_IMPEXP_ETK void* etk_clone_locker(void *data)
{
	etk_posix_locker_t *locker = (etk_posix_locker_t*)data;
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
	etk_posix_locker_t *locker = (etk_posix_locker_t*)data;
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

	pthread_mutex_destroy(&(locker->iLocker));
	pthread_mutex_destroy(&(locker->Locker));
	pthread_cond_destroy(&(locker->Cond));

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
	etk_posix_locker_t *locker = (etk_posix_locker_t*)data;
	if(!locker) return E_BAD_VALUE;

	etk_lock_locker_inter(locker);
	if(locker->closed)
	{
		etk_unlock_locker_inter(locker);
		return E_ERROR;
	}
	locker->closed = true;
	pthread_cond_broadcast(&(locker->Cond));
	etk_unlock_locker_inter(locker);

	return E_OK;
}


_IMPEXP_ETK e_status_t etk_lock_locker(void *data)
{
	return etk_lock_locker_etc(data, E_TIMEOUT, E_INFINITE_TIMEOUT);
}


_IMPEXP_ETK e_status_t etk_lock_locker_etc(void *data, euint32 flags, e_bigtime_t microseconds_timeout)
{
	etk_posix_locker_t *locker = (etk_posix_locker_t*)data;
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

	if(locker->HolderThreadIsCurrent() == false)
	{
		pthread_mutex_t *iLocker = &(locker->iLocker);
		pthread_mutex_t *Locker = &(locker->Locker);
		pthread_cond_t *Cond = &(locker->Cond);

		if(!wait_forever && microseconds_timeout == currentTime)
		{
			if(pthread_mutex_trylock(Locker) != 0)
			{
				etk_unlock_locker_inter(locker);
				return E_WOULD_BLOCK;
			}
		}
		else
		{
			struct timespec ts;

			ts.tv_sec = (long)(microseconds_timeout / E_INT64_CONSTANT(1000000));
			ts.tv_nsec = (long)(microseconds_timeout % E_INT64_CONSTANT(1000000)) * 1000L;

			int ret;
			while((ret = pthread_mutex_trylock(Locker)) != 0)
			{
				if(ret != EBUSY || locker->closed)
				{
					etk_unlock_locker_inter(locker);
					return E_ERROR;
				}

				ret = (wait_forever ? pthread_cond_wait(Cond, iLocker) : pthread_cond_timedwait(Cond, iLocker, &ts));

				if(ret != 0)
				{
					if(ret == ETIMEDOUT && !wait_forever)
					{
						etk_unlock_locker_inter(locker);
						return E_TIMED_OUT;
					}
					else return E_ERROR;
				}
			}
		}

		if(locker->closed)
		{
			pthread_mutex_unlock(Locker);
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
	etk_posix_locker_t *locker = (etk_posix_locker_t*)data;
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
			if(pthread_mutex_unlock(&(locker->Locker)) != 0)
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
			pthread_cond_broadcast(&(locker->Cond));
		}

		etk_unlock_locker_inter(locker);
	}

	return E_OK;
}


_IMPEXP_ETK eint64 etk_count_locker_locks(void *data)
{
	eint64 retVal = E_INT64_CONSTANT(0);

	etk_posix_locker_t *locker = (etk_posix_locker_t*)data;

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
	pthread_mutex_t *locker = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
	if(!locker) return NULL;

	if(pthread_mutex_init(locker, NULL) != 0)
	{
		free(locker);
		return NULL;
	}

	return (void*)locker;
}


_IMPEXP_ETK e_status_t etk_delete_simple_locker(void* data)
{
	pthread_mutex_t *locker = (pthread_mutex_t*)data;
	if(!locker) return E_ERROR;
	pthread_mutex_destroy(locker);
	free(locker);
	return E_OK;
}


_IMPEXP_ETK bool etk_lock_simple_locker(void *data)
{
	pthread_mutex_t *locker = (pthread_mutex_t*)data;
	if(!locker) return false;

	if(pthread_mutex_lock(locker) != 0) return false;

	return true;
}


_IMPEXP_ETK void etk_unlock_simple_locker(void *data)
{
	pthread_mutex_t *locker = (pthread_mutex_t*)data;
	if(!locker) return;

	pthread_mutex_unlock(locker);
}


#ifdef ETK_BUILD_WITH_MEMORY_TRACING
static pthread_mutex_t __etk_posix_memory_tracing_locker = PTHREAD_MUTEX_INITIALIZER;


_IMPEXP_ETK bool etk_memory_tracing_lock(void)
{
	if(pthread_mutex_lock(&__etk_posix_memory_tracing_locker) != 0) return false;
	return true;
}


_IMPEXP_ETK void etk_memory_tracing_unlock(void)
{
	pthread_mutex_unlock(&__etk_posix_memory_tracing_locker);
}
#endif // ETK_BUILD_WITH_MEMORY_TRACING

