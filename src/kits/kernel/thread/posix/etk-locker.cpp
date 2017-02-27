/* --------------------------------------------------------------------------
 *
 * BHAPI++ previously named ETK++, The Easy Toolkit for C++ programing
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
 * File: etk-locker.cpp
 *
 * --------------------------------------------------------------------------*/
#ifdef BHAPI_OS_LINUX

#include <pthread.h>
#include <errno.h"

#include "../kernel/Kernel.h"
#include "../support/String.h"

typedef struct b_posix_locker_t {
	b_posix_locker_t()
		: holderThreadId(B_INT64_CONSTANT(0)), lockCount(B_INT64_CONSTANT(0)), closed(false), created(false), refCount(0)
	{
	}

	~b_posix_locker_t()
	{
		if(created)
		{
			created = false;
			b_delete_locker((void*)this);
		}
	}

	void SetHolderThreadId(b_int64 id)
	{
		holderThreadId = id;
	}

	bool HolderThreadIsCurrent(void)
	{
		return(holderThreadId == b_get_current_thread_id());
	}

	b_int64			holderThreadId;
	b_int64			lockCount;
	bool			closed;
	pthread_mutex_t		iLocker;
	pthread_mutex_t		Locker;
	pthread_cond_t		Cond;

	bool			created;

	b_uint32			refCount;
} b_posix_locker_t;


static void b_lock_locker_inter(b_posix_locker_t *locker)
{
	pthread_mutex_lock(&(locker->iLocker));
}


static void b_unlock_locker_inter(b_posix_locker_t *locker)
{
	pthread_mutex_unlock(&(locker->iLocker));
}


IMPEXP_BHAPI void* b_create_locker(void)
{
	b_posix_locker_t *locker = new b_posix_locker_t();
	if(!locker) return NULL;

	b_uint32 successFlags = 0;

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


IMPEXP_BHAPI void* b_clone_locker(void *data)
{
	b_posix_locker_t *locker = (b_posix_locker_t*)data;
	if(!locker) return NULL;

	b_lock_locker_inter(locker);

	if(locker->closed || locker->refCount >= B_MAXUINT32)
	{
		b_unlock_locker_inter(locker);
		return NULL;
	}

	locker->refCount += 1;

	b_unlock_locker_inter(locker);

	return data;
}


IMPEXP_BHAPI b_status_t b_delete_locker(void *data)
{
	b_posix_locker_t *locker = (b_posix_locker_t*)data;
	if(!locker) return B_BAD_VALUE;

	b_lock_locker_inter(locker);
	b_uint32 count = --(locker->refCount);
#if 0
	bool showWarning = (locker->HolderThreadIsCurrent() && locker->closed == false && count > 0);
#endif
	b_unlock_locker_inter(locker);

#if 0
	if(showWarning)
		BHAPI_OUTPUT("\n\
**************************************************************************\n\
*                      [KERNEL]: b_delete_locker                       *\n\
*                                                                        *\n\
*  Locker still locked by current thread, and some clone-copies existed  *\n\
*  It's recommended that unlock or close the locker before delete it.    *\n\
*  Otherwise, the waitting thread will block!                            *\n\
**************************************************************************\n");
#endif

	if(count > 0) return B_OK;

	pthread_mutex_destroy(&(locker->iLocker));
	pthread_mutex_destroy(&(locker->Locker));
	pthread_cond_destroy(&(locker->Cond));

	if(locker->created)
	{
		locker->created = false;
		delete locker;
	}

	return B_OK;
}


/* after you call "b_close_locker":
 * 	1. the next "b_lock_locker..." function call will be failed
 * */
IMPEXP_BHAPI b_status_t b_close_locker(void *data)
{
	b_posix_locker_t *locker = (b_posix_locker_t*)data;
	if(!locker) return B_BAD_VALUE;

	b_lock_locker_inter(locker);
	if(locker->closed)
	{
		b_unlock_locker_inter(locker);
		return B_ERROR;
	}
	locker->closed = true;
	pthread_cond_broadcast(&(locker->Cond));
	b_unlock_locker_inter(locker);

	return B_OK;
}


IMPEXP_BHAPI b_status_t b_lock_locker(void *data)
{
	return b_lock_locker_etc(data, B_TIMEOUT, B_INFINITE_TIMEOUT);
}


IMPEXP_BHAPI b_status_t b_lock_locker_etc(void *data, b_uint32 flags, b_bigtime_t microseconds_timeout)
{
	b_posix_locker_t *locker = (b_posix_locker_t*)data;
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

	b_lock_locker_inter(locker);

	if(locker->closed)
	{
		b_unlock_locker_inter(locker);
		return B_ERROR;
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
				b_unlock_locker_inter(locker);
				return B_WOULD_BLOCK;
			}
		}
		else
		{
			struct timespec ts;

			ts.tv_sec = (long)(microseconds_timeout / B_INT64_CONSTANT(1000000));
			ts.tv_nsec = (long)(microseconds_timeout % B_INT64_CONSTANT(1000000)) * 1000L;

			int ret;
			while((ret = pthread_mutex_trylock(Locker)) != 0)
			{
				if(ret != EBUSY || locker->closed)
				{
					b_unlock_locker_inter(locker);
					return B_ERROR;
				}

				ret = (wait_forever ? pthread_cond_wait(Cond, iLocker) : pthread_cond_timedwait(Cond, iLocker, &ts));

				if(ret != 0)
				{
					if(ret == ETIMEDOUT && !wait_forever)
					{
						b_unlock_locker_inter(locker);
						return B_TIMED_OUT;
					}
					else return B_ERROR;
				}
			}
		}

		if(locker->closed)
		{
			pthread_mutex_unlock(Locker);
			b_unlock_locker_inter(locker);
			return B_ERROR;
		}

		locker->SetHolderThreadId(b_get_current_thread_id());
		locker->lockCount = B_INT64_CONSTANT(1);

		b_unlock_locker_inter(locker);
	}
	else
	{
		if(B_MAXINT64 - locker->lockCount < B_INT64_CONSTANT(1))
		{
			b_unlock_locker_inter(locker);
			return B_ERROR;
		}

		locker->lockCount++;
		b_unlock_locker_inter(locker);
	}

	return B_OK;
}


