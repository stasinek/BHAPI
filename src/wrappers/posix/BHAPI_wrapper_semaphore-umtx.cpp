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
 * File: BHAPI_wrapper_semaphore-umtx.cpp
 *
 * --------------------------------------------------------------------------*/
#ifdef BHAPI_OS_LINUX


#include <fcntl.h>

#include <pthread.h>
#include <semaphore.h>

#include <sys/types.h>
#include <unistd.h>

#include <time.h>
#include <errno.h>

extern "C" {
/* use umtx instead of pshared-semaphore */
#include <machine/atomic.h>
#include <ucontext.h>
#include <sys/thr.h>
#include <sys/umtx.h>
} // extern "C"

#include <sys/stat.h>
#include <sys/mman.h>

#include <os/kernel.h>
#include <kits/support/String.h>

typedef struct b_posix_sem_info {
	b_posix_sem_info()
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
		seq = B_INT64_CONSTANT(0);
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

	// for IPC
	struct umtx		locker;
	long			seq;

	// for local
	pthread_mutex_t		mutex;
	pthread_cond_t		cond;

	uint32_t			refCount;
} b_posix_sem_info;


typedef struct b_posix_sem_t {
	b_posix_sem_t()
		: mapping(NULL), semInfo(NULL), iMutex(NULL), iCond(NULL),
		  created(false), no_clone(false)
	{
	}

	~b_posix_sem_t()
	{
		if(created)
		{
			created = false;
			bhapi::delete_sem((void*)this);
		}
	}

	// for IPC (name != NULL)
	void*			mapping;
	b_posix_sem_info*	semInfo;

	struct umtx*		iLocker;
	long*			iSeq;

	// for local
	pthread_mutex_t*	iMutex;
	pthread_cond_t*		iCond;

	bool			created;
	bool			no_clone;
} b_posix_sem_t;


static long bhapi::get_current_bsd_tid()
{
	long tid;
	thr_self(&tid);
	return tid;
}

#ifndef SEM_FAILED
#define SEM_FAILED	(-1)
#endif

class b_posix_sem_locker_t {
public:
	sem_t *fSem;
	pthread_mutex_t fLocker;

	b_posix_sem_locker_t()
		: fSem((sem_t*)SEM_FAILED)
	{
		pthread_mutex_init(&fLocker, NULL);
	}

	~b_posix_sem_locker_t()
	{
		pthread_mutex_destroy(&fLocker);

		if(fSem != (sem_t*)SEM_FAILED)
		{
			sem_close(fSem);
			// leave global semaphore, without sem_unlink
		}
	}

	void Init()
	{
		if(fSem != (sem_t*)SEM_FAILED) return;

		const char *semName = "/_bhapi_global_";
		if((fSem = (sem_t*)sem_open(semName, O_CREAT | O_EXCL, 0666, 1)) == (sem_t*)SEM_FAILED)
		{
//			BHAPI_DEBUG("[KERNEL]: Unable to create global semaphore, errno: %d", errno);
			fSem = (sem_t*)sem_open(semName, 0);
		}
		if(fSem == (sem_t*)SEM_FAILED)
			BHAPI_ERROR("[KERNEL]: Can't initialize global semaphore! errno: %d", errno);
	}

	void LockLocal()
	{
		pthread_mutex_lock(&fLocker);
	}

	void UnlockLocal()
	{
		pthread_mutex_unlock(&fLocker);
	}

	void LockIPC()
	{
		LockLocal();
		Init();
		UnlockLocal();
		sem_wait(fSem);
	}

	void UnlockIPC()
	{
		sem_post(fSem);
	}
};


static b_posix_sem_locker_t __bhapi_semaphore_locker__;
#define BHAPI_LOCK_IPC_SEMAPHORE()		__bhapi_semaphore_locker__.LockIPC()
#define BHAPI_UNLOCK_IPC_SEMAPHORE()		__bhapi_semaphore_locker__.UnlockIPC()
#define BHAPI_LOCKBHAPI_LOCAL_SEMAPHORE()		__bhapi_semaphore_locker__.LockLocal()
#define BHAPI_UNLOCKBHAPI_LOCAL_SEMAPHORE()		__bhapi_semaphore_locker__.UnlockLocal()


static bool bhapi::is_sem_for_IPC(const b_posix_sem_t *sem)
{
	if(!sem) return false;
	return(sem->mapping != NULL);
}


static void bhapi::lock_sem_inter(b_posix_sem_t *sem)
{
	if(bhapi::is_sem_for_IPC(sem))
		umtx_lock(sem->iLocker, bhapi::get_current_bsd_tid());
	else
		pthread_mutex_lock(sem->iMutex);
}


static void bhapi::unlock_sem_inter(b_posix_sem_t *sem)
{
	if(bhapi::is_sem_for_IPC(sem))
		umtx_unlock(sem->iLocker, bhapi::get_current_bsd_tid());
	else
		pthread_mutex_unlock(sem->iMutex);
}