IMPEXP_BHAPI b_status_t b_unlock_locker(void *data)
{
	b_posix_locker_t *locker = (b_posix_locker_t*)data;
	if(!locker) return B_BAD_VALUE;

	b_lock_locker_inter(locker);

	if(locker->HolderThreadIsCurrent() == false)
	{
		b_unlock_locker_inter(locker);
		BHAPI_WARNING("[KERNEL]: %s -- Can't unlock when didn't hold it in current thread!", __PRETTY_FUNCTION__);
		return B_ERROR;
	}
	else
	{
		if(locker->lockCount <= B_INT64_CONSTANT(1))
		{
			if(pthread_mutex_unlock(&(locker->Locker)) != 0)
			{
				b_unlock_locker_inter(locker);
				return B_ERROR;
			}
		}

		locker->lockCount--;

		if(locker->lockCount <= B_INT64_CONSTANT(0))
		{
			locker->SetHolderThreadId(B_INT64_CONSTANT(0));
			locker->lockCount = B_INT64_CONSTANT(0);
			pthread_cond_broadcast(&(locker->Cond));
		}

		b_unlock_locker_inter(locker);
	}

	return B_OK;
}


IMPEXP_BHAPI b_int64 b_count_locker_locks(void *data)
{
	b_int64 retVal = B_INT64_CONSTANT(0);

	b_posix_locker_t *locker = (b_posix_locker_t*)data;

	if(locker)
	{
		b_lock_locker_inter(locker);
		if(locker->HolderThreadIsCurrent()) retVal = locker->lockCount;
		else if(locker->lockCount > B_INT64_CONSTANT(0)) retVal = -(locker->lockCount);
		b_unlock_locker_inter(locker);
	}

	return retVal;
}


IMPEXP_BHAPI void* b_create_simple_locker(void)
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


IMPEXP_BHAPI b_status_t b_delete_simple_locker(void* data)
{
	pthread_mutex_t *locker = (pthread_mutex_t*)data;
	if(!locker) return B_ERROR;
	pthread_mutex_destroy(locker);
	free(locker);
	return B_OK;
}


IMPEXP_BHAPI bool b_lock_simple_locker(void *data)
{
	pthread_mutex_t *locker = (pthread_mutex_t*)data;
	if(!locker) return false;

	if(pthread_mutex_lock(locker) != 0) return false;

	return true;
}


IMPEXP_BHAPI void b_unlock_simple_locker(void *data)
{
	pthread_mutex_t *locker = (pthread_mutex_t*)data;
	if(!locker) return;

	pthread_mutex_unlock(locker);
}


#ifdef BHAPI_BUILD_WITH_MEMORY_TRACING
static pthread_mutex_t __bhapi_posix_memory_tracing_locker = PTHREAD_MUTEX_INITIALIZER;


IMPEXP_BHAPI bool b_memory_tracing_lock(void)
{
	if(pthread_mutex_lock(&__bhapi_posix_memory_tracing_locker) != 0) return false;
	return true;
}


IMPEXP_BHAPI void b_memory_tracing_unlock(void)
{
	pthread_mutex_unlock(&__bhapi_posix_memory_tracing_locker);
}
#endif // BHAPI_BUILD_WITH_MEMORY_TRACING
#endif // BHAPI_OS_LINUX