static void* bhapi::create_sem_for_IPC(int64 count, const char *name, bhapi::area_access area_access)
{
	if(count < B_INT64_CONSTANT(0) || name == NULL || *name == 0 || strlen(name) > B_OS_NAME_LENGTH) return NULL;

	b_posix_sem_t *sem = new b_posix_sem_t();
	if(!sem) return NULL;

	BHAPI_LOCK_IPC_SEMAPHORE();

	if((sem->mapping = bhapi::create_area(name, (void**)&(sem->semInfo), sizeof(b_posix_sem_info),
					  B_READ_AREA | B_WRITE_AREA, BHAPI_AREA_SYSTEM_SEMAPHORE_DOMAIN, area_access)) == NULL ||
	   sem->semInfo == NULL)
	{
		BHAPI_DEBUG("[KERNEL]: %s --- Can't create sem : create area failed --- \"%s\"", __PRETTY_FUNCTION__, name);
		if(sem->mapping) bhapi::delete_area(sem->mapping);
		BHAPI_UNLOCK_IPC_SEMAPHORE();
		delete sem;
		return NULL;
	}

	b_posix_sem_info *sem_info = sem->semInfo;
	sem_info->InitData();
	memcpy(sem_info->name, name, (size_t)strlen(name));

	umtx_init(sem->iLocker = &(sem_info->locker));
	sem->iSeq = &(sem_info->seq);

	sem->semInfo->count = count;
	sem->semInfo->refCount = 1;

	BHAPI_UNLOCK_IPC_SEMAPHORE();

	sem->created = true;

	return (void*)sem;
}


BHAPI_IMPEXP void* bhapi::clone_sem(const char *name)
{
	if(name == NULL || *name == 0 || strlen(name) > B_OS_NAME_LENGTH) return NULL;

	b_posix_sem_t *sem = new b_posix_sem_t();
	if(!sem) return NULL;

	BHAPI_LOCK_IPC_SEMAPHORE();

	if((sem->mapping = bhapi::clone_area(name, (void**)&(sem->semInfo),
					  B_READ_AREA | B_WRITE_AREA, BHAPI_AREA_SYSTEM_SEMAPHORE_DOMAIN)) == NULL ||
	   sem->semInfo == NULL || sem->semInfo->refCount >= B_MAXUINT32)
	{
//		BHAPI_DEBUG("[KERNEL]: %s --- Can't clone semaphore : clone area failed --- \"%s\"", __PRETTY_FUNCTION__, name);
		if(sem->mapping) bhapi::delete_area(sem->mapping);
		BHAPI_UNLOCK_IPC_SEMAPHORE();
		delete sem;
		return NULL;
	}

	b_posix_sem_info *sem_info = sem->semInfo;

	sem->iLocker = &(sem_info->locker);
	sem->iSeq = &(sem_info->seq);

	sem->semInfo->refCount += 1;

	BHAPI_UNLOCK_IPC_SEMAPHORE();

	sem->created = true;

	return (void*)sem;
}


BHAPI_IMPEXP void* bhapi::clone_sem_by_source(void *data)
{
	b_posix_sem_t *sem = (b_posix_sem_t*)data;
	if(!sem || !sem->semInfo) return NULL;

	if(bhapi::is_sem_for_IPC(sem)) return bhapi::clone_sem(sem->semInfo->name);

	BHAPI_LOCKBHAPI_LOCAL_SEMAPHORE();
	if(sem->no_clone || sem->semInfo->refCount >= B_MAXUINT32 || sem->semInfo->refCount == 0)
	{
		BHAPI_UNLOCKBHAPI_LOCAL_SEMAPHORE();
		return NULL;
	}
	sem->semInfo->refCount += 1;
	BHAPI_UNLOCKBHAPI_LOCAL_SEMAPHORE();

	return data;
}


static void* bhapi::create_sem_for_local(int64 count)
{
	if(count < B_INT64_CONSTANT(0)) return NULL;

	b_posix_sem_t *sem = new b_posix_sem_t();
	if(!sem) return NULL;

	if((sem->semInfo = new b_posix_sem_info()) == NULL ||
	   (sem->iMutex = new pthread_mutex_t) == NULL ||
	   (sem->iCond = new pthread_cond_t) == NULL)
	{
		if(sem->iMutex) delete sem->iMutex;
		if(sem->iCond) delete sem->iCond;
		if(sem->semInfo) delete sem->semInfo;
		delete sem;
		return NULL;
	}

	uint32_t successFlags = 0;

	if(pthread_mutex_init(sem->iMutex, NULL) != 0) successFlags |= (1 << 1);
	if(pthread_cond_init(sem->iCond, NULL) != 0) successFlags |= (1 << 2);

	if(successFlags != 0)
	{
		if(!(successFlags & (1 << 1))) pthread_mutex_destroy(sem->iMutex);
		if(!(successFlags & (1 << 2))) pthread_cond_destroy(sem->iCond);
		delete sem->iMutex;
		delete sem->iCond;
		delete sem->semInfo;
		delete sem;
		return NULL;
	}

	sem->semInfo->count = count;
	sem->semInfo->refCount = 1;
	sem->created = true;

	return (void*)sem;
}


BHAPI_IMPEXP void* bhapi::create_sem(int64 count, const char *name, bhapi::area_access area_access)
{
	return((name == NULL || *name == 0) ?
			bhapi::create_sem_for_local(count) :
			bhapi::create_sem_for_IPC(count, name, area_access));
}


BHAPI_IMPEXP status_t bhapi::get_sem_info(void *data, bhapi::sem_info *info)
{
	b_posix_sem_t *sem = (b_posix_sem_t*)data;
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


BHAPI_IMPEXP status_t bhapi::delete_sem(void *data)
{
	b_posix_sem_t *sem = (b_posix_sem_t*)data;
	if(!sem || !sem->semInfo) return B_BAD_VALUE;

	if(bhapi::is_sem_for_IPC(sem)) BHAPI_LOCK_IPC_SEMAPHORE();
	else BHAPI_LOCKBHAPI_LOCAL_SEMAPHORE();

	uint32_t count = --(sem->semInfo->refCount);

	if(bhapi::is_sem_for_IPC(sem)) BHAPI_UNLOCK_IPC_SEMAPHORE();
	else BHAPI_UNLOCKBHAPI_LOCAL_SEMAPHORE();

	if(bhapi::is_sem_for_IPC(sem))
	{
		bhapi::delete_area(sem->mapping);
	}
	else
	{
		if(count > 0) return B_OK;

		pthread_mutex_destroy(sem->iMutex);
		pthread_cond_destroy(sem->iCond);
		delete sem->iMutex;
		delete sem->iCond;
		delete sem->semInfo;
	}

	if(sem->created)
	{
		sem->created = false;
		delete sem;
	}

	return B_OK;
}


BHAPI_IMPEXP status_t bhapi::delete_sem_etc(void *data, bool no_clone)
{
	b_posix_sem_t *sem = (b_posix_sem_t*)data;
	if(!sem || !sem->semInfo) return B_BAD_VALUE;

	if(bhapi::is_sem_for_IPC(sem)) BHAPI_LOCK_IPC_SEMAPHORE();
	else BHAPI_LOCKBHAPI_LOCAL_SEMAPHORE();

	if(!bhapi::is_sem_for_IPC(sem) && no_clone) sem->no_clone = true;
	uint32_t count = --(sem->semInfo->refCount);

	if(bhapi::is_sem_for_IPC(sem)) BHAPI_UNLOCK_IPC_SEMAPHORE();
	else BHAPI_UNLOCKBHAPI_LOCAL_SEMAPHORE();

	if(bhapi::is_sem_for_IPC(sem))
	{
		bhapi::delete_area_etc(sem->mapping, no_clone);
	}
	else
	{
		if(count > 0) return B_OK;

		pthread_mutex_destroy(sem->iMutex);
		pthread_cond_destroy(sem->iCond);
		delete sem->iMutex;
		delete sem->iCond;
		delete sem->semInfo;
	}

	if(sem->created)
	{
		sem->created = false;
		delete sem;
	}

	return B_OK;
}


BHAPI_IMPEXP status_t bhapi::close_sem(void *data)
{
	b_posix_sem_t *sem = (b_posix_sem_t*)data;
	if(!sem) return B_BAD_VALUE;

	bhapi::lock_sem_inter(sem);

	if(sem->semInfo->closed)
	{
		bhapi::unlock_sem_inter(sem);
		return B_ERROR;
	}
	sem->semInfo->closed = true;

	if(bhapi::is_sem_for_IPC(sem))
	{
		if(sem->semInfo->acquiringCount > B_INT64_CONSTANT(0))
		{
			*(sem->iSeq) += 1;
			umtx_wake((struct umtx*)sem->iSeq, 1);
		}
	}
	else
	{
		pthread_cond_broadcast(sem->iCond);
	}

	bhapi::unlock_sem_inter(sem);

	return B_OK;
}


BHAPI_IMPEXP status_t bhapi::acquire_sem_etc(void *data,  int64 count,  uint32_t flags, bigtime_t microseconds_timeout)
{
	b_posix_sem_t *sem = (b_posix_sem_t*)data;
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

	sem->semInfo->acquiringCount += count;
	if(bhapi::is_sem_for_IPC(sem))
	{
		if(sem->semInfo->minAcquiringCount == B_INT64_CONSTANT(0) ||
		   sem->semInfo->minAcquiringCount > count) sem->semInfo->minAcquiringCount = count;
	}

	struct timespec ts;
	bigtime_t timeout = microseconds_timeout - currentTime;
	ts.tv_sec = (long)(microseconds_timeout / B_INT64_CONSTANT(1000000));
	ts.tv_nsec = (long)(microseconds_timeout % B_INT64_CONSTANT(1000000)) * 1000L;

	status_t retval = B_ERROR;

	while(true)
	{
		if(bhapi::is_sem_for_IPC(sem))
		{
			long seq = *(sem->iSeq);

			bhapi::unlock_sem_inter(sem);

			int ret;
			if(wait_forever)
			{
				ret = umtx_wait((struct umtx*)sem->iSeq, seq, NULL);
			}
			else
			{
				struct timespec ts2;
				ts2.tv_sec = (long)(timeout / B_INT64_CONSTANT(1000000));
				ts2.tv_nsec = (long)(timeout % B_INT64_CONSTANT(1000000)) * 1000L;

				ret = (timeout < 0 ? ETIMEDOUT : umtx_wait((struct umtx*)sem->iSeq, seq, &ts2));

				timeout = microseconds_timeout - b_real_time_clock_usecs();
			}

			bhapi::lock_sem_inter(sem);
			if(ret != 0)
			{
				if(ret == ETIMEDOUT && !wait_forever) retval = B_TIMED_OUT;
				break;
			}
		}
		else
		{
			int ret = (wait_forever ? pthread_cond_wait(sem->iCond, sem->iMutex) :
						  pthread_cond_timedwait(sem->iCond, sem->iMutex, &ts));

			if(ret != 0)
			{
				if(ret == ETIMEDOUT && !wait_forever)
					retval = B_TIMED_OUT;
				else
					bhapi::lock_sem_inter(sem);
				break;
			}
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

		if(!bhapi::is_sem_for_IPC(sem)) continue;

		if(sem->semInfo->minAcquiringCount > sem->semInfo->count) continue;
		if(sem->semInfo->minAcquiringCount == B_INT64_CONSTANT(0) ||
		   sem->semInfo->minAcquiringCount > count) sem->semInfo->minAcquiringCount = count;

		*(sem->iSeq) += 1;
		umtx_wake((struct umtx*)sem->iSeq, 1);
	}

	sem->semInfo->acquiringCount -= count;
	if(bhapi::is_sem_for_IPC(sem))
	{
		if(sem->semInfo->minAcquiringCount == count) sem->semInfo->minAcquiringCount = B_INT64_CONSTANT(0);
		*(sem->iSeq) += 1;
		umtx_wake((struct umtx*)sem->iSeq, 1);
	}

	bhapi::unlock_sem_inter(sem);

	return retval;
}


BHAPI_IMPEXP status_t bhapi::acquire_sem(void *data)
{
	return bhapi::acquire_sem_etc(data, B_INT64_CONSTANT(1), B_TIMEOUT, B_INFINITE_TIMEOUT);
}


BHAPI_IMPEXP status_t bhapi::release_sem_etc(void *data,  int64 count,  uint32_t flags)
{
	b_posix_sem_t *sem = (b_posix_sem_t*)data;
	if(!sem || count < B_INT64_CONSTANT(0)) return B_BAD_VALUE;

	bhapi::lock_sem_inter(sem);

	status_t retval = B_ERROR;

	if(sem->semInfo->closed == false && (B_MAXINT64 - sem->semInfo->count >= count))
	{
		sem->semInfo->count += count;

		if(flags != B_DO_NOT_RESCHEDULE)
		{
			if(bhapi::is_sem_for_IPC(sem))
			{
				if(sem->semInfo->acquiringCount > B_INT64_CONSTANT(0))
				{
					*(sem->iSeq) += 1;
					umtx_wake((struct umtx*)sem->iSeq, 1);
				}
			}
			else
			{
				pthread_cond_broadcast(sem->iCond);
			}
		}

		retval = B_OK;
	}

	bhapi::unlock_sem_inter(sem);

	return retval;
}


BHAPI_IMPEXP status_t bhapi::release_sem(void *data)
{
	return bhapi::release_sem_etc(data, B_INT64_CONSTANT(1), 0);
}


BHAPI_IMPEXP status_t bhapi::get_sem_count(void *data,  int64 *count)
{
	b_posix_sem_t *sem = (b_posix_sem_t*)data;
	if(!sem || !count) return B_BAD_VALUE;

	bhapi::lock_sem_inter(sem);
	*count = (sem->semInfo->acquiringCount <= B_INT64_CONSTANT(0) ?
			sem->semInfo->count : B_INT64_CONSTANT(-1) * (sem->semInfo->acquiringCount));
	bhapi::unlock_sem_inter(sem);

	return B_OK;
}
#endif
